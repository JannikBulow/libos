// Copyright 2026 Jannik Laugmand Bülow

#include "libos/dynlib.h"

#include <dlfcn.h>
#include <unistd.h>

#include <stdlib.h>

struct os_dynlib {
    void* handle;
};

os_result os_dynlib_load(os_dynlib** out_lib, os_cstring path, os_dynlib_load_intents intents) {
    if (!out_lib || !path) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
    if (intents == 0) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    if ((intents & (OS_DYNLIB_LAZY | OS_DYNLIB_EAGER)) == 0) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    int flags = 0;
    if (intents & OS_DYNLIB_LAZY) flags |= RTLD_LAZY;
    if (intents & OS_DYNLIB_EAGER) flags |= RTLD_NOW;
    if (intents & OS_DYNLIB_LOCAL) flags |= RTLD_LOCAL;
    if (intents & OS_DYNLIB_GLOBAL) flags |= RTLD_GLOBAL;

#ifdef RTLD_NODELETE
    if (intents & OS_DYNLIB_NODELETE) flags |= RTLD_NODELETE;
#else
    if (intents & OS_DYNLIB_NODELETE) return os_set_and_return_result__(OS_ERROR_NOT_SUPPORTED);
#endif

    dlerror();

    void* handle = dlopen(path, flags);
    if (!handle) return os_set_and_return_result__(os_map_platform_error__());

    os_dynlib* lib = malloc(sizeof(os_dynlib));
    if (!lib) {
        dlclose(handle);
        return os_set_and_return_result__(OS_ERROR_NO_MEMORY);
    }

    lib->handle = handle;

    *out_lib = lib;
    return os_set_and_return_result__(OS_OK);
}

os_result os_dynlib_unload(os_dynlib* lib) {
    if (!lib) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    void* handle = lib->handle;

    free(lib);

    if (dlclose(handle) != 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    return os_set_and_return_result__(OS_OK);
}

os_result os_dynlib_get_symbol(os_dynlib* lib, os_cstring symbol, void** out_symbol) {
    if (!lib || !symbol) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    dlerror();

    void* address = dlsym(lib->handle, symbol);

    if (dlerror() != NULL) { // fuck posix
        return os_set_and_return_result__(OS_ERROR_NOT_FOUND);
    }

    LIBOS_OUT__(out_symbol) = address;
    return os_set_and_return_result__(OS_OK);
}