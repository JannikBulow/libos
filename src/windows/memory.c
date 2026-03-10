// Copyright 2026 Jannik Laugmand Bülow

#include "libos/memory.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static SYSTEM_INFO si;

#ifndef LIBOS_OPT_ASSUME_POW2_PAGESIZE
static os_size (*aligntopagesize)(os_size, os_size) = NULL;
static os_size (*aligntolargepagesize)(os_size, os_size) = NULL;

static os_size alignto_normal(os_size x, os_size a) {
    return ((x + a - 1) / a) * a;
}
#endif

static os_size alignto_pow2(os_size x, os_size a) {
    return (x + a - 1) & ~(a - 1);
}

void os_mem_init(void) {
    GetSystemInfo(&si);
    os_size large_page_size = (os_size) GetLargePageMinimum();

#ifndef LIBOS_OPT_ASSUME_POW2_PAGESIZE
    if (si.dwPageSize && ((si.dwPageSize & (si.dwPageSize - 1)) == 0)) {
        aligntopagesize = alignto_pow2;
    } else {
        aligntopagesize = alignto_normal;
    }

    if (large_page_size && ((large_page_size & (large_page_size - 1)) == 0)) {
        aligntolargepagesize = alignto_pow2;
    } else {
        aligntolargepagesize = alignto_normal;
    }
#endif
}

size_t os_mem_getpagesize(void) {
    return si.dwPageSize;
}

os_size os_mem_getlargepagesize(void) {
    return GetLargePageMinimum();
}

size_t os_mem_aligntopagesize(size_t x) {
#ifdef LIBOS_OPT_ASSUME_POW2_PAGESIZE
    return aligntopagesize_pow2(x, si.dwPageSize);
#else
    return aligntopagesize(x, si.dwPageSize);
#endif
}

os_size os_mem_aligntolargepagesize(os_size x) {
#ifdef LIBOS_OPT_ASSUME_POW2_PAGESIZE
    return aligntopagesize_pow2(x, GetLargePageMinimum())
#else
    return aligntopagesize(x, GetLargePageMinimum());
#endif
}
