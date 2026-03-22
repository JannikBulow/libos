// Copyright 2026 Jannik Laugmand Bülow

// I really like the Windows API approach to virtual memory, so you may notice some slight similarities.

#ifndef LIBOS_MEMORY_H
#define LIBOS_MEMORY_H 1

#include "libos/defines.h"
#include "libos/error.h"

#ifdef __cplusplus
extern "C" {
#endif

enum os_mem_alloc_intents {
    OS_MEM_RESERVE      = 1 << 0,
    OS_MEM_COMMIT       = 1 << 1,
    OS_MEM_LARGE_PAGES  = 1 << 2, // If you specify this intent, you must also specify RESERVE and COMMIT

    OS_MEM_ZERO         = 1 << 3,
};

enum os_mem_free_intents {
    OS_MEM_DECOMMIT     = 1 << 0,
    OS_MEM_RELEASE      = 1 << 1,
};

enum os_mem_protect_intents {
    OS_MEM_PROTECT_NOACCESS             = 1 << 0,
    OS_MEM_PROTECT_READ                 = 1 << 1,
    OS_MEM_PROTECT_READWRITE            = 1 << 2, //NOTE: I am a fan of having the control to make write-only memory, but please name one situation where that is needed.
    OS_MEM_PROTECT_EXECUTE              = 1 << 3,
    OS_MEM_PROTECT_EXECUTE_READ         = OS_MEM_PROTECT_EXECUTE | OS_MEM_PROTECT_READ,
    OS_MEM_PROTECT_EXECUTE_READWRITE    = OS_MEM_PROTECT_EXECUTE | OS_MEM_PROTECT_READWRITE,
};

typedef os_u32 os_mem_alloc_intents;
typedef os_u32 os_mem_free_intents;
typedef os_u32 os_mem_protect_intents;

LIBOS_EXPORT void os_mem_init(void);

LIBOS_EXPORT os_size os_mem_getpagesize(void);
LIBOS_EXPORT os_size os_mem_getlargepagesize(void);
LIBOS_EXPORT os_size os_mem_aligntopagesize(os_size x);
LIBOS_EXPORT os_size os_mem_aligntolargepagesize(os_size x);

LIBOS_EXPORT os_result os_mem_allocate(void** out_pointer, void* start_address, os_size size, os_mem_alloc_intents alloc_intents, os_mem_protect_intents protect_intents);
LIBOS_EXPORT os_result os_mem_free(void* address, os_size size, os_mem_free_intents free_intents);

#ifdef __cplusplus
}
#endif

#endif // LIBOS_MEMORY_H
