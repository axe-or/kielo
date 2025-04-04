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

int mem_compare(void const* left, void const* right, isize count);

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

static inline
isize mem_align_forward_size(isize p, isize a){
	ensure(mem_valid_alignment(a), "Alignment must be a power of 2 greater than 0");
	isize mod = p & (a - 1); /* Fast modulo for powers of 2 */
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

typedef enum {
	ArenaType_Buffer = 0,  /* Static arena backed by buffer */
	ArenaType_Dynamic = 1, /* Arena with backup storage from the heap */
	ArenaType_Virtual = 2, /* Arena with large pre-reserved allocation that commits as needed */
} ArenaType;

//// Arena allocator
typedef struct Arena Arena;

struct Arena {
	void* data;
	isize offset;
	isize capacity;
	isize commited; /* Virtual arena only */
	void* last_allocation;
	Arena* next; /* Always null for non-dynamic arenas */
	i32 region_count;
	u32 type;
};

typedef struct {
	Arena* arena;
	isize offset;
} ArenaRegion;

#define arena_make(A, Type, Count) \
	((Type *)arena_alloc((A), sizeof(Type) * (Count), alignof(Type)))

Arena arena_create(uint8_t* buf, isize buf_size);

Arena arena_create_dynamic(uint8_t* buf, isize buf_size);

Arena arena_create_virtual(isize reserve_size);

void* arena_alloc(Arena* arena, isize size, isize align);

bool arena_resize_in_place(Arena* arena, void* ptr, isize size);

void arena_reset(Arena* arena);

ArenaRegion arena_region_begin(Arena* a);

void arena_region_end(ArenaRegion reg);

void* arena_realloc(Arena* a, void* ptr, isize old_size, isize new_size, isize align);

//// Virtual memory allocator

// TODO: add assertion to enforce this.
#define MEM_VIRTUAL_PAGE_SIZE (4ll * 1024ll)

typedef enum {
	MemoryProtection_NoAccess = 0,
	MemoryProtection_Read     = (1 << 0),
	MemoryProtection_Write    = (1 << 1),
	MemoryProtection_Execute  = (1 << 2),
} MemoryProtection;

void* virtual_reserve(isize size);

void virtual_free(void* p, isize size);

void* virtual_commit(void* p, isize size);

void virtual_decommit(void* p, isize size);

void virtual_protect(void* p, isize size, u8 prot);

//// Heap allocator
void* heap_alloc(isize size, isize align);

void heap_free(void* ptr);

