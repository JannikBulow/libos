// Copyright 2026 Jannik Laugmand Bülow

#include "libos/error.h"

//TODO: figure out if _Thread_local good to use or should I use TLS from the library (soontm)
_Thread_local os_result __os_last_error;

os_result __os_set_and_return_error(os_result result) {
    if (result < 0) __os_last_error = result;
    return result;
}

os_result os_last_error(void) {
    return __os_last_error;
}

os_string os_result_getname(os_result res) {
    return NULL; //TODO: implement
}

os_string os_result_describe(os_result res) {
    return NULL;
}

void os_free_message(os_string message) {
}
