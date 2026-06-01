// Copyright 2026 Jannik Laugmand Bülow

#include "libos/_/windows_helpers.h"

#include "libos/file.h"

#include <stdlib.h>

struct os_file {
    HANDLE handle;
    os_string path;
};

os_string os_copy_message__(os_cstring message);

static os_result get_std_handle(DWORD* out_std_handle, HANDLE* out_handle, os_stdfile stdfile) {
    DWORD stdhandle;
    switch (stdfile) {
        case OS_STDIN:
            stdhandle = STD_INPUT_HANDLE;
            break;
        case OS_STDOUT:
            stdhandle = STD_OUTPUT_HANDLE;
            break;
        case OS_STDERR:
            stdhandle = STD_ERROR_HANDLE;
            break;
        default:
            return OS_ERROR_INVALID_ARGUMENT;
    }

    HANDLE handle = GetStdHandle(stdhandle);
    if (handle == INVALID_HANDLE_VALUE) return os_map_platform_error__();
    if (handle == NULL) return OS_ERROR_NOT_FOUND;

    if (out_std_handle) *out_std_handle = stdhandle;
    if (out_handle) *out_handle = handle;
    return OS_OK;
}

static os_result os_file_from_std_handle(os_file** out_file, HANDLE handle) {
    if (handle == INVALID_HANDLE_VALUE || handle == NULL) return OS_ERROR_NOT_FOUND;

    os_file* file = malloc(sizeof(os_file));
    if (file == NULL) return OS_ERROR_NO_MEMORY;

    file->handle = handle;
    file->path = NULL;

    *out_file = file;
    return OS_OK;
}

