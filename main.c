#include "base/types.h"
#include "base/ensure.h"
#include "base/string.h"
#include "base/memory.h"
#include "base/thread.h"

#define SPECIAL_TOKENS \
	X(Unknown, "<Unknown>") \
	X(EndOfFile, "<EOF>") \
	X(Whitespace, "<WS>") \
	X(Comment, "Comment") \
	X(Identifier, "Id") \
	X(String, "String") \
	X(Real, "Real") \
	X(Integer, "Int") \

#define DELIMITER_TOKENS \
	/* Delimiters */ \
	X(ParenOpen, "(") \
	X(ParenClose, ")") \
	X(SquareOpen, "[") \
	X(SquareClose, "]") \
	X(CurlyOpen, "{") \
	X(CurlyClose, "}") \
	X(RightArrow, "->") \
	X(Dot, ".") \
	X(Comma, ",") \
	X(Colon, ":") \
	X(Semicolon, ";") \
	/* Arithmetic */ \
	X(Plus, "+") \
	X(Minus, "-") \
	X(Star, "*") \
	X(Slash, "/") \
	X(Modulo, "%") \
	X(And, "&") \
	X(Or, "|") \
	X(Tilde, "~") \
	X(ShiftLeft, "<<") \
	X(ShiftRight, ">>") \
	/* Logic */ \
	X(LogicAnd, "&&") \
	X(LogicOr, "||") \
	X(LogicNot, "!") \
	/* Comparison */ \
	X(Equal, "==") \
	X(NotEqual, "!=") \
	X(Greater, ">") \
	X(Less, "<") \
	X(GreaterEqual, ">=") \
	X(LessEqual, "<=") \
	/* Assignment */ \
	X(Assign, "=") \
	X(AssignPlus, "+=") \
	X(AssignMinus, "-=") \
	X(AssignStar, "*=") \
	X(AssignSlash, "/=") \
	X(AssignModulo, "%=") \
	X(AssignAnd, "&=") \
	X(AssignOr, "|=")

#define KEYWORD_TOKENS \
	X(Let, "let") \
	X(Fn, "fn") \
	X(Return, "return") \
	X(If, "if") \
	X(Else, "else") \
	X(For, "for") \
	X(Continue, "continue") \
	X(Break, "break") \

#define ALL_TOKENS \
	SPECIAL_TOKENS \
	DELIMITER_TOKENS \
	KEYWORD_TOKENS \

typedef enum {
	#define X(Name, _) TokenKind_##Name,
	ALL_TOKENS
	#undef X
	TokenKind__len,
} TokenKind;

