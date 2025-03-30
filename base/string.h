#pragma once
#include "types.h"

typedef struct {
	byte bytes[4];
	i32  len;
} UTF8Encoded;

typedef struct {
	rune codepoint;
	i32  len;
} UTF8Decoded;

static inline
bool utf8_is_continuation_byte(rune c){
	static const rune CONTINUATION1 = 0x80;
	static const rune CONTINUATION2 = 0xbf;
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

i32 utf8_rune_size(rune c);

UTF8Encoded utf8_encode(rune r);

UTF8Decoded utf8_decode(byte const* buf, isize n);

