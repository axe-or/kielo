#define _DEFAULT_SOURCE
#include "memory.h"

#include <sys/mman.h>

void* virtual_reserve(isize size){
	void* p = mmap(NULL, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	return p;
}

void virtual_free(void* p, isize size){
	munmap(p, size);
}

void* virtual_commit(void* p, isize size){
	if(mprotect(p, size, PROT_READ | PROT_WRITE) < 0){
		return NULL;
	}
	return p;
}

void virtual_decommit(void* p, isize size){
	mprotect(p, size, PROT_NONE);
	madvise(p, size, MADV_FREE);
}

void virtual_protect(void* p, isize size, u8 prot){
	int flags = 0;

	if(prot & MemoryProtection_Read){
		flags |= PROT_READ;
	}
	if(prot & MemoryProtection_Write){
		flags |= PROT_WRITE;
	}
	if(prot & MemoryProtection_Execute){
		flags |= PROT_EXEC;
	}

	mprotect(p, size, flags);
}
