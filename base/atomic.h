#pragma once
#include "types.h"
#include <stdatomic.h>

typedef _Atomic(i8)  atomic_i8;
typedef _Atomic(i16) atomic_i16;
typedef _Atomic(i32) atomic_i32;
typedef _Atomic(i64) atomic_i64;

typedef _Atomic(u8)  atomic_u8;
typedef _Atomic(u16) atomic_u16;
typedef _Atomic(u32) atomic_u32;
typedef _Atomic(u64) atomic_u64;

typedef _Atomic(isize) atomic_isize;
typedef _Atomic(usize) atomic_usize;

typedef struct {
	atomic_i32 _state;
} Spinlock;

static inline
void spinlock_aquire(Spinlock* lock){
	for(;;){
		if(!atomic_exchange_explicit(&lock->_state, 1, memory_order_acquire)){
			break;
		}
		while(atomic_load_explicit(&lock->_state, memory_order_relaxed));
	}
}

static inline
void spinlock_release(Spinlock* lock){
	atomic_store_explicit(&lock->_state, 0, memory_order_release);
}

static inline
bool spinlock_try_acquire(Spinlock* lock){
	return !atomic_load_explicit(&lock->_state, memory_order_relaxed)
		&& !atomic_exchange_explicit(&lock->_state, 1, memory_order_acquire);
}

