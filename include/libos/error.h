// Copyright 2026 Jannik Laugmand Bülow

#ifndef LIBOS_ERROR_H
#define LIBOS_ERROR_H 1

#include "libos/defines.h"

#ifdef __cplusplus
extern "C" {
#endif

// If any of these values are negated (in this case positive), it indicates some sort of success, but with a minor problem.
// This means that positive values should be treated as warnings rather than errors.
enum os_error {
    OS_OK                       = 0,     // Indicates successful operation.

    OS_UNKNOWN_ERROR            = -1,   // Indicates an internal error that couldn't be accurately portrayed using an os_error. This will be used sparsely.
    OS_ERROR_INVALID_ARGUMENT   = -2,   // Indicates an invalid argument provided by the user. Common causes include NULL pointers and mismatching intents.
    OS_ERROR_INVALID_STATE      = -3,   // Indicates an invalid internal state usually caused by memory corruption or incompetent developers (both from the library developers and users).

    OS_ERROR_NO_MEMORY          = -10,  // Indicates that the host system is out of memory. This is never recoverable, and it's recommended to just clean up and exit the program when encountered.
    OS_ERROR_ACCESS_DENIED      = -11,  // Indicates that a resource was found, but the platform-dependent user or current process is missing permissions to access it.
    OS_ERROR_NOT_FOUND          = -12,  // Indicates that a resource could not be found when the intent was to access it without creation.
    OS_ERROR_ALREADY_EXISTS     = -13,  // Indicates that a resource already exists when the intent was to explicitly create a new one.
    OS_ERROR_NOT_SUPPORTED      = -14,  // Indicates that the current platform cannot support an intent. If negated, the provided intents were fulfilled using less than ideal workarounds.
};

LIBOS_EXPORT os_i64 os_last_platform_error(void);                // Returns the latest platform-dependent error code in the form of the largest possible integer.
LIBOS_EXPORT os_string os_platform_error_getname(os_i64 error);  // Returns the platform-dependent name associated with a given error code. May return an empty string if there is no name. Return value must be released using `os_free_message`.
LIBOS_EXPORT os_string os_platform_error_describe(os_i64 error); // Returns the platform-dependent description associated with a given error code. May return an empty string if there is no description. Return value must be released using `os_free_message`.

LIBOS_EXPORT os_result os_last_error(void);                      // Returns the last error
LIBOS_EXPORT os_string os_result_getname(os_result res);         // Returns the name associated with a given result (e.g. `-10` -> `"OS_ERROR_NO_MEMORY"`). Return value must be released using `os_free_message`.
LIBOS_EXPORT os_string os_result_describe(os_result res);        // Returns the description associated with a given result (e.g. `-10` -> `"OS_ERROR_NO_MEMORY"`). Return value must be released using `os_free_message`.

LIBOS_EXPORT void os_free_message(os_string message);            // The library may communicate in human-readable messages (mainly to describe errors) in string form, and these strings are copied into a unique buffer to avoid the user changing the internal description of an error. So the messages must be freed using this uniform function.

#ifdef __cplusplus
}
#endif

#endif // LIBOS_ERROR_H
