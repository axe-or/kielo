#define _XOPEN_SOURCE 800
#include <pthread.h>
#include "memory.h"
#include "thread.h"

struct Thread {
	pthread_t handle;
	ThreadFunc fn;
	void* arg;
};

static inline
void* thread_pthread_wrapper(void* wrapper){
	Thread* w = wrapper;
	w->fn(w->arg);
	return NULL;
}

Thread* thread_create(ThreadFunc f, void* arg){
	Thread* t = heap_alloc(sizeof(Thread), alignof(Thread));
	t->fn = f;
	t->arg = arg;

	pthread_create(&t->handle, NULL, thread_pthread_wrapper, t);
	return t;
}

void thread_join(Thread* t){
	if(t == NULL){ return; }
	void* p = 0;
	pthread_join(t->handle, &p);
}

void thread_destroy(Thread* t){
	if(t == NULL){ return; }
	pthread_cancel(t->handle);
	heap_free(t);
}


