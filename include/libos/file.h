// Copyright 2026 Jannik Laugmand Bülow

#ifndef LIBOS_FILE_H
#define LIBOS_FILE_H 1

#include "libos/defines.h"
#include "libos/error.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {

#endif

typedef struct os_file os_file;

enum os_stdfile_enum {
    OS_STDIN = 0,
    OS_STDOUT = 1,
    OS_STDERR = 2
};

enum os_file_open_intents_enum {
    OS_FILE_READ = 1 << 0,
    OS_FILE_WRITE = 1 << 1,
    OS_FILE_APPEND = 1 << 2,

    OS_FILE_CREATE = 1 << 3,
    OS_FILE_TRUNCATE = 1 << 4,
    OS_FILE_EXCLUSIVE = 1 << 5,

    OS_FILE_TEMPORARY = 1 << 6,

    OS_FILE_RANDOM_ACCESS = 1 << 7,
    OS_FILE_SEQUENTIAL = 1 << 8,

    OS_FILE_DIRECT = 1 << 9,
    OS_FILE_SYNC = 1 << 10,

    OS_FILE_DELETE_ON_CLOSE = 1 << 11,
};

enum os_file_seek_origin_enum {
    OS_FILE_SEEK_BEGIN = 1 << 0,
    OS_FILE_SEEK_CURRENT = 1 << 1,
    OS_FILE_SEEK_END = 1 << 2,
};

enum os_file_flags_enum { // TODO: real flags
    OS_FILE_FLAG_DIRECTORY = 1 << 0,
    OS_FILE_FLAG_READONLY = 1 << 1,
    OS_FILE_FLAG_HIDDEN = 1 << 2,
    OS_FILE_FLAG_SYMLINK = 1 << 3,
};

typedef struct os_file_info {
    os_cstring path;
    os_size size;
    os_i64 creation_time; // unix time
    os_i64 access_time; // unix time
    os_i64 modification_time; // unix time
    os_u32 flags;
} os_file_info;

typedef os_u32 os_file_open_intents;
typedef os_u32 os_file_seek_origin;
typedef os_u32 os_stdfile;

LIBOS_EXPORT os_result os_file_get_stdfile(os_file** out_file, os_stdfile stdfile);
LIBOS_EXPORT os_result os_file_set_stdfile(os_stdfile stdfile, os_file* file);

LIBOS_EXPORT os_result os_file_open(os_file** out_file, os_cstring path, os_file_open_intents intents);
LIBOS_EXPORT os_result os_file_dup(os_file** out_file, os_file* file);
LIBOS_EXPORT os_result os_file_close(os_file* file);

LIBOS_EXPORT os_result os_file_read(os_file* file, void* buffer, os_size bytes_to_read, os_size* out_bytes_read);

LIBOS_EXPORT os_result os_file_write(os_file* file, const void* buffer, os_size bytes_to_write, os_size* out_bytes_written);

LIBOS_EXPORT os_result os_file_read_at(os_file* file, os_u64 offset, void* buffer, os_size bytes_to_read, os_size* out_bytes_read);

LIBOS_EXPORT os_result os_file_write_at(os_file* file, os_u64 offset, const void* buffer, os_size bytes_to_write, os_size* out_bytes_written);

LIBOS_EXPORT os_result os_file_seek(os_file* file, os_i64 offset, os_file_seek_origin origin);

LIBOS_EXPORT os_result os_file_tell(os_file* file, os_size* out_position);

// Don't think that this means libos does buffering. This is only to call the OS flush functions if present
LIBOS_EXPORT os_result os_file_flush(os_file* file);

LIBOS_EXPORT os_result os_file_getinfo(os_file* file, os_file_info* out_info);

LIBOS_EXPORT os_result os_file_getsize(os_file* file, os_size* out_size);

LIBOS_EXPORT os_result os_file_resize(os_file* file, os_size new_size);

LIBOS_EXPORT os_result os_file_delete(os_cstring path);

LIBOS_EXPORT os_result os_file_rename(os_cstring path, os_cstring new_path);

LIBOS_EXPORT os_result os_file_get_platform_handle(os_file* file, void** out_platform_handle);

LIBOS_EXPORT os_result os_file_to_cfile(FILE** out_cfile, os_file* file, const char* mode);

#ifdef __cplusplus
}
#endif

#endif // LIBOS_FILE_H
