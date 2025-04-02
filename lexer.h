#pragma once

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


static struct { String lexeme; TokenKind kind; } keyword_lexemes[] = {
	#define X(Name, Str) { str_lit(Str), TokenKind_##Name },
	KEYWORD_TOKENS
	#undef X
};

// #undef SPECIAL_TOKENS
// #undef KEYWORD_TOKENS
// #undef DELIMITER_TOKENS
// #undef ALL_TOKENS

String token_kind_name(TokenKind k);

typedef enum {
	LexerError_None = 0,
	LexerError_UnknownCodepoint,
	LexerError_UnclosedString,
} LexerError;

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

Lexer lexer_create(String source, Arena* error_arena);

str_attribute_format(3,4)
void lexer_emit_error(Lexer* lex, LexerError type, char const* fmt, ...);

UTF8Decoded lexer_advance(Lexer* lex);

bool lexer_consume_matching(Lexer* l, rune match);

UTF8Decoded lexer_peek(Lexer* lex, isize delta);

String lexer_current_lexeme(Lexer const* lex);

Token lexer_consume_whitespace(Lexer* lex);

Token lexer_consume_line_comment(Lexer* lex);

Token lexer_consume_identifier_or_keyword(Lexer* lex);

Token lexer_next_token(Lexer* lex);

