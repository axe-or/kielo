#include "base/types.h"
#include "base/ensure.h"
#include "base/string.h"
#include "base/memory.h"
#include "kielo.h"
#include "lexer.c"

void print_compiler_error(CompilerError const* err){
	printf(TERM_COLOR_RED "error" TERM_COLOR_RESET " (%.*s:%lld) %.*s\n",
		str_fmt(err->filename),
		(long long)err->offset,
		str_fmt(err->message));
}

#define mem_GiB (1024ll * 1024ll * 1024ll)

int main(){
#define BIG_SIZE 64 * mem_GiB
	char* p = virtual_reserve(BIG_SIZE);
	char* p2 = virtual_commit(p, 4096);

	printf("%p\n", p);
	printf("%02x\n", p2[4096-1]);

	virtual_free(p, BIG_SIZE);
	return 0;
}

#if 0
int main(){
	const isize arena_size = 8 * mem_megabyte;
	byte* arena_mem = heap_alloc(arena_size, 4096);
	Arena arena = arena_create(arena_mem, arena_size);

	String source = str_lit(
		"//+-*/%+=-=*=/=%=>><<<><=>=!!=&|~&&&=|||=\n"
		"0xfea0_caf3_babe\n"
		"0b1010010101010010100101111111010\n"
		"0o100\n"
		"1920390\n"
		"1e-3\n"
	);

	Lexer lex = lexer_create(source, &arena);

	do {
		Token tk = lexer_next_token(&lex);
		if(tk.kind == TokenKind_EndOfFile){ break; }
		if(tk.kind == TokenKind_Whitespace){ continue; }

		printf("%12.*s | ", str_fmt(token_kind_name(tk.kind)));
		if(tk.lexeme.len > 0 && tk.kind != TokenKind_Whitespace){
			printf("\"%.*s\"", str_fmt(tk.lexeme));
			if(tk.kind == TokenKind_Integer){
				printf("-> %td\n", tk.value.integer);
			} else if(tk.kind == TokenKind_Real){
				printf("-> %g\n", tk.value.real);
			} else {
				printf("\n");
			}
		} else {
			printf("_\n");
		}
	} while(1);

	for(CompilerError* err = lex.error; err != NULL; err = err->next){
		print_compiler_error(err);
	}

	heap_free(arena_mem);
}
#endif
