// Copyright 2026 Jannik Laugmand Bülow

#include "libos/error.h"

#include <errno.h>
#include <string.h>

os_string os_copy_message__(os_cstring message);

os_i64 os_last_platform_error(void) {
    return (os_i64) errno;
}

os_string os_platform_error_describe(os_i64 error) {
    return os_copy_message__(strerror((int) error));
}

os_result os_map_platform_error__(void) {
    return OS_UNKNOWN_ERROR; // TODO: implement
}