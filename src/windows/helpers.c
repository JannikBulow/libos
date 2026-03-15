// Copyright 2026 Jannik Laugmand Bülow

#include "libos/_/windows_helpers.h"

microsoft_string __os_string_to_microsoft_string(os_cstring s, microsoft_string (*alloc)(os_size), void (*dealloc)(microsoft_string)) {
    if (!s) return NULL;

    DWORD length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    if (length == 0) return NULL;

    microsoft_string buffer = alloc(sizeof(*buffer) * length);
    if (!buffer) return NULL;

    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, buffer, length)) {
        dealloc(buffer);
        return NULL;
    }

    return buffer;
}

os_string __os_microsoft_string_to_string(microsoft_string s, os_string (*alloc)(os_size), void (*dealloc)(os_string)) {
    if (!s) return NULL;

    DWORD length = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
    if (length == 0) return NULL;

    os_string buffer = alloc(sizeof(*buffer) * length);
    if (!buffer) return NULL;

    if (!WideCharToMultiByte(CP_UTF8, 0, s, -1, buffer, length, NULL, NULL)) {
        dealloc(buffer);
        return NULL;
    }

    return buffer;
}
