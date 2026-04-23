// Copyright 2026 Jannik Laugmand Bülow

#include "libos/error.h"

#include <stdlib.h>
#include <string.h>

//TODO: figure out if _Thread_local good to use or should I use TLS from the library (soontm)
_Thread_local os_result os_last_error__;

os_string os_allocate_message__(os_size length_bytes);
os_string os_copy_message__(os_string message);

os_result os_set_and_return_result__(os_result result) {
    if (result < 0) os_last_error__ = result;
    return result;
}

os_result os_last_error(void) {
    return os_last_error__;
}

os_string os_result_getname(os_result res) {
    if (res > 0) res = -res;

    switch (res) {
        case OS_OK: return os_copy_message__("OK");
        case OS_UNKNOWN_ERROR: return os_copy_message__("UNKNOWN_ERROR");
        case OS_ERROR_INVALID_ARGUMENT: return os_copy_message__("INVALID_ARGUMENT");
        case OS_ERROR_INVALID_STATE: return os_copy_message__("INVALID_STATE");
        case OS_ERROR_NO_MEMORY: return os_copy_message__("NO_MEMORY");
        case OS_ERROR_ACCESS_DENIED: return os_copy_message__("ACCESS_DENIED");
        case OS_ERROR_NOT_FOUND: return os_copy_message__("NOT_FOUND");
        case OS_ERROR_ALREADY_EXISTS: return os_copy_message__("ALREADY_EXISTS");
        case OS_ERROR_NOT_SUPPORTED: return os_copy_message__("NOT_SUPPORTED");
        default: return os_allocate_message__(0);
    }
}

os_string os_result_describe(os_result res) {
    return os_allocate_message__(0); //TODO: write proper descriptions
}

// This function will automatically allocate 1 extra byte and set it and every other byte to 0.
os_string os_allocate_message__(os_size length_bytes) {
    os_string message = (os_string) malloc(length_bytes + 1);
    if (message) memset(message, 0, length_bytes + 1);
    return message;
}

os_string os_copy_message__(os_string message) {
    os_size length_bytes = strlen(message);
    os_string copy = os_allocate_message__(length_bytes);
    if (copy) memcpy(copy, message, length_bytes);
    return copy;
}

void os_free_message(os_string message) {
    if (message) free(message);
}
