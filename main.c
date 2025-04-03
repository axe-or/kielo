#include "base/types.h"
#include "base/ensure.h"
#include "base/string.h"
#include "base/memory.h"
#include "base/thread.h"
#include "kielo.h"
#include "lexer.c"

void print_compiler_error(CompilerError const* err){
	printf(TERM_COLOR_RED "error" TERM_COLOR_RESET " (%.*s:%lld) %.*s\n",
		str_fmt(err->filename),
		(long long)err->offset,
		str_fmt(err->message));
}

int main(){
	const isize arena_size = 8 * mem_megabyte;
	byte* arena_mem = heap_alloc(arena_size, 4096);
	Arena arena = arena_create(arena_mem, arena_size);

	String source = str_lit(
		"+-*/%+=-=*=/=%=>><<<><=>=!!=&|~&&&=|||=\n"
		"let x:i32 = ;"
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

	for(CompilerError* err = lex.error; err != NULL; err = err->next){
		print_compiler_error(err);
	}
	
	i64 n = 0;
	ensure(str_parse_i64(str_lit("ffff_ffff_ffff_ffff"), 16, &n), "");
	printf("%td\n", n);

	heap_free(arena_mem);
}

