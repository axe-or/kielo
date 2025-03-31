/* Main base translation unit, shall be compiled ONCE per project */
#include "memory.c"
#include "arena.c"
#include "heap.c"

#include "string.c"
#include "format.c"

#if defined(OS_LINUX)
#include "thread_posix.c"
#else
#include "thread_windows.c"
#endif

