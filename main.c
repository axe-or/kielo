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

	Thread* workers[20] = {0};
	for(int i = 0; i < 20; i++){
		workers[i] = thread_create(worker, (void*)(uintptr)i);
	}

	for(int i = 0; i < 20; i++){
		thread_join(workers[i]);
		thread_destroy(workers[i]);
	}
}

