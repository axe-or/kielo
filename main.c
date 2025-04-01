#include "base/types.h"
#include "base/ensure.h"
#include "base/string.h"
#include "base/memory.h"
#include "base/thread.h"

typedef enum {
	None = 0,

	UnknownCodepoint,
	UnclosedString,
} LexerError;

#define SPECIAL_TOKENS \
	X(Unknown, "<Unknown>") \
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

typedef struct {
	String filename;
	u64 offset;
	String message;
	u32 type;
} CompilerError;

typedef struct {
	u32 type;
	String lexeme;
} Token;

typedef struct {
	Token* v;
	isize len;
} TokenArray;

typedef struct {
	String source;
	i32 current;
	i32 start;
} Lexer;

Lexer lexer_make(String source){
	Lexer lex = {
		.source = source,
		.current = 0,
		.start = 0,
	};
	return lex;
}

UTF8Decoded lexer_advance(Lexer* lex){
	if(lex->current >= lex->source.len){
		return (UTF8Decoded){0,0};
	}
	UTF8Decoded res = utf8_decode(lex->source.v + lex->current, lex->source.len - lex->current);
	lex->current += res.len;
	return res;
}


void worker(void* p){
	int n = (int)(uintptr)p;
	printf("> %d\n", n);
}

int main(){
	const isize arena_size = 8 * mem_megabyte;
	byte* arena_mem = heap_alloc(arena_size, 4096);
	Arena arena = arena_create(arena_mem, arena_size);

	String s = str_format(&arena, "%s %d %d\n", "HELLOOOO", 69, 69);
	printf("%p %ld %.*s\n", s.v, s.len, (int)s.len, (char const*)s.v);

	heap_free(arena_mem);

	for(int i = 0; i < TokenKind__len; i++){
		String s = token_kind_names[i];
		printf("%.*s = %td\n", (int)s.len, (char const*)s.v, s.len);
	}
}

