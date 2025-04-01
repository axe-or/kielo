#pragma once
#include "types.h"

typedef struct Thread Thread;

typedef void (*ThreadFunc)(void*);

Thread* thread_create(ThreadFunc f, void* arg);

void thread_join(Thread* t);

void thread_terminate(Thread* t);

void thread_destroy(Thread* t);


