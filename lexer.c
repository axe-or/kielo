#include "kielo.h"

static inline
bool is_alpha(rune c){
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline
bool is_decimal(rune c){
	return (c >= '0' && c <= '9');
}

static inline
bool is_whitespace(rune c){
	return (c == '\n') || (c == '\r') || (c == '\t') || (c == ' ') || (c == '\v');
}

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

bool lexer_match_advance(Lexer* l, rune match){
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

		if(str_equals(token.lexeme, kw_lexeme)){
			token.kind = kw_kind;
			break;
		}
	}
	return token;
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

static inline
rune escape_rune(rune code){
	switch(code){
	case 'r':  return '\r';
	case 'n':  return '\n';
	case 't':  return '\t';
	case '\'': return '\'';
	case '"':  return '"';
	}
	return 0;
}

Token lexer_consume_non_decimal_integer(Lexer* lex, int base){
	unimplemented("Nondec num");
}

Token lexer_consume_decimal(Lexer* lex){
	unimplemented("Dec num");
}

Token lexer_consume_number(Lexer* lex){
	rune first = lexer_peek(lex, 0).codepoint;
	Token token = {0};
	ensure(is_decimal(first), "Not on a number");

	rune second = lexer_peek(lex, 1).codepoint;
	int base = 10;

	if(is_alpha(second)){
		switch(second){
		case 'b': case 'B': base = 16; break;
		case 'o': case 'O': base = 8; break;
		case 'x': case 'X': base = 2; break;
		default: {
			lexer_emit_error(lex, LexerError_InvalidBase, "Invalid base prefix: '%c'\n", second);
			return token;
		} break;
		}
	}

	if(base != 10){
		return lexer_consume_non_decimal_integer(lex, base);
	}

	token = lexer_consume_decimal(lex);

	return token;
}

Token lexer_consume_string(Lexer* lex){
	unimplemented("String");
}

// Lexer helpers to make it cleaner to read
#define MATCH_NEXT(Char, TType) if(lexer_match_advance(lex, Char)){ token.kind = TokenKind_##TType; break; }
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

		case '}':
			MATCH_DEFAULT(CurlyClose);

		case '.':
			MATCH_DEFAULT(Dot);

		case ',':
			MATCH_DEFAULT(Comma);

		case ':':
			MATCH_DEFAULT(Colon);

		case ';':
			MATCH_DEFAULT(Semicolon);

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

		case '=':
			MATCH_NEXT('=', Equal);
			MATCH_DEFAULT(Assign);

		case '!':
			MATCH_NEXT('=', NotEqual);
			MATCH_DEFAULT(LogicNot);

		case '/': {
			if(lexer_match_advance(lex, '=')){
				token.kind = TokenKind_AssignSlash;
			} else if(lexer_match_advance(lex, '/')){
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

		case '"': {
			token = lexer_consume_string(lex);
		} break;

		default: {
			if(is_alpha(c) || c == '_'){
				lex->current -= 1;
				token = lexer_consume_identifier_or_keyword(lex);
			}
			else if(is_decimal(c)){
				lex->current -= 1;
				token = lexer_consume_number(lex);
			}
		} break;
	}

	return token;
}

#undef MATCH_NEXT
#undef MATCH_DEFAULT

