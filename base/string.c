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
	unimplemented("pref");
}

bool str_ends_with(String s, String postfix){
	unimplemented("post");
}