static inline
bool is_alpha(rune c){
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

String token_kind_name(TokenKind k){
	String s = str_lit("<INVALID TOKEN KIND>");
	switch(k){
	#define X(Name, Str) case TokenKind_##Name: s = str_lit(Str); break;
	ALL_TOKENS
	#undef X
	case TokenKind__len: break;
	}
	return s;
}

static struct { String lexeme; TokenKind kind; } keyword_lexemes[] = {
	#define X(Name, Str) { str_lit(Str), TokenKind_##Name },
	KEYWORD_TOKENS
	#undef X
};

#undef SPECIAL_TOKENS
#undef KEYWORD_TOKENS
#undef DELIMITER_TOKENS
#undef ALL_TOKENS

typedef enum {
	CompilerStage_None = 0,
	CompilerStage_Lex,
	CompilerStage_Parse,
	CompilerStage_Check,
	CompilerStage_Emmit,

	CompilerStage__len,
} CompilerStage;

typedef enum {
	LexerError_None = 0,
	LexerError_UnknownCodepoint,
	LexerError_UnclosedString,
} LexerError;

typedef struct CompilerError CompilerError;

struct CompilerError {
	String filename;
	String message;
	u64 offset;
	u32 stage;
	u32 type;
	CompilerError* next;
};

typedef struct {
	String lexeme;
	u32 kind;
} Token;

typedef struct {
	Token* v;
	isize len;
} TokenArray;

typedef struct {
	String source;
	String filename;
	isize current;
	isize start;

	Arena* error_arena;
	CompilerError* error;
} Lexer;

Lexer lexer_create(String source, Arena* error_arena){
	Lexer lex = {
		.source = source,
		.current = 0,
		.start = 0,
		.error_arena = error_arena,
		.error = NULL,
	};
	return lex;
}

str_attribute_format(3,4)
void lexer_emit_error(Lexer* lex, LexerError type, char const* fmt, ...){
	CompilerError* err = arena_make(lex->error_arena, CompilerError, 1);
	err->type = (u32)type;
	err->stage = CompilerStage_Lex;
	err->offset = lex->current;
	err->filename = lex->filename;

	/* Format message */ {
		va_list argp;
		va_start(argp, fmt);
		err->message = str_vformat(lex->error_arena, fmt, argp);
		va_end(argp);
	}

	err->next  = lex->error;
	lex->error = err;
}

#ifdef TERM_NO_COLOR
#define TERM_COLOR_RED   ""
#define TERM_COLOR_GREEN ""
#define TERM_COLOR_RESET ""
#else
#define TERM_COLOR_RED   "\e[0;31m"
#define TERM_COLOR_GREEN "\e[0;32m"
#define TERM_COLOR_RESET "\e[0m"
#endif

void print_compiler_error(CompilerError const* err){
	printf(TERM_COLOR_RED "error" TERM_COLOR_RESET " (%.*s:%lld) %.*s\n",
		str_fmt(err->filename),
		(long long)err->offset,
		str_fmt(err->message));
}

UTF8Decoded lexer_advance(Lexer* lex){
	if(lex->current >= lex->source.len){
		return (UTF8Decoded){0,0};
	}
	UTF8Decoded res = utf8_decode(lex->source.v + lex->current, lex->source.len - lex->current);
	lex->current += res.len;
	return res;
}

UTF8Decoded lexer_peek(Lexer* lex, isize delta){
	UTF8Decoded res = {0, 1};
	isize pos = lex->current + delta;
	if(pos < 0 || pos > lex->source.len){ return res; }

	res = utf8_decode(lex->source.v + pos, lex->source.len - pos);
	return res;
}

static inline
bool is_whitespace(rune c){
	return (c == '\n') || (c == '\r') || (c == '\t') || (c == ' ') || (c == '\v');
}

String lexer_current_lexeme(Lexer const* lex){
	byte const* start = &lex->source.v[lex->start];
	return (String){
		.v = start,
		.len = lex->current - lex->start,
	};
}

Token lexer_consume_whitespace(Lexer* lex){
	ensure(is_whitespace(lex->source.v[lex->current]), "Not on whitespace");
	lex->start = lex->current;
	Token tk = {
		.kind = TokenKind_Whitespace,
	};

	do {
		if(!is_whitespace(lex->source.v[lex->current])){ break; }
		lex->current += 1;
	} while(lex->current <= lex->source.len);

	tk.lexeme = lexer_current_lexeme(lex);

	return tk;
}

bool lexer_consume_matching(Lexer* l, rune match){
	UTF8Decoded dec = lexer_peek(l, 0);
	if(dec.codepoint == match){
		l->current += dec.len;
		return true;
	}
	return false;
}

Token lexer_consume_line_comment(Lexer* l){
	unimplemented("Comment");
}

Token lexer_consume_identifier_or_keyword(Lexer* lex){
	rune first = lex->source.v[lex->current];
	Token token = {0};
	ensure(is_alpha(first) || first == '_', "Not on part of identifier");

	lex->start = lex->current;
	do {
		UTF8Decoded dec = lexer_advance(lex);
		rune c = dec.codepoint;

		if(!is_alpha(c) && c != '_'){
			lex->current -= dec.len;
			break;
		}
	} while(lex->current < lex->source.len);

	token.lexeme = lexer_current_lexeme(lex);
	token.kind = TokenKind_Identifier;

	const isize n =  sizeof(keyword_lexemes) / sizeof(keyword_lexemes[0]);

	for(isize i = 0; i < n; i += 1){
		String kw_lexeme = keyword_lexemes[i].lexeme;
		TokenKind kw_kind = keyword_lexemes[i].kind;
		// printf("CMP: '%.*s' <> '%.*s'\n", str_fmt(kw_lexeme), str_fmt(token.lexeme));

		if(str_equals(token.lexeme, kw_lexeme)){
			token.kind = kw_kind;
			break;
		}
	}
	return token;
}

// Lexer helpers to make it cleaner to read
#define MATCH_NEXT(Char, TType) if(lexer_consume_matching(lex, Char)){ token.kind = TokenKind_##TType; break; }
#define MATCH_DEFAULT(TType)    { token.kind = TokenKind_##TType; break; }

Token lexer_next_token(Lexer* lex){
	Token token = {
		.kind = TokenKind_Unknown,
	};

	UTF8Decoded dec = lexer_advance(lex);
	rune c = dec.codepoint;

	if(c == 0){
		token.kind = TokenKind_EndOfFile;
		return token;
	}

	switch(c){
		case '(':
			MATCH_DEFAULT(ParenOpen);

		case ')':
			MATCH_DEFAULT(ParenClose);

		case '[':
			MATCH_DEFAULT(SquareOpen);

		case ']':
			MATCH_DEFAULT(SquareClose);

		case '{':
			MATCH_DEFAULT(CurlyOpen);

		case '.':
			MATCH_DEFAULT(Dot);

		case ',':
			MATCH_DEFAULT(Comma);

		case ':':
			MATCH_DEFAULT(Colon);

		case ';':
			MATCH_DEFAULT(Semicolon);

		case '}':
			MATCH_DEFAULT(CurlyClose);

		case '>':
			MATCH_NEXT('=', GreaterEqual);
			MATCH_NEXT('>', ShiftRight);
			MATCH_DEFAULT(Greater);

		case '<':
			MATCH_NEXT('=', LessEqual);
			MATCH_NEXT('<', ShiftLeft);
			MATCH_DEFAULT(Less);

		case '-':
			MATCH_NEXT('>', RightArrow);
			MATCH_NEXT('=', AssignMinus);
			MATCH_DEFAULT(Minus);

		case '+':
			MATCH_NEXT('=', AssignPlus);
			MATCH_DEFAULT(Plus);

		case '*':
			MATCH_NEXT('=', AssignStar);
			MATCH_DEFAULT(Star);

		case '%':
			MATCH_NEXT('=', AssignModulo);
			MATCH_DEFAULT(Modulo);

		case '&':
			MATCH_NEXT('&', LogicAnd);
			MATCH_NEXT('=', AssignAnd);
			MATCH_DEFAULT(And);

		case '|':
			MATCH_NEXT('|', LogicOr);
			MATCH_NEXT('=', AssignOr);
			MATCH_DEFAULT(Or);
			
		case '~':
			MATCH_DEFAULT(Tilde);

		case '!':
			MATCH_NEXT('=', NotEqual);
			MATCH_DEFAULT(LogicNot);

		case '/': {
			if(lexer_consume_matching(lex, '=')){
				token.kind = TokenKind_AssignSlash;
			} else if(lexer_consume_matching(lex, '/')){
				lex->current -= 2;
				token = lexer_consume_line_comment(lex);
			} else {
				token.kind = TokenKind_Slash;
			}
		} break;

		case '\n': case '\r': case '\t': case '\v': case ' ': {
			lex->current -= 1;
			token = lexer_consume_whitespace(lex);
		} break;

		default: {
			if(is_alpha(c) || c == '_'){
				lex->current -= 1;
				token = lexer_consume_identifier_or_keyword(lex);
			}
		} break;
	}

	return token;
}

#undef MATCH_NEXT
#undef MATCH_DEFAULT

int main(){
	const isize arena_size = 8 * mem_megabyte;
	byte* arena_mem = heap_alloc(arena_size, 4096);
	Arena arena = arena_create(arena_mem, arena_size);

	String source = str_lit(
		"+-*/%+=-=*=/=%=>><<<><=>=!!=&|~&&&=|||=\n"
		"let x = 100;\n"
	);

	Lexer lex = lexer_create(source, &arena);

	do {
		Token tk = lexer_next_token(&lex);
		if(tk.kind == TokenKind_EndOfFile){ break; }

		printf("%12.*s | ", str_fmt(token_kind_name(tk.kind)));
		if(tk.lexeme.len > 0 && tk.kind != TokenKind_Whitespace){
			printf("\"%.*s\"\n", str_fmt(tk.lexeme));
		} else {
			printf("_\n");
		}
	} while(1);

	// lexer_emit_error(&lex, LexerError_None, "CU porra");
	// lexer_emit_error(&lex, LexerError_UnclosedString, "CU porra %d", 69);
	// lexer_emit_error(&lex, LexerError_UnclosedString, "SKibidi porra");
	//
	// for(CompilerError* err = lex.error; err != NULL; err = err->next){
	// 	print_compiler_error(err);
	// }

	heap_free(arena_mem);
}

