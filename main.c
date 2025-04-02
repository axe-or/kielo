#include "base/types.h"
#include "base/ensure.h"
#include "base/string.h"
#include "base/memory.h"
#include "base/thread.h"

#define SPECIAL_TOKENS \
	X(Unknown, "<Unknown>") \
	X(EndOfFile, "<EOF>") \
	X(Whitespace, "<Whitespace>") \
	X(Comment, "") \
	X(Identifier, "") \
	X(String, "") \
	X(Real, "") \
	X(Integer, "") \

#define DELIMITER_TOKENS \
	/* Delimiters */ \
	X(ParenOpen, "(") \
	X(ParenClose, ")") \
	X(SquareOpen, "[") \
	X(SquareClose, "]") \
	X(CurlyOpen, "{") \
	X(CurlyClose, "}") \
	X(Dot, ".") \
	X(Comma, ",") \
	X(Colon, ":") \
	X(Semicolon, ";") \
	X(Assign, "=") \
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
	X(LessEqual, "<=")

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

static String token_kind_names[TokenKind__len] = {
	#define X(Name, Str) [TokenKind_##Name] = str_lit(Str),
	ALL_TOKENS
	#undef X
};

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
	u32 kind;
	String lexeme;
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
		lex->current += 1;
		if(!is_whitespace(lex->source.v[lex->current])){ break; }
	} while(lex->current < lex->source.len);

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

#define MATCH_NEXT(Char, TType) if(lexer_consume_matching(lex, Char)){ token.kind = TokenKind_##TType; } break
#define MATCH_DEFAULT(TType)    { token.kind = TokenKind_##TType; } break

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
		case '\n': case '\r': case '\t': case '\v': case ' ': {
			lex->current -= 1;
			token = lexer_consume_whitespace(lex);
		} break;

		case '(':
			MATCH_DEFAULT(ParenOpen);

		case '>':
			MATCH_NEXT('=', GreaterEqual);
			MATCH_NEXT('>', ShiftRight);
			MATCH_DEFAULT(Greater);

		case '-':
			// MATCH_NEXT('>', TokenKind_Arrow)
			MATCH_DEFAULT(Minus);

		case '+':
			MATCH_DEFAULT(Plus);

		default: {
			token.kind = TokenKind_Unknown;
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
		"fn main(){\n"
		"	return;\n"
	);

	Lexer lex = lexer_create(source, &arena);

	do {
		Token tk = lexer_next_token(&lex);
		if(tk.kind == TokenKind_EndOfFile){ break; }
		printf("%.*s \t| \"%.*s\"\n", str_fmt(token_kind_names[tk.kind]), str_fmt(tk.lexeme));
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

