#include "memory.h"
#include "ensure.h"

Arena arena_create_buffer(uint8_t* buf, isize buf_size){
	MemoryBlock block = {
		.data = buf,
		.commited = buf_size,
		.reserved = buf_size,
	};

	return (Arena){
		.offset = 0,
		.last_allocation = NULL,
		.type = ArenaType_Buffer,
		.region_count = 0,
	};
}

Arena arena_create_dynamic(uint8_t* buf, isize buf_size){
	Arena arena = arena_create_buffer(buf, buf_size);
	arena.type = ArenaType_Dynamic;
	return arena;
}

#define ARENA_COMMIT_SIZE (1024 * 16)

void* arena_alloc(Arena* a, isize size, isize align){
	uintptr base = (uintptr)a->block.data;
	uintptr current = base + (uintptr)a->offset;

	isize available = a->block.commited - (current - base);

	uintptr aligned = mem_align_forward_ptr(current, align);
	uintptr padding = aligned - current;
	isize required  = padding + size;

	if(required > available){
		switch((ArenaType)a->type){
			case ArenaType_Buffer: return NULL; /* Out of memory */

			case ArenaType_Dynamic:
				if(a->next != NULL){
					return arena_alloc(a->next, size, align);
				}
				else {
					Arena* new_arena = heap_alloc(sizeof(Arena), alignof(Arena));
					if(new_arena == NULL){
						return NULL; /* VERY out of memory */
					}
					isize new_arena_size = mem_align_forward_size(size, 1024);
					byte* new_arena_buf = heap_alloc(new_arena_size, max(align, (isize)alignof(void*) * 2));
					if(new_arena_buf == NULL){
						heap_free(new_arena);
						return NULL; /* VERY out of memory */
					}
					*new_arena = arena_create_dynamic(new_arena_buf, new_arena_size);
					return arena_alloc(new_arena, size, align);
				}
			break;

			case ArenaType_Virtual:
				if(required >= a->block.reserved){
					return NULL; /* Allocation too big */
				}

				if(a->block.commited < a->block.reserved){
					byte* ptr = (byte*)a->block.data + a->block.commited;
					isize virtual_remaining = a->block.reserved - a->block.commited;
					isize to_commit = required;

					if(required > virtual_remaining){
						return NULL;
					}
					else if(ARENA_COMMIT_SIZE < virtual_remaining){
						to_commit = ARENA_COMMIT_SIZE;
					}

					if(virtual_commit(ptr, to_commit) == NULL){
						return NULL; /* Out of memory */
					}

					return arena_alloc(a, size, align);
				}
			break;
		}
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

