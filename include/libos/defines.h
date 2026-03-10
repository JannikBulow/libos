// Copyright 2026 Jannik Laugmand Bülow

// Everything defined here is meant for internal use in headers which is why they use LIBOS prefix instead of OS prefix.

// Maybe a shitty name for a single header to import in all other libos headers.

#ifndef LIBOS_DEFINES_H
#define LIBOS_DEFINES_H 1

#include "libos/api.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//TODO: Remove libc dependency from all headers eventually, then remove it from sources in the far future.
//      Once the libc dependency is gone, maybe use this header for all the declarations of libc-ish utilities.
//      Just spitting ideas for now.

typedef int8_t      os_i8;
typedef int16_t     os_i16;
typedef int32_t     os_i32;
typedef int64_t     os_i64;
typedef uint8_t     os_u8;
typedef uint16_t    os_u16;
typedef uint32_t    os_u32;
typedef uint64_t    os_u64;

typedef float       os_f32;
typedef double      os_f64;

typedef os_u8       os_bool;

typedef char        os_char;
typedef os_char*    os_string; // All strings in the libos api are UTF-8 encoded and NUL terminated.

// These two help with compatability (cough cough Windows).
typedef os_u16      os_char16;
typedef os_u32      os_char32;

typedef size_t      os_size;
typedef ssize_t     os_ssize;

typedef uintptr_t   os_uptr;
typedef intptr_t    os_iptr;

// These won't be used raw, but rather for typedef-ing specific flag types in each subsystem.
typedef os_u32      os_flags32;
typedef os_u64      os_flags64;

typedef os_i32      os_result;

typedef void*       os_handle; // Can in theory hold any typed handle as long as your compiler is chill with implicit casts. You may want to enable the LIBOS_SUPERSAFE_HANDLES feature if you unironically prefer this.

os_result __os_set_and_return_result(os_result result); // INTERNAL USE ONLY. Sets the last error in the current thread to `result` if `result` is less than 0, then returns the same result.

#ifdef __cplusplus
}
#endif

#endif // LIBOS_DEFINES_H
