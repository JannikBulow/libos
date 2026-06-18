// Copyright 2026 Jannik Laugmand Bülow

#include "libos/memory.h"

#include <sys/mman.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static os_size page_size;
static os_size large_page_size;

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
    page_size = sysconf(_SC_PAGESIZE);

    // time for cancer code because unix is good
    int meminfo = open("/proc/meminfo", O_RDONLY);
    if (meminfo < 0) {
        large_page_size = 0;
        goto skip_large_page_size;
    }

    //TODO: loop until file is fully read. this should suffice for now but just in case it won't for other people yk
    char buf[8192];
    ssize_t r = read(meminfo, buf, sizeof(buf) - 1);
    if (r <= 0) {
        close(meminfo);
        large_page_size = 0;
        goto skip_large_page_size;
    }

    buf[r] = '\0';

    char* line = strstr(buf, "Hugepagesize:");
    if (line) {
        large_page_size = strtoull(line + strlen("Hugepagesize:"), NULL, 10);
    } else {
        large_page_size = 0;
    }

    close(meminfo);

    skip_large_page_size:

#ifndef LIBOS_OPT_ASSUME_POW2_PAGESIZE
    if (page_size && ((page_size & (page_size - 1)) == 0)) {
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

os_size os_mem_getpagesize(void) {
    return page_size;
}

os_size os_mem_getlargepagesize(void) {
    return large_page_size;
}

os_size os_mem_aligntopagesize(os_size x) {
#ifdef LIBOS_OPT_ASSUME_POW2_PAGESIZE
    return alignto_pow2(x, page_size);
#else
    return aligntopagesize(x, page_size);
#endif
}

os_size os_mem_aligntolargepagesize(os_size x) {
#ifdef LIBOS_OPT_ASSUME_POW2_PAGESIZE
    return alignto_pow2(x, large_page_size);
#else
    return aligntolargepagesize(x, large_page_size);
#endif
}

static int translate_protect_intents(os_mem_protect_intents intents) {
    if (intents & OS_MEM_PROTECT_NOACCESS) {
        return PROT_NONE;
    }

    int prot = 0;
    if (intents & OS_MEM_PROTECT_READ) prot |= PROT_READ;
    if (intents & OS_MEM_PROTECT_READWRITE) prot |= PROT_READ | PROT_WRITE;
    if (intents & OS_MEM_PROTECT_EXECUTE) prot |= PROT_EXEC;

    return prot;
}

os_result os_mem_allocate(void** out_pointer, void* start_address, os_size size, os_mem_alloc_intents alloc_intents, os_mem_protect_intents protect_intents) {
    if (!out_pointer || size == 0) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    if (!(alloc_intents & (OS_MEM_RESERVE | OS_MEM_COMMIT))) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    if (alloc_intents & OS_MEM_LARGE_PAGES) {
        if (!((alloc_intents & OS_MEM_RESERVE) && (alloc_intents & OS_MEM_COMMIT))) {
            return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
        }
    }

    if ((alloc_intents & OS_MEM_COMMIT) && !(alloc_intents & OS_MEM_RESERVE)) {
        if (!start_address)
            return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

        int prot = translate_protect_intents(protect_intents);
        if (mprotect(start_address, size, prot) != 0)
            return os_set_and_return_result__(os_map_platform_error__());

        *out_pointer = start_address;
        return os_set_and_return_result__(OS_OK);
    }

    int prot  = translate_protect_intents(protect_intents);
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;

    if (!(alloc_intents & OS_MEM_COMMIT)) {
        prot = PROT_NONE;
    }

    if (alloc_intents & OS_MEM_LARGE_PAGES) {
#if defined(PLATFORM_LINUX)
        flags |= MAP_HUGETLB;
#elif defined(PLATFORM_BSD)
        flags |= MAP_ALIGNED_SUPER;
#elif defined(PLATFORM_MACOS)
        // Superpage support is unreliable and absent on Apple Silicon. Just ignore this intent and let it allocate. Normally.
        // Maybe make it return -OS_ERROR_NOT_SUPPORTED instead?
        (void) 0;
#endif
    }

    void* pointer = mmap(start_address, size, prot, flags, -1, 0);
    if (pointer == MAP_FAILED) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    *out_pointer = pointer;
    return os_set_and_return_result__(OS_OK);
}

os_result os_mem_free(void* address, os_size size, os_mem_free_intents free_intents) {
    if (!address || size == 0) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    if (free_intents == OS_MEM_DECOMMIT) {
#if defined(PLATFORM_LINUX) && defined(MADV_FREE)
        if (madvise(address, size, MADV_FREE) != 0)
#else
        if (madvise(address, size, MADV_DONTNEED) != 0)
#endif
            return os_set_and_return_result__(os_map_platform_error__());

        return os_set_and_return_result__(OS_OK);
    } else if (free_intents == OS_MEM_RELEASE) {
        if (munmap(address, size) != 0) return os_set_and_return_result__(os_map_platform_error__());
        return os_set_and_return_result__(OS_OK);
    } else {
        return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
    }
}

os_result os_mem_protect(void* address, os_size size, os_mem_protect_intents protect_intents) {
    if (!address || size == 0) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    int prot = translate_protect_intents(protect_intents);
    if (mprotect(address, size, prot) != 0) return os_set_and_return_result__(os_map_platform_error__());

    return os_set_and_return_result__(OS_OK);
}

os_result os_mem_flush_instruction_cache(const void* address, os_size size) {
    // This doesn't have a kernel call like on Windows, so we use GCC intrinsics to achieve the same result. More compiler support in 2030.
    char* begin = (char*) address;
    char* end = begin + size;
    __builtin___clear_cache(begin, end);
    return os_set_and_return_result__(OS_OK);
}
