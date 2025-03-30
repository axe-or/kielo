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
	const isize arena_size = 8 * mem_megabyte;
	byte* arena_mem = heap_alloc(arena_size, 4096);
	Arena arena = arena_create(arena_mem, arena_size);

	printf("%p\n", arena.data);
}

