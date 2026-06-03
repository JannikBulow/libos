// Copyright 2026 Jannik Laugmand Bülow

#include "libos/_/windows_helpers.h"

#include "libos/dynlib.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct os_dynlib {
    HMODULE module;
};

os_result os_dynlib_load(os_dynlib** out_lib, os_cstring path, os_dynlib_load_intents intents) {
    if (!out_lib || !path) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
    if (intents == 0) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    if (intents & OS_DYNLIB_LAZY) return os_set_and_return_result__(OS_ERROR_NOT_SUPPORTED);
    if (intents & OS_DYNLIB_GLOBAL) return os_set_and_return_result__(OS_ERROR_NOT_SUPPORTED);
    if (intents & OS_DYNLIB_NODELETE) return os_set_and_return_result__(OS_ERROR_NOT_SUPPORTED);

    os_i32 length = os_string_to_microsoft_string__(path, NULL, 0);
    microsoft_string wpath = malloc(length * sizeof(WCHAR));
    if (!wpath) return os_set_and_return_result__(OS_ERROR_NO_MEMORY);

    os_string_to_microsoft_string__(path, wpath, length);

    HMODULE module = LoadLibraryW(wpath);
    free(wpath);
    if (module == NULL) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    os_dynlib* lib = malloc(sizeof(os_dynlib));
    if (!lib) {
        FreeLibrary(module);
        return os_set_and_return_result__(OS_ERROR_NO_MEMORY);
    }
    lib->module = module;

    // No need for LIBOS_OUT__ because it has to not be NULL anyway
    *out_lib = lib;

    return os_set_and_return_result__(OS_OK);
}

os_result os_dynlib_unload(os_dynlib* lib) {
    if (!lib) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    HANDLE heap = GetProcessHeap();

    HMODULE module = lib->module;

    HeapFree(heap, 0, lib);

    BOOL result = FreeLibrary(module);
    if (result == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    return os_set_and_return_result__(OS_OK);
}

os_result os_dynlib_get_symbol(os_dynlib* lib, os_cstring symbol, void** out_symbol) {
    if (!lib || !symbol) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    FARPROC address = GetProcAddress(lib->module, symbol);
    if (address == NULL) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    LIBOS_OUT__(out_symbol) = address;

    return os_set_and_return_result__(OS_OK);
}
