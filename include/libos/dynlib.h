// Copyright 2026 Jannik Laugmand Bülow

#ifndef LIBOS_DYNLIB_H
#define LIBOS_DYNLIB_H 1

#include "libos/defines.h"
#include "libos/error.h"

#ifdef __cplusplus
extern "C" {
#endif

enum os_dynlib_load_intents_enum {
    OS_DYNLIB_LAZY      = 1 << 0,
    OS_DYNLIB_EAGER     = 1 << 1,

    OS_DYNLIB_LOCAL     = 1 << 2,
    OS_DYNLIB_GLOBAL    = 1 << 3,

    OS_DYNLIB_NODELETE  = 1 << 4,
};

typedef os_u32 os_dynlib_load_intents;

typedef struct os_dynlib os_dynlib;

LIBOS_EXPORT os_result os_dynlib_load(os_dynlib** out_lib, os_cstring path, os_dynlib_load_intents intents);
LIBOS_EXPORT os_result os_dynlib_unload(os_dynlib* lib);
LIBOS_EXPORT os_result os_dynlib_get_symbol(os_dynlib* lib, os_cstring symbol, void** out_symbol);

#ifdef __cplusplus
}
#endif

#endif //LIBOS_DYNLIB_H