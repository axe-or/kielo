#include "testing.h"
#include "lexer_test.c"

int main(){
	bool ok = true
		&& test_lexer()
	;
	return !ok;
}

