#define _XOPEN_SOURCE 800
#include <pthread.h>
#include "memory.h"
#include "thread.h"
#include "atomic.h"

enum {
	Thread_Suspended = 0,
	Thread_Running = 1,
	Thread_Done = 2,
};

struct Thread {
	pthread_t handle;
	pthread_mutex_t mutex;
	ThreadFunc fn;
	void* arg;
	u32 status;
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
	t->status = Thread_Running;

	int mutex_status = pthread_mutex_init(&t->mutex, NULL);
	int thread_status = pthread_create( &t->handle, NULL, thread_pthread_wrapper, t);
	ensure(thread_status >= 0 && mutex_status >= mutex_status, "Failed to create thread");
	return t;
}

void thread_join(Thread* t){
	if(t == NULL){ return; }
	pthread_mutex_lock(&t->mutex);
	{
		ensure(t->status == Thread_Running, "Cannot join a non-running thread");
		void* p = 0;
		pthread_join(t->handle, &p);
		t->status = Thread_Done;
	}
	pthread_mutex_unlock(&t->mutex);
}

void thread_terminate(Thread* t){
	pthread_mutex_lock(&t->mutex);
	{
		ensure(t->status == Thread_Running, "Cannot terminate non-running thread");
		pthread_cancel(t->handle);
		t->status = Thread_Done;
	}
	pthread_mutex_unlock(&t->mutex);
}

void thread_destroy(Thread* t){
	if(t == NULL){ return; }

	pthread_mutex_lock(&t->mutex);
	{
		if(t->status == Thread_Running){
			pthread_detach(t->handle);
		}
	}
	pthread_mutex_unlock(&t->mutex);

	pthread_mutex_destroy(&t->mutex);
	heap_free(t);
}

