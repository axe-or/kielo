#pragma once
#include "base/types.h"
#include "base/ensure.h"

struct TestCase {
	char const* title;
	i32 failed;
	i32 total;
};

bool test_predicate_ex(struct TestCase* t, bool predicate, char const* msg, char const* filename, i32 line){
	t->total += 1;
	if(!predicate){
		t->failed += 1;
		printf("(%s:%d) Test failure: %s", filename, line, msg);
	}
	return predicate;
}

void test_display(struct TestCase t){
	char const* status = t.failed == 0 ? "PASS" : "FAIL";
	printf("[ %s ] %s ok in %d/%d\n", t.title, status, t.total - t.failed, t.total);
}

#define TEST_BEGIN(Name) struct TestCase _test = {.title = Name};

#define TEST(Pred) test_predicate_ex(&_test, (Pred), #Pred, __FILE__, __LINE__)

#define TEST_END test_display(_test); return _test.failed;


