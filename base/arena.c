#include "memory.h"
#include "ensure.h"

Arena arena_create(uint8_t* buf, isize buf_size){
	return (Arena){
		.data = buf,
		.offset = 0,
		.capacity = buf_size,
		.last_allocation = NULL,
		.commited = buf_size,
		.type = ArenaType_Buffer,
		.region_count = 0,
	};
}

void* arena_alloc(Arena* a, isize size, isize align){
	uintptr base = (uintptr)a->data;
	uintptr current = base + (uintptr)a->offset;

	isize available = a->capacity - (current - base);

	uintptr aligned = mem_align_forward_ptr(current, align);
	uintptr padding = aligned - current;
	isize required  = padding + size;

	if(required > available){
		return NULL; /* Out of memory */
	}

	a->offset += required;
	void* allocation = (void*)aligned;
	a->last_allocation = allocation;
	mem_set(allocation, 0, size);

	return allocation;
}

void* arena_realloc(Arena* a, void* ptr, isize old_size, isize new_size, isize align){
	ensure(old_size > 0 && new_size > 0, "Invalid sizes");

	if(ptr == NULL){
		return arena_alloc(a, new_size, align);
	}

	bool in_place = arena_resize_in_place(a, ptr, new_size);
	if(in_place){
		return ptr;
	}

	void* new_alloc = arena_alloc(a, new_size, align);
	if(!new_alloc){
		return NULL;
	}

	mem_copy_no_overlap(new_alloc, ptr, min(old_size, new_size));
	return new_alloc;
}

bool arena_resize_in_place(Arena* a, void* ptr, isize new_size){
	uintptr base    = (uintptr)a->data;
	uintptr current = base + (uintptr)a->offset;
	uintptr limit   = base + a->capacity;

	ensure((uintptr)ptr >= base && (uintptr)ptr < limit, "Pointer is not owned by arena");

	if(ptr == a->last_allocation){
		isize last_allocation_size = current - (uintptr)a->last_allocation;
		if(((current - last_allocation_size) + new_size) > limit){
			return false; /* No space left */
		}

		a->offset += new_size - last_allocation_size;
		return true;
	}

	return false;
}

void arena_reset(Arena* arena){
	ensure(arena->region_count == 0, "Arena has dangling regions");
	arena->offset = 0;
	arena->last_allocation = NULL;
}

ArenaRegion arena_region_begin(Arena* a){
	ArenaRegion reg = {
		.arena = a,
		.offset = a->offset,
	};
	a->region_count += 1;
	return reg;
}

void arena_region_end(ArenaRegion reg){
	ensure(reg.arena->region_count > 0, "Arena has a improper region counter");
	ensure(reg.arena->offset >= reg.offset, "Arena has a lower offset than region");

	reg.arena->offset = reg.offset;
	reg.arena->region_count -= 1;
}

