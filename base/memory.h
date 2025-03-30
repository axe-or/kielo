#pragma once

#include "types.h"
#include "ensure.h"

#define mem_kilobyte (1024ll)
#define mem_megabyte (1024ll * 1024ll)
#define mem_gigabyte (1024ll * 1024ll * 1024ll)

//// Core memory operations
void mem_copy(void* dest, void const* source, isize count);

void mem_copy_no_overlap(void* dest, void const* source, isize count);

void mem_set(void* dest, byte val, isize count);

int mem_compare(void const* left, void* const right, isize count);

static inline
bool mem_valid_alignment(usize a){
	return ((a & (a - 1)) == 0) && a > 0;
}

static inline
uintptr mem_align_forward_ptr(uintptr p, uintptr a){
	ensure(mem_valid_alignment(a), "Alignment must be a power of 2 greater than 0");
	uintptr mod = p & (a - 1); /* Fast modulo for powers of 2 */
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

//// Arena allocator
typedef struct {
	void*  data;
	isize offset;
	isize capacity;
	void*  last_allocation;
	int    region_count;
} Arena;

typedef struct {
	Arena* arena;
	isize offset;
} ArenaRegion;

#define arena_make(A, Type, Count) \
	((Type *)arena_alloc((A), sizeof(Type) * (Count), alignof(Type)))

Arena arena_create(uint8_t* buf, isize buf_size);

void* arena_alloc(Arena* arena, isize size, isize align);

bool arena_resize_in_place(Arena* arena, void* ptr, isize size);

void arena_reset(Arena* arena);

ArenaRegion arena_region_begin(Arena* a);

void arena_region_end(ArenaRegion reg);

void* arena_realloc(Arena* a, void* ptr, isize old_size, isize new_size, isize align);

//// Heap allocator
void* heap_alloc(isize size, isize align);

void heap_free(void* ptr);

//// Container helpers
// typedef void* (*MemReallocFunc)(void* ctx, void* ptr, isize old_size, isize new_size, isize align);
//
// static inline
// void* arena_realloc_wrapper(void* a, void* ptr, isize old_size, isize new_size, isize align){
// 	return arena_realloc(a, ptr, old_size, new_size, align);
// }

