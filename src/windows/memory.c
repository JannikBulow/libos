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

// Converts intents into a 3-bit mask, then checks a table.
// R W X
static os_result translate_protect_intents(DWORD* out_flags, os_mem_protect_intents intents) {
    static DWORD win_protect_table[8] = {
        PAGE_NOACCESS, // ---
        PAGE_READONLY, // R--
        PAGE_READWRITE, // -W-   write implies read in this basic ahh table
        PAGE_READWRITE, // RW-
        PAGE_EXECUTE, // --X
        PAGE_EXECUTE_READ, // R-X
        PAGE_EXECUTE_READWRITE, // -WX
        PAGE_EXECUTE_READWRITE, // RWX
    };

    os_u8 mask =
            ((intents & OS_MEM_PROTECT_READ) ? 1 : 0) |
            ((intents & OS_MEM_PROTECT_READWRITE) ? 2 : 0) |
            ((intents & OS_MEM_PROTECT_EXECUTE) ? 4 : 0);

    __LIBOS_OUT(out_flags) = win_protect_table[mask];
    return OS_OK;
}

os_result os_mem_allocate(void** out_pointer, void* start_address, os_size size, os_mem_alloc_intents alloc_intents, os_mem_protect_intents protect_intents) {
    DWORD alloc = 0;
    DWORD prot = 0;

    os_result res = translate_protect_intents(&prot, alloc_intents);
    if (res != OS_OK) return res;

    if (alloc_intents & OS_MEM_LARGE_PAGES) {
        if (!((alloc_intents & OS_MEM_RESERVE) && (alloc_intents & OS_MEM_COMMIT))) {
            return OS_ERROR_INVALID_ARGUMENT;
        }
        alloc = MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES;
    } else {
        if (alloc_intents & OS_MEM_RESERVE) alloc |= MEM_RESERVE;
        if (alloc_intents & OS_MEM_COMMIT) alloc |= MEM_COMMIT;
    }

    if (alloc == 0) return OS_ERROR_INVALID_ARGUMENT;

    void* pointer = VirtualAlloc(start_address, size, alloc, prot);
    if (pointer == NULL) {
        DWORD error = GetLastError();
        switch (error) {
            case ERROR_NOT_ENOUGH_MEMORY: return OS_ERROR_NO_MEMORY;

            case ERROR_BAD_ARGUMENTS:
            case ERROR_INVALID_PARAMETER:
            case ERROR_INVALID_ADDRESS:
                return OS_ERROR_INVALID_ARGUMENT;

            default: return OS_UNKNOWN_ERROR;
        }
    }

    *out_pointer = pointer;

    return OS_OK;
}

os_result os_mem_free(void* address, os_size size, os_mem_free_intents free_intents) {
    WINBOOL res;
    if (free_intents == OS_MEM_DECOMMIT) {
        res = VirtualFree(address, size, MEM_DECOMMIT);
    } else if (free_intents == OS_MEM_RELEASE) {
        res = VirtualFree(address, size, MEM_RELEASE);
    } else {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (res == 0) { // nonzero = error for VirtualFree
        DWORD error = GetLastError();
        switch (error) {
            case ERROR_INVALID_PARAMETER:
            case ERROR_BAD_ARGUMENTS:
            case ERROR_INVALID_ADDRESS:
                return OS_ERROR_INVALID_ARGUMENT;

            default: return OS_UNKNOWN_ERROR;
        }
    }

    return OS_OK;
}
