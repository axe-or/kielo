#define WIN32_MEAN_AND_LEAN
#include "memory.h"
#include <windows.h>

#define aligned_to_page_boundary(X) \
	(((uintptr)(X) & (MEM_VIRTUAL_PAGE_SIZE - 1)) == 0)

void* virtual_reserve(isize size){
	// ensure(aligned_to_page_boundary(size), "Size is not aligned to page boundary");
	void* p = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
	return p;
}

void virtual_free(void* p, isize size){
	// ensure(aligned_to_page_boundary(size), "Size is not aligned to page boundary");
	// ensure(aligned_to_page_boundary(p), "Pointer is not aligned to page boundary");
	VirtualFree(p, size, MEM_RELEASE);
}

void* virtual_commit(void* p, isize size){
	// ensure(aligned_to_page_boundary(size), "Size is not aligned to page boundary");
	// ensure(aligned_to_page_boundary(p), "Pointer is not aligned to page boundary");
	return VirtualAlloc(p, size, MEM_COMMIT, PAGE_READWRITE);
}

void virtual_decommit(void* p, isize size){
	// ensure(aligned_to_page_boundary(size), "Size is not aligned to page boundary");
	// ensure(aligned_to_page_boundary(p), "Pointer is not aligned to page boundary");
	VirtualFree(p, size, MEM_DECOMMIT);
}

void virtual_protect(void* p, isize size, u8 prot){
	// ensure(aligned_to_page_boundary(size), "Size is not aligned to page boundary");
	// ensure(aligned_to_page_boundary(p), "Pointer is not aligned to page boundary");
	uintptr old = 0;
	i32 prot_flag = -1;
	(void)old;

	switch(prot){
	case MemoryProtection_NoAccess: prot_flag = PAGE_NOACCESS; break;

	case MemoryProtection_Read: prot_flag = PAGE_READONLY; break;

	case MemoryProtection_Execute: prot_flag = PAGE_EXECUTE; break;

	case MemoryProtection_Read
		| MemoryProtection_Write: prot_flag = PAGE_READONLY; break;

	case MemoryProtection_Execute
		| MemoryProtection_Read
		| MemoryProtection_Write: prot_flag = PAGE_EXECUTE_READWRITE; break;
	}
	ensure(prot_flag > 0, "Invalid memory protection");

	VirtualProtect(p, size, prot_flag, (void*)&old);
}



