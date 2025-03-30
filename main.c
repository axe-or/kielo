#include "base/types.h"
#include "base/ensure.h"
#include "base/memory.h"

typedef enum {
	None = 0,

	UnknownCodepoint,
	UnclosedString,
} LexerError;

typedef enum {
	Unknown = 0,

	ParenOpen,
	ParenClose,
	SquareOpen,
	SquareClose,
	CurlyOpen,
	CurlyClose,
} TokenType;

typedef struct {
	String filename;
	u64 offset;
} SourceLocation;

typedef struct {
	SourceLocation location;
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

// void* resize_func(void* ctx, void* ptr, isize old_size, isize new_size, isize align)
// void* resize_func(void* ctx, void* ptr, isize old_size, isize new_size, isize align)

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

int main(){
	static byte memory[200];
	Arena arena = arena_create(memory, 200);
}

