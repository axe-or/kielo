#include "string.h"

isize str_compare(String left, String right){
	if(left.len > right.len){
		return left.len;
	}
	if(left.len < right.len){
		return -right.len;
	}

	isize cmp = mem_compare(left.v, right.v, min(left.len, right.len));
	return cmp;
}

bool str_equals(String left, String right){
	if(left.len != right.len){ return false; }
	isize cmp = mem_compare(left.v, right.v, left.len);
	return cmp == 0;
}

bool str_starts_with(String s, String prefix){
	if(prefix.len > s.len){ return false; }
	if(prefix.len == 0){ return true; }
	return mem_compare(s.v, prefix.v, prefix.len) == 0;
}

bool str_ends_with(String s, String postfix){
	if(postfix.len > s.len){ return false; }
	if(postfix.len == 0){ return true; }
	return mem_compare(s.v + (s.len - postfix.len), postfix.v, postfix.len) == 0;
}

String str_sub(String s, isize start, isize end){
	ensure(start <= s.len && end <= s.len && end >= start, "Improper range");
	return (String){ .v = s.v + start, .len = end - start };
}
