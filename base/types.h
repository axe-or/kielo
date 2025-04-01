#pragma once

//// Compilation context
#if defined(__clang__)
	#define COMPILER_CLANG 1
#elif defined(__GNUC__)
	#define COMPILER_GCC 1
#elif defined(_MSC_VER)
	#define COMPILER_MSVC 1
#else
	#error "Unsupported compiler"
#endif

#if defined(__linux__)
	#define OS_LINUX 1
#elif defined(_WIN32) || defined(_WIN64)
	#define OS_WINDOWS 1
#else
	#error "Unsupported operating system"
#endif

//// Useful macros
#if defined(COMPILER_MSVC)
	#define typeof(X) __typeof(X)
#else
	#define typeof(X) __typeof__(X)
#endif

#if defined(COMPILER_MSVC)
	#define force_inline __forceinline
#else
	#define force_inline __attribute__((always_inline)) inline
#endif

#define static_assert(Pred, Msg) _Static_assert((Pred), (Msg))

#define min(A, B) (((A) < (B)) ? (A) : (B))

#define max(A, B) (((A) > (B)) ? (A) : (B))

#define clamp(Lo, X, Hi) min(max(Lo, X), Hi)

//// Core types
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdarg.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef ptrdiff_t isize;
typedef size_t    usize;
typedef uintptr_t uintptr;
typedef uint8_t   byte;
typedef int32_t   rune;

typedef struct {
	byte const* v;
	isize len;
} String;

// String literal
#define str_lit(S) ((String){.v = (byte const*)("" S ""), .len = (sizeof(S) - 1)})

