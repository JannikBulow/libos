// Copyright 2026 Jannik Laugmand Bülow

#ifndef LIBOS_MEMORY_H
#define LIBOS_MEMORY_H 1

#include "libos/defines.h"

#ifdef __cplusplus
extern "C" {
#endif

enum os_mem_alloc_intents {
    OS_MEM_RESERVE      = 1 << 0,
    OS_MEM_COMMIT       = 1 << 1,
    OS_MEM_LARGE_PAGES  = 1 << 2,
    OS_MEM_ZERO         = 1 << 3,
};

LIBOS_EXPORT void os_mem_init(void);

LIBOS_EXPORT os_size os_mem_getpagesize(void);
LIBOS_EXPORT os_size os_mem_getlargepagesize(void);
LIBOS_EXPORT os_size os_mem_aligntopagesize(os_size x);
LIBOS_EXPORT os_size os_mem_aligntolargepagesize(os_size x);

#ifdef __cplusplus
}
#endif

#endif // LIBOS_MEMORY_H
