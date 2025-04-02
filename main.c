#include "base/types.h"
#include "base/ensure.h"
#include "base/string.h"
#include "base/memory.h"
#include "base/thread.h"
#include "kielo.h"

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
		"let x = 100;\n"
	);

	Lexer lex = lexer_create(source, &arena);

	// do {
	// 	Token tk = lexer_next_token(&lex);
	// 	if(tk.kind == TokenKind_EndOfFile){ break; }
	//
	// 	printf("%12.*s | ", str_fmt(token_kind_name(tk.kind)));
	// 	if(tk.lexeme.len > 0 && tk.kind != TokenKind_Whitespace){
	// 		printf("\"%.*s\"\n", str_fmt(tk.lexeme));
	// 	} else {
	// 		printf("_\n");
	// 	}
	// } while(1);

	ensure(str_starts_with(str_lit("pipi popo"), str_lit("pipi")), "");
	ensure(str_starts_with(str_lit("pipi popo"), str_lit("pipi popo")), "");
	ensure(str_starts_with(str_lit("pipi popo"), str_lit("")), "");
	ensure(!str_starts_with(str_lit("pipi popo"), str_lit("pipi popo.")), "");

	ensure(str_ends_with(str_lit("pipi popo"), str_lit("popo")), "");
	ensure(str_ends_with(str_lit("pipi popo"), str_lit("pipi popo")), "");
	ensure(str_ends_with(str_lit("pipi popo"), str_lit("")), "");
	ensure(!str_ends_with(str_lit("pipi popo"), str_lit("pipi popo.")), "");

	// lexer_emit_error(&lex, LexerError_None, "CU porra");
	// lexer_emit_error(&lex, LexerError_UnclosedString, "CU porra %d", 69);
	// lexer_emit_error(&lex, LexerError_UnclosedString, "SKibidi porra");

	for(CompilerError* err = lex.error; err != NULL; err = err->next){
		print_compiler_error(err);
	}

	heap_free(arena_mem);
}

#include "lexer.c"
