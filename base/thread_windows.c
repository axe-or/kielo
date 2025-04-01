#include "thread.h"

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

struct Thread {
	HANDLE handle;
	ThreadFunc fn;
	void* arg;
};

DWORD WINAPI thread_win32_wrapper(void* wrapper){
	Thread* w = wrapper;
	w->fn(w->arg);
	return 0;
}

Thread* thread_create(ThreadFunc f, void* arg){
	Thread* t = heap_alloc(sizeof(Thread), alignof(Thread));
	t->fn = f;
	t->arg = arg;
	t->handle = CreateThread(NULL, 0, thread_win32_wrapper, t, 0, NULL);
	ensure(t->handle != NULL, "Failed to create thread");
	return t;
}

void thread_join(Thread* t){
	WaitForSingleObject(t->handle, INFINITE);
}

void thread_destroy(Thread* t){
	TerminateThread(t->handle, 1);
	heap_free(t);
}

void thread_terminate(Thread* t){
	TerminateThread(t->handle, 1);
}