os_result os_file_get_stdfile(os_file** out_file, os_stdfile stdfile) {
    if (!out_file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    HANDLE raw_handle;
    os_result res = get_std_handle(NULL, &raw_handle, stdfile);
    if (res != OS_OK) return os_set_and_return_result__(res);

    HANDLE duped_handle;
    if (DuplicateHandle(GetCurrentProcess(), raw_handle, GetCurrentProcess(), &duped_handle, 0, FALSE, DUPLICATE_SAME_ACCESS) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    return os_set_and_return_result__(os_file_from_std_handle(out_file, duped_handle));
}

os_result os_file_set_stdfile(os_stdfile stdfile, os_file* file) {
    if (!file || file->handle == INVALID_HANDLE_VALUE || file->handle == NULL) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    DWORD stdhandle_id;
    os_result res = get_std_handle(&stdhandle_id, NULL, stdfile);
    if (res != OS_OK) return os_set_and_return_result__(res);

    HANDLE duped_handle;
    if (DuplicateHandle(GetCurrentProcess(), file->handle, GetCurrentProcess(), &duped_handle, 0, FALSE, DUPLICATE_SAME_ACCESS) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    if (SetStdHandle(stdhandle_id, duped_handle) == 0) {
        CloseHandle(duped_handle);
        return os_set_and_return_result__(os_map_platform_error__());
    }

    return os_set_and_return_result__(OS_OK);
}

static microsoft_string convert_path(os_cstring path) {
    os_i32 path_length = os_string_to_microsoft_string__(path, NULL, 0);
    microsoft_string result = malloc(path_length * sizeof(*result));
    if (result) {
        os_string_to_microsoft_string__(path, result, path_length * sizeof(WCHAR));
    }
    return result;
}

static os_result build_create_file_params(os_file_open_intents intents, DWORD* out_desired_access, DWORD* out_share_mode, DWORD* out_creation_disposition, DWORD* out_flags_and_attributes, BOOL*  out_delete_on_close) {
    DWORD access = 0;

    if (intents & OS_FILE_READ) access |= GENERIC_READ;
    if (intents & OS_FILE_WRITE) access |= GENERIC_WRITE;
    if (intents & OS_FILE_APPEND) access |= FILE_APPEND_DATA;

    if (access == 0) return OS_ERROR_INVALID_ARGUMENT;

    DWORD share = FILE_SHARE_READ;

    BOOL has_create = (intents & OS_FILE_CREATE) != 0;
    BOOL has_truncate = (intents & OS_FILE_TRUNCATE) != 0;
    BOOL has_exclusive = (intents & OS_FILE_EXCLUSIVE) != 0;

    if (!has_create && has_exclusive) return OS_ERROR_INVALID_ARGUMENT;

    DWORD disposition;
    if (!has_create && !has_truncate) disposition = OPEN_EXISTING;
    else if (!has_create && has_truncate) disposition = TRUNCATE_EXISTING;
    else if (has_create && !has_truncate && !has_exclusive) disposition = OPEN_ALWAYS;
    else if (has_create && has_truncate && !has_exclusive) disposition = CREATE_ALWAYS;
    else /* has_create && has_exclusive */ disposition = CREATE_NEW;

    if (disposition == TRUNCATE_EXISTING && !(access & GENERIC_WRITE))
        return OS_ERROR_INVALID_ARGUMENT;

    DWORD flags = FILE_ATTRIBUTE_NORMAL;

    if (intents & OS_FILE_RANDOM_ACCESS) flags |= FILE_FLAG_RANDOM_ACCESS;
    if (intents & OS_FILE_SEQUENTIAL) flags |= FILE_FLAG_SEQUENTIAL_SCAN;
    if (intents & OS_FILE_DIRECT) flags |= FILE_FLAG_NO_BUFFERING;
    if (intents & OS_FILE_SYNC) flags |= FILE_FLAG_WRITE_THROUGH;
    if (intents & OS_FILE_DELETE_ON_CLOSE) flags |= FILE_FLAG_DELETE_ON_CLOSE;

    if (intents & OS_FILE_TEMPORARY) {
        flags &= ~FILE_ATTRIBUTE_NORMAL;
        flags |= FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;
    }

    *out_desired_access = access;
    *out_share_mode = share;
    *out_creation_disposition = disposition;
    *out_flags_and_attributes = flags;
    *out_delete_on_close = (intents & (OS_FILE_DELETE_ON_CLOSE | OS_FILE_TEMPORARY)) != 0;
    return OS_OK;
}

os_result os_file_open(os_file** out_file, os_cstring _path, os_file_open_intents intents) {
    if (!out_file || !_path) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    os_string path = os_copy_message__(_path);
    if (!path) return os_set_and_return_result__(OS_ERROR_NO_MEMORY);

    DWORD access;
    DWORD share;
    DWORD disposition;
    DWORD flags;
    BOOL delete_on_close;
    os_result res = build_create_file_params(intents, &access, &share, &disposition, &flags, &delete_on_close);
    if (res != OS_OK) return os_set_and_return_result__(res);

    microsoft_string wpath = convert_path(path);
    if (!wpath) return OS_ERROR_NO_MEMORY;

    HANDLE handle = CreateFileW(wpath, access, share, NULL, disposition, flags, NULL);

    free(wpath);

    if (handle == INVALID_HANDLE_VALUE) return os_set_and_return_result__(os_map_platform_error__());

    os_file* file = malloc(sizeof(os_file));
    if (!file) {
        CloseHandle(handle);
        return os_set_and_return_result__(OS_ERROR_NO_MEMORY);
    }

    file->handle = handle;
    file->path = path;

    *out_file = file;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_close(os_file* file) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    free(file->path);
    if (CloseHandle(file->handle) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    free(file);

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_read(os_file* file, void* buffer, os_size bytes_to_read, os_size* out_bytes_read) {
    if (!file || !buffer) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    DWORD read = 0;
    if (ReadFile(file->handle, buffer, (DWORD) bytes_to_read, &read, NULL) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    LIBOS_OUT__(out_bytes_read) = (os_size) read;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_write(os_file* file, const void* buffer, os_size bytes_to_write, os_size* out_bytes_written) {
    if (!file || !buffer) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    DWORD written = 0;
    if (WriteFile(file->handle, buffer, (DWORD) bytes_to_write, &written, NULL) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    LIBOS_OUT__(out_bytes_written) = (os_size) written;
    return os_set_and_return_result__(OS_OK);
}

static void offset_to_overlapped(os_u64 offset, OVERLAPPED* overlapped) {
    memset(overlapped, 0, sizeof(OVERLAPPED));
    overlapped->Offset = (DWORD) (offset & 0xFFFFFFFF);
    overlapped->OffsetHigh = (DWORD) (offset >> 32);
}

os_result os_file_read_at(os_file* file, os_u64 offset, void* buffer, os_size bytes_to_read, os_size* out_bytes_read) {
    if (!file || !buffer) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    OVERLAPPED overlapped;
    offset_to_overlapped(offset, &overlapped);

    DWORD read = 0;
    if (ReadFile(file->handle, buffer, (DWORD) bytes_to_read, &read, &overlapped) == 0) {
        if (GetLastError() != ERROR_HANDLE_EOF) {
            return os_set_and_return_result__(os_map_platform_error__());
        }
    }

    LIBOS_OUT__(out_bytes_read) = (os_size) read;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_write_at(os_file* file, os_u64 offset, const void* buffer, os_size bytes_to_write, os_size* out_bytes_written) {
    if (!file || !buffer) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    OVERLAPPED overlapped;
    offset_to_overlapped(offset, &overlapped);

    DWORD written = 0;
    if (WriteFile(file->handle, buffer, (DWORD) bytes_to_write, &written, &overlapped) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    LIBOS_OUT__(out_bytes_written) = (os_size) written;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_seek(os_file* file, os_i64 offset, os_file_seek_origin origin) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    DWORD move_method;
    switch (origin) {
        case OS_FILE_SEEK_BEGIN:
            move_method = FILE_BEGIN;
            break;
        case OS_FILE_SEEK_CURRENT:
            move_method = FILE_CURRENT;
            break;
        case OS_FILE_SEEK_END:
            move_method = FILE_END;
            break;
        default:
            return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
    }

    LARGE_INTEGER distance;
    distance.QuadPart = (LONGLONG) offset;

    if (SetFilePointerEx(file->handle, distance, NULL, move_method) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_tell(os_file* file, os_size* out_position) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    LARGE_INTEGER zero = {0};
    LARGE_INTEGER position = {0};
    if (SetFilePointerEx(file->handle, zero, &position, FILE_CURRENT) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    LIBOS_OUT__(out_position) = (os_size) position.QuadPart;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_flush(os_file* file) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    if (!FlushFileBuffers(file->handle)) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    return os_set_and_return_result__(OS_OK);
}

#define FILETIME_TO_UNIX_EPOCH 116444736000000000ULL

static os_i64 filetime_to_unix_time(FILETIME filetime) {
    ULARGE_INTEGER t;
    t.LowPart = filetime.dwLowDateTime;
    t.HighPart = filetime.dwHighDateTime;
    return (os_i64) ((t.QuadPart - FILETIME_TO_UNIX_EPOCH) * 100);
}

os_result os_file_getinfo(os_file* file, os_file_info* out_info) {
    if (!file || !out_info) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    BY_HANDLE_FILE_INFORMATION information;
    if (GetFileInformationByHandle(file->handle, &information) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    out_info->path = file->path;

    ULARGE_INTEGER size;
    size.LowPart = information.nFileSizeLow;
    size.HighPart = information.nFileSizeHigh;
    out_info->size = (os_size) size.QuadPart;

    out_info->creation_time = filetime_to_unix_time(information.ftCreationTime);
    out_info->access_time = filetime_to_unix_time(information.ftLastAccessTime);
    out_info->modification_time = filetime_to_unix_time(information.ftLastWriteTime);

    os_u32 flags = 0;
    if (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) flags |= OS_FILE_FLAG_DIRECTORY;
    if (information.dwFileAttributes & FILE_ATTRIBUTE_READONLY)  flags |= OS_FILE_FLAG_READONLY;
    if (information.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)    flags |= OS_FILE_FLAG_HIDDEN;
    if (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) flags |= OS_FILE_FLAG_SYMLINK;
    out_info->flags = flags;

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_getsize(os_file* file, os_size* out_size) {
    if (!file || !out_size) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    LARGE_INTEGER size;
    if (GetFileSizeEx(file->handle, &size) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    LIBOS_OUT__(out_size) = (os_size) size.QuadPart;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_resize(os_file* file, os_size new_size) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    LARGE_INTEGER zero = {0};
    LARGE_INTEGER current_position = {0};
    if (!SetFilePointerEx(file->handle, zero, &current_position, FILE_CURRENT) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    LARGE_INTEGER target;
    target.QuadPart = (LONGLONG) new_size;
    if (SetFilePointerEx(file->handle, target, NULL, FILE_BEGIN) == 0) {
        return os_set_and_return_result__(os_map_platform_error__());
    }

    os_result res = OS_OK;
    if (SetEndOfFile(file->handle) == 0) {
        res = os_map_platform_error__();
    }

    if (current_position.QuadPart > target.QuadPart) {
        current_position = target;
    }

    SetFilePointerEx(file->handle, current_position, NULL, FILE_BEGIN);

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_delete(os_cstring path) {
    if (!path) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    microsoft_string wpath = convert_path(path);
    if (!wpath) return os_set_and_return_result__(OS_ERROR_NO_MEMORY);

    os_result res = OS_OK;
    if (DeleteFileW(wpath) == 0) res = os_map_platform_error__();

    free(wpath);

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_rename(os_cstring path, os_cstring new_path) {
    if (!path || !new_path) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    microsoft_string wpath = convert_path(path);
    if (!wpath) return os_set_and_return_result__(OS_ERROR_NO_MEMORY);

    microsoft_string wnew_path = convert_path(new_path);
    if (!wnew_path) {
        free(wpath);
        return os_set_and_return_result__(OS_ERROR_NO_MEMORY);
    }

    os_result res = OS_OK;
    if (MoveFileExW(wpath, wnew_path, MOVEFILE_REPLACE_EXISTING) == 0) {
        res = os_map_platform_error__();
    }

    free(wpath);
    free(wnew_path);

    return os_set_and_return_result__(res);
}

os_result os_file_get_platform_handle(os_file* file, void** out_platform_handle) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    LIBOS_OUT__(out_platform_handle) = (void*) file->handle;
    return os_set_and_return_result__(OS_OK);
}
