#pragma once

#include "base/types.h"

#ifdef TERM_NO_COLOR
#define TERM_COLOR_RED   ""
#define TERM_COLOR_GREEN ""
#define TERM_COLOR_RESET ""
#else
#define TERM_COLOR_RED   "\e[0;31m"
#define TERM_COLOR_GREEN "\e[0;32m"
#define TERM_COLOR_RESET "\e[0m"
#endif

typedef enum {
	CompilerStage_None = 0,
	CompilerStage_Lex,
	CompilerStage_Parse,
	CompilerStage_Check,
	CompilerStage_Emmit,

	CompilerStage__len,
} CompilerStage;

typedef struct CompilerError CompilerError;

struct CompilerError {
	String filename;
	String message;
	u64 offset;
	u32 stage;
	u32 type;
	CompilerError* next;
};

