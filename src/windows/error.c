// Copyright 2026 Jannik Laugmand Bülow

#include "libos/_/windows_helpers.h"

#include "libos/error.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

os_string __os_allocate_message(os_size size);

os_i64 os_last_platform_error(void) {
    return (os_i64) GetLastError();
}

os_string os_platform_error_getname(os_i64 error) {
    return __os_allocate_message(0);
}

os_string os_platform_error_describe(os_i64 error) {
    microsoft_string microsoft_desc = NULL;

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        0,
        (LPWSTR) &microsoft_desc,
        0,
        NULL
    );
    os_string desc = __os_microsoft_string_to_string(microsoft_desc, __os_allocate_message, os_free_message);
    LocalFree(microsoft_desc);

    if (desc == NULL) return __os_allocate_message(0);
    return desc;
}
