// Copyright 2026 Jannik Laugmand Bülow

#include "libos/file.h"

#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

struct os_file {
    int fd;
    os_string path;
};

static os_file* os_stdfiles[3] = {NULL, NULL, NULL};

os_string os_copy_message__(os_cstring message);

static os_result get_std_fd(int* out_fd, os_stdfile stdfile) {
    int fd;

    switch (stdfile) {
        case OS_STDIN:
            fd = STDIN_FILENO;
            break;
        case OS_STDOUT:
            fd = STDOUT_FILENO;
            break;
        case OS_STDERR:
            fd = STDERR_FILENO;
            break;
        default:
            return OS_ERROR_INVALID_ARGUMENT;
    }

    *out_fd = fd;
    return OS_OK;
}

static os_result os_file_from_fd(os_file** out_file, int fd) {
    os_file* file = malloc(sizeof(os_file));
    if (!file) return OS_ERROR_NO_MEMORY;

    file->fd = fd;
    file->path = NULL;

    *out_file = file;
    return OS_OK;
}

static os_result os_stdfile_ensure_initialized(os_stdfile stdfile) {
    if (stdfile >= 3) return OS_ERROR_INVALID_ARGUMENT;
    if (os_stdfiles[stdfile]) return OS_OK;

    int fd;
    os_result res = get_std_fd(&fd, stdfile);
    if (res != OS_OK) return res;

    int dupfd = dup(fd);
    if (dupfd < 0) return os_map_platform_error__();

    os_file* file;
    res = os_file_from_fd(&file, dupfd);
    if (res != OS_OK) {
        close(dupfd);
        return res;
    }

    os_stdfiles[stdfile] = file;
    return OS_OK;
}

os_result os_file_get_stdfile(os_file** out_file, os_stdfile stdfile) {
    if (!out_file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    os_result res = os_stdfile_ensure_initialized(stdfile);
    if (res != OS_OK) return os_set_and_return_result__(res);

    *out_file = os_stdfiles[stdfile];
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_set_stdfile(os_stdfile stdfile, os_file* file) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    os_file* duped_file;
    os_result res = os_file_dup(&duped_file, file);
    if (res != OS_OK) return os_set_and_return_result__(res);

    int stdfd;
    res = get_std_fd(&stdfd, stdfile);
    if (res != OS_OK) {
        os_file_close(duped_file);
        return os_set_and_return_result__(res);
    }

    if (dup2(duped_file->fd, stdfd) < 0) {
        os_file_close(duped_file);
        return os_set_and_return_result__(os_map_platform_error__());
    }

    if (os_stdfiles[stdfile]) os_file_close(os_stdfiles[stdfile]);
    os_stdfiles[stdfile] = duped_file;

    return os_set_and_return_result__(OS_OK);
}

static os_result build_open_flags(os_file_open_intents intents, int* out_flags) {
    int flags = 0;

    os_bool read   = intents & OS_FILE_READ;
    os_bool write  = intents & OS_FILE_WRITE;
    os_bool append = intents & OS_FILE_APPEND;

    if (read && (write || append)) flags |= O_RDWR;
    else if (read) flags |= O_RDONLY;
    else if (write || append) flags |= O_WRONLY;
    else return OS_ERROR_INVALID_ARGUMENT;

    if (append) flags |= O_APPEND;

    if (intents & OS_FILE_CREATE) flags |= O_CREAT;

    if (intents & OS_FILE_TRUNCATE) flags |= O_TRUNC;

    if (intents & OS_FILE_EXCLUSIVE) flags |= O_EXCL;

#ifdef O_SYNC
    if (intents & OS_FILE_SYNC) flags |= O_SYNC;
#endif

#if PLATFORM_LINUX
#ifdef O_DIRECT
    if (intents & OS_FILE_DIRECT) flags |= O_DIRECT;
#endif
#endif

    *out_flags = flags;
    return OS_OK;
}

os_result os_file_open(os_file** out_file, os_cstring path, os_file_open_intents intents) {
    if (!out_file || !path) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    os_string copy = os_copy_message__(path);
    if (!copy) return os_set_and_return_result__(OS_ERROR_NO_MEMORY);

    int flags;
    os_result res = build_open_flags(intents, &flags);
    if (res != OS_OK) {
        os_free_message(copy);
        return res;
    }

    int fd = open(path, flags, 0666);
    if (fd < 0) {
        os_free_message(copy);
        return os_set_and_return_result__(os_map_platform_error__());
    }

#ifdef POSIX_FADV_RANDOM
    if (intents & OS_FILE_RANDOM_ACCESS) posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);
#endif
#ifdef POSIX_FADV_SEQUENTIAL
    if (intents & OS_FILE_SEQUENTIAL) posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);
