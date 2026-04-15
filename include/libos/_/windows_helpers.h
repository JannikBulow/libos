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
typedef LPCWSTR microsoft_cstring;

LIBOS_EXPORT os_i32 os_string_to_microsoft_string__(os_cstring s, microsoft_string buffer, os_i32 buffer_size);
LIBOS_EXPORT os_i32 os_microsoft_string_to_string__(microsoft_cstring s, os_string buffer, os_i32 buffer_size);

#ifdef __cplusplus
}
#endif

#endif // LIBOS_WINDOWS_HELPERS_H
