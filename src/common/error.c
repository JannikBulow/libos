// Copyright 2026 Jannik Laugmand Bülow

#include "libos/error.h"

#include <stdlib.h>
#include <string.h>

//TODO: figure out if _Thread_local good to use or should I use TLS from the library (soontm)
_Thread_local os_result __os_last_error;

os_string __os_allocate_message(os_size length_bytes);
os_string __os_copy_message(os_string message);

os_result __os_set_and_return_error(os_result result) {
    if (result < 0) __os_last_error = result;
    return result;
}

os_result os_last_error(void) {
    return __os_last_error;
}

os_string os_result_getname(os_result res) {
    if (res > 0) res = -res;

    switch (res) {
        case OS_OK: return __os_copy_message("OK");
        case OS_UNKNOWN_ERROR: return __os_copy_message("UNKNOWN_ERROR");
        case OS_ERROR_INVALID_ARGUMENT: return __os_copy_message("INVALID_ARGUMENT");
        case OS_ERROR_INVALID_STATE: return __os_copy_message("INVALID_STATE");
        case OS_ERROR_NO_MEMORY: return __os_copy_message("NO_MEMORY");
        case OS_ERROR_ACCESS_DENIED: return __os_copy_message("ACCESS_DENIED");
        case OS_ERROR_NOT_FOUND: return __os_copy_message("NOT_FOUND");
        case OS_ERROR_ALREADY_EXISTS: return __os_copy_message("ALREADY_EXISTS");
        case OS_ERROR_NOT_SUPPORTED: return __os_copy_message("NOT_SUPPORTED");
        default: return __os_allocate_message(0);
    }
}

os_string os_result_describe(os_result res) {
    return __os_allocate_message(0); //TODO: write proper descriptions
}

// This function will automatically allocate 1 extra byte and set it and every other byte to 0.
os_string __os_allocate_message(os_size length_bytes) {
    os_string message = (os_string) malloc(length_bytes + 1);
    if (message) memset(message, 0, length_bytes + 1);
    return message;
}

os_string __os_copy_message(os_string message) {
    os_size length_bytes = strlen(message);
    os_string copy = __os_allocate_message(length_bytes);
    if (copy) memcpy(copy, message, length_bytes);
    return copy;
}

void os_free_message(os_string message) {
    if (message) free(message);
}