#endif

    if (intents & (OS_FILE_DELETE_ON_CLOSE | OS_FILE_TEMPORARY)) {
        unlink(path);
    }

    os_file* file = malloc(sizeof(os_file));
    if (!file) {
        close(fd);
        os_free_message(copy);
        return os_set_and_return_result__(os_map_platform_error__());
    }

    file->fd = fd;
    file->path = copy;

    *out_file = file;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_dup(os_file** out_file, os_file* file) {
    if (!out_file || !file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    int fd = dup(file->fd);
    if (fd < 0) return os_set_and_return_result__(os_map_platform_error__());

    os_string path = os_copy_message__(file->path);
    if (!path) {
        close(fd);
        return os_set_and_return_result__(OS_ERROR_NO_MEMORY);
    }

    os_file* new_file = malloc(sizeof(os_file));
    if (!new_file) {
        close(fd);
        os_free_message(path);
        return os_set_and_return_result__(OS_ERROR_NO_MEMORY);
    }

    new_file->fd = fd;
    new_file->path = path;

    *out_file = new_file;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_close(os_file* file) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    close(file->fd);
    os_free_message(file->path);
    free(file);

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_read(os_file* file, void* buffer, os_size bytes_to_read, os_size* out_bytes_read) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    ssize_t r = read(file->fd, buffer, bytes_to_read);
    if (r < 0) return os_set_and_return_result__(os_map_platform_error__());

    LIBOS_OUT__(out_bytes_read) = (os_size) r;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_write(os_file* file, const void* buffer, os_size bytes_to_write, os_size* out_bytes_written) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    ssize_t r = write(file->fd, buffer, bytes_to_write);
    if (r < 0) return os_set_and_return_result__(os_map_platform_error__());

    LIBOS_OUT__(out_bytes_written) = (os_size) r;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_read_at(os_file* file, os_u64 offset, void* buffer, os_size bytes_to_read, os_size* out_bytes_read) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    ssize_t r = pread(file->fd, buffer, bytes_to_read, offset);
    if (r < 0) return os_set_and_return_result__(os_map_platform_error__());

    LIBOS_OUT__(out_bytes_read) = (os_size) r;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_write_at(os_file* file, os_u64 offset, const void* buffer, os_size bytes_to_write, os_size* out_bytes_written) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    ssize_t r = write(file->fd, buffer, bytes_to_write);
    if (r < 0) return os_set_and_return_result__(os_map_platform_error__());

    LIBOS_OUT__(out_bytes_written) = (os_size) r;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_seek(os_file* file, os_i64 offset, os_file_seek_origin origin) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    int whence;
    switch (origin) {
        case OS_FILE_SEEK_BEGIN:
            whence = SEEK_SET;
            break;
        case OS_FILE_SEEK_CURRENT:
            whence = SEEK_CUR;
            break;
        case OS_FILE_SEEK_END:
            whence = SEEK_END;
            break;
        default:
            return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);
    }

    if (lseek(file->fd, offset, whence) == (off_t) -1) return os_set_and_return_result__(os_map_platform_error__());
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_tell(os_file* file, os_size* out_position) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    off_t position = lseek(file->fd, 0, SEEK_CUR);
    if (position == (off_t) -1) return os_set_and_return_result__(os_map_platform_error__());

    LIBOS_OUT__(out_position) = (os_size) position;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_flush(os_file* file) {
#if PLATFORM_MACOS
    if (fcntl(file->fd, F_FULLFSYNC) == -1) return os_set_and_return_result__(os_map_platform_error__());
#else
    if (fsync(file->fd) == -1) return os_set_and_return_result__(os_map_platform_error__());
#endif

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_getinfo(os_file* file, os_file_info* out_info) {
    return os_set_and_return_result__(OS_ERROR_NOT_SUPPORTED); // TODO: implement this (i'm lazy rn)
}

os_result os_file_getsize(os_file* file, os_size* out_size) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    struct stat st;
    if (fstat(file->fd, &st) != 0) return os_set_and_return_result__(os_map_platform_error__());

    LIBOS_OUT__(out_size) = (os_size) st.st_size;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_resize(os_file* file, os_size new_size) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    off_t position = lseek(file->fd, 0, SEEK_CUR);

    if (ftruncate(file->fd, (off_t) new_size)) return os_set_and_return_result__(os_map_platform_error__());
    if (position > (off_t) new_size) position = (off_t) new_size;

    lseek(file->fd, position, SEEK_SET);

    return os_set_and_return_result__(OS_OK);
}

os_result os_file_delete(os_cstring path) {
    if (unlink(path) == -1) return os_set_and_return_result__(os_map_platform_error__());
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_rename(os_cstring path, os_cstring new_path) {
    if (rename(path, new_path) == -1) return os_set_and_return_result__(os_map_platform_error__());
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_get_platform_handle(os_file* file, void** out_platform_handle) {
    if (!file) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    LIBOS_OUT__(out_platform_handle) = (void*) (intptr_t) file->fd;
    return os_set_and_return_result__(OS_OK);
}

os_result os_file_to_cfile(FILE** out_cfile, os_file* file, const char* mode) {
    if (!out_cfile || !file || !mode) return os_set_and_return_result__(OS_ERROR_INVALID_ARGUMENT);

    int dupfd = dup(file->fd);
    if (dupfd < 0) return os_set_and_return_result__(os_map_platform_error__());

    FILE* cfile = fdopen(dupfd, mode);
    if (!cfile) {
        close(dupfd);
        return os_set_and_return_result__(os_map_platform_error__());
    }

    *out_cfile = cfile;
    return os_set_and_return_result__(OS_OK);
}