// Copyright 2026 Jannik Laugmand Bülow

#include "libos/_/windows_helpers.h"

#include "libos/dynlib.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct os_dynlib {
    HMODULE module;
    os_bool nodelete;
};

os_result os_dynlib_load(os_dynlib** out_lib, os_cstring path, os_dynlib_load_intents intents) {
    if (!out_lib || !path) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
    if (intents == 0) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    if (intents & OS_DYNLIB_LAZY) return os_set_and_return_result__(OS_ERROR_NOT_SUPPORTED);
    if (intents & OS_DYNLIB_GLOBAL) return os_set_and_return_result__(OS_ERROR_NOT_SUPPORTED);

    HANDLE heap = GetProcessHeap();
    os_i32 length = os_string_to_microsoft_string__(path, NULL, 0);
    microsoft_string microsoft_path = HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, length * sizeof(WCHAR));
    os_string_to_microsoft_string__(path, microsoft_path, length);

    HMODULE module = LoadLibraryW(microsoft_path);
    HeapFree(heap, 0, microsoft_path);
    if (module == NULL) {
        DWORD error = GetLastError();
        switch (error) {
            case ERROR_ACCESS_DENIED:
                return os_set_and_return_result__(OS_ERROR_ACCESS_DENIED);
            case ERROR_NOT_ENOUGH_MEMORY:
                return os_set_and_return_result__(OS_ERROR_NO_MEMORY);
            case ERROR_INVALID_PARAMETER:
                return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
            case ERROR_MOD_NOT_FOUND:
            case ERROR_PROC_NOT_FOUND:
            case ERROR_DLL_NOT_FOUND:
                return os_set_and_return_result__(OS_ERROR_NOT_FOUND);
            case ERROR_BAD_EXE_FORMAT:
            case ERROR_INVALID_DLL:
                return os_set_and_return_result__(OS_ERROR_INVALID_FORMAT);
            default:
                return os_set_and_return_result__(OS_UNKNOWN_ERROR);
        }
    }

    os_dynlib* lib = HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, sizeof(os_dynlib));
    lib->module = module;
    lib->nodelete = intents & OS_DYNLIB_NODELETE;

    // No need for LIBOS_OUT__ because it has to not be NULL anyway
    *out_lib = lib;

    if (lib->nodelete) return os_set_and_return_result__(-OS_ERROR_NOT_SUPPORTED);

    return os_set_and_return_result__(OS_OK);
}

os_result os_dynlib_unload(os_dynlib* lib) {
    if (!lib) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    HANDLE heap = GetProcessHeap();

    HMODULE module = lib->module;
    os_bool nodelete = lib->nodelete;

    HeapFree(heap, 0, lib);

    if (nodelete) return os_set_and_return_result__(OS_OK);

    BOOL result = FreeLibrary(module);
    if (result == 0) {
        DWORD error = GetLastError();
        switch (error) {
            case ERROR_INVALID_HANDLE:
                return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
            default:
                return os_set_and_return_result__(OS_UNKNOWN_ERROR);
        }
    }

    return os_set_and_return_result__(OS_OK);
}

os_result os_dynlib_get_symbol(os_dynlib* lib, os_cstring symbol, void** out_symbol) {
    if (!lib || !symbol) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    FARPROC address = GetProcAddress(lib->module, symbol);
    if (address == NULL) {
        DWORD error = GetLastError();
        switch (error) {
            case ERROR_PROC_NOT_FOUND:
            case ERROR_MOD_NOT_FOUND:
                return os_set_and_return_result__(OS_ERROR_NOT_FOUND);
            case ERROR_INVALID_HANDLE:
                return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
            default:
                return os_set_and_return_result__(OS_UNKNOWN_ERROR);
        }
    }

    return os_set_and_return_result__(OS_OK);
}
