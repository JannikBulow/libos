// Copyright 2026 Jannik Laugmand Bülow

#ifndef LIBOS_WINDOWS_HELPERS_H
#define LIBOS_WINDOWS_HELPERS_H 1

#ifndef PLATFORM_WINDOWS
#error "You shouldn't even include this to begin with :)"
#endif

#include "libos/defines.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef LPWSTR microsoft_string;

LIBOS_EXPORT microsoft_string __os_string_to_microsoft_string(os_cstring s, microsoft_string (*alloc)(os_size), void (*dealloc)(microsoft_string));
LIBOS_EXPORT os_string __os_microsoft_string_to_string(microsoft_string s, os_string (*alloc)(os_size), void (*dealloc)(os_string));

#ifdef __cplusplus
}
#endif

#endif // LIBOS_WINDOWS_HELPERS_H
