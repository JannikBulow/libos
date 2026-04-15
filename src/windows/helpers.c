// Copyright 2026 Jannik Laugmand Bülow

#include "libos/_/windows_helpers.h"

os_i32 os_string_to_microsoft_string__(os_cstring s, microsoft_string buffer, os_i32 buffer_size) {
    return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, buffer, buffer_size);
}

os_i32 os_microsoft_string_to_string__(microsoft_cstring s, os_string buffer, os_i32 buffer_size) {
    return WideCharToMultiByte(CP_UTF8, 0, s, -1, buffer, buffer_size, NULL, NULL);
}
