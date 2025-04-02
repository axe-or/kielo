#include "string.h"
#include "types.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#include <stdlib.h>

#define STRCONV_TEMP_BUFFER_SIZE 128

f64 str_parse_f64(String s){
	char temp[STRCONV_TEMP_BUFFER_SIZE] = {0};
	mem_copy_no_overlap(temp, s.v, min(s.len, STRCONV_TEMP_BUFFER_SIZE));
	return atof(temp);
}

i64 str_parse_i64(String s){
	char temp[STRCONV_TEMP_BUFFER_SIZE] = {0};
	mem_copy_no_overlap(temp, s.v, min(s.len, STRCONV_TEMP_BUFFER_SIZE));
	return atoll(temp);
}

String str_vformat(Arena* arena, char const * restrict fmt, va_list argp){
	char* ptr = arena->data + arena->offset;
	isize len = arena->capacity - arena->offset;
	isize n = 1;

	n = stbsp_vsnprintf(ptr, len, fmt, argp);
	arena->offset += n;

	String s = {
		.v = (byte const*)ptr,
		.len = n,
	};
	return s;
}

String str_format(Arena* arena, char const * restrict fmt, ...){
	String s = {};
	va_list argp;
	va_start(argp, fmt);
	s = str_vformat(arena, fmt, argp);
	va_end(argp);
	return s;
}

#undef STRCONV_TEMP_BUFFER_SIZE
