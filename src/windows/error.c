// Copyright 2026 Jannik Laugmand Bülow

#include "libos/_/windows_helpers.h"

#include "libos/error.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

os_string os_allocate_message__(os_size size);

os_i64 os_last_platform_error(void) {
    return (os_i64) GetLastError();
}

os_string os_platform_error_getname(os_i64 error) {
    return os_allocate_message__(0);
}

os_string os_platform_error_describe(os_i64 error) {
    microsoft_string microsoft_desc = NULL;

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        0,
        (LPWSTR) &microsoft_desc,
        0,
        NULL
    );

    os_i32 length = os_microsoft_string_to_string__(microsoft_desc, NULL, 0);
    os_string desc = os_allocate_message__(length);
    os_microsoft_string_to_string__(microsoft_desc, desc, length);

    LocalFree(microsoft_desc);

    if (desc == NULL) return os_allocate_message__(0);
    return desc;
}

os_result os_map_platform_error__(void) {
    switch (GetLastError()) {
        case ERROR_SUCCESS:
            return OS_OK;

        case ERROR_FILE_NOT_FOUND:          // 2  – named file absent
        case ERROR_PATH_NOT_FOUND:          // 3  – one or more path components absent
        case ERROR_INVALID_DRIVE:           // 15 – drive letter not present
        case ERROR_BAD_UNIT:               // 20 – device not present
        case ERROR_DEV_NOT_EXIST:          // 55 – network resource no longer available
        case ERROR_BAD_NETPATH:            // 53 – UNC path not reachable
        case ERROR_BAD_NET_NAME:           // 67 – network share name not found
        case ERROR_MOD_NOT_FOUND:          // 126 – LoadLibrary: DLL/module not found
        case ERROR_PROC_NOT_FOUND:         // 127 – GetProcAddress: export not in module
        case ERROR_NO_MORE_ITEMS:          // 259 – enumeration exhausted (treat as not-found)
        case ERROR_NO_MORE_FILES:          // 18  – FindNextFile exhausted
        case ERROR_ENVVAR_NOT_FOUND:       // 203 – environment variable absent
        case ERROR_DELETE_PENDING:         // 303 – file is already queued for deletion
        case ERROR_DEVICE_UNREACHABLE:     // 321 – device path exists but is unreachable
        case ERROR_SEM_NOT_FOUND:          // 187 – named semaphore does not exist
            return OS_ERROR_NOT_FOUND;

        case ERROR_FILE_EXISTS:            // 80  – CreateFile with CREATE_NEW
        case ERROR_ALREADY_EXISTS:         // 183 – general "already exists"
        case ERROR_ALREADY_ASSIGNED:       // 85  – drive letter already in use
        case ERROR_DUP_NAME:               // 52  – duplicate name on network
        case ERROR_NOTIFICATION_GUID_ALREADY_DEFINED: // 309
            return OS_ERROR_ALREADY_EXISTS;

        case ERROR_ACCESS_DENIED:          // 5   – principal lacks permission
        case ERROR_SHARING_VIOLATION:      // 32  – file open by another process
        case ERROR_LOCK_VIOLATION:         // 33  – byte-range lock conflict
        case ERROR_NETWORK_ACCESS_DENIED:  // 65  – network ACL denial
        case ERROR_WRITE_PROTECT:          // 19  – media or file is read-only
        case ERROR_INVALID_PASSWORD:       // 86  – bad credentials
        case ERROR_NOT_OWNER:              // 288 – mutex/object not owned by caller
        case ERROR_DRIVE_LOCKED:           // 108 – disk locked by another process
        case ERROR_LOCK_FAILED:            // 167 – LockFile could not lock the region
        case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED: // 174 – FS can't change lock type atomically
        case ERROR_OPLOCK_NOT_GRANTED:     // 300 – opportunistic lock denied
        case ERROR_FILE_CHECKED_OUT:       // 220 – checked out by another user
        case ERROR_CHECKOUT_REQUIRED:      // 221 – file must be checked out first
        case ERROR_FORMS_AUTH_REQUIRED:    // 224 – web location requires auth
        case ERROR_VIRUS_INFECTED:         // 225 – AV blocked the file
        case ERROR_VIRUS_DELETED:          // 226 – AV removed the file
        case ERROR_NOT_ALLOWED_ON_SYSTEM_FILE: // 313 – operation forbidden on FS internal file
        case ERROR_CANCEL_VIOLATION:       // 173 – cancel on non-existent lock
            return OS_ERROR_ACCESS_DENIED;

        case ERROR_NOT_ENOUGH_MEMORY:      // 8   – kernel pool exhausted
        case ERROR_OUTOFMEMORY:            // 14  – heap/storage exhausted
        case ERROR_NO_MORE_SEARCH_HANDLES: // 113 – kernel handle table full
        case ERROR_TOO_MANY_OPEN_FILES:    // 4   – process or system fd limit reached
        case ERROR_TOO_MANY_SEMAPHORES:    // 100 – kernel semaphore limit
        case ERROR_TOO_MANY_MODULES:       // 214 – loader module table full
        case ERROR_TOO_MANY_NAMES:         // 68  – NetBIOS name table full
        case ERROR_TOO_MANY_SESS:          // 69  – NetBIOS session limit
        case ERROR_TOO_MANY_TCBS:          // 155 – thread control block limit
        case ERROR_MAX_THRDS_REACHED:      // 164 – system thread limit
        case ERROR_TOO_MANY_POSTS:         // 298 – semaphore post limit
        case ERROR_OUT_OF_STRUCTURES:      // 84  – kernel structure table full
        case ERROR_IS_JOIN_PATH:           // 147 – insufficient resources (reuses this code)
        case ERROR_DEVICE_NO_RESOURCES:    // 322 – target device resource exhaustion
        case ERROR_DISK_RESOURCES_EXHAUSTED: // 314 – physical disk resource limit
        case ERROR_EA_TABLE_FULL:          // 277 – extended attribute table full
            return OS_ERROR_NO_MEMORY;

        case ERROR_NOT_SUPPORTED:          // 50  – generic "not supported"
        case ERROR_CALL_NOT_IMPLEMENTED:   // 120 – function absent on this Windows SKU
        case ERROR_BAD_DRIVER_LEVEL:       // 119 – driver doesn't support the IOCTL level
        case ERROR_INVALID_FUNCTION:       // 1   – IOCTL not valid for this device
        case ERROR_NOT_SAME_DEVICE:        // 17  – cross-device move not possible
        case ERROR_CANNOT_COPY:            // 266 – copy functions not applicable
        case ERROR_EAS_NOT_SUPPORTED:      // 282 – FS doesn't support extended attributes
        case ERROR_SEEK_ON_DEVICE:         // 132 – seeking not supported on this device
        case ERROR_DIRECT_ACCESS_HANDLE:   // 130 – raw disk handle used inappropriately
        case ERROR_NEGATIVE_SEEK:          // 131 – seek before file start
        case ERROR_BAD_FILE_TYPE:          // 222 – file type is policy-blocked
        case ERROR_FILE_TOO_LARGE:         // 223 – exceeds FS or policy size limit
        case ERROR_DISK_TOO_FRAGMENTED:    // 302 – FS can't satisfy due to fragmentation
        case ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED: // 326 – device doesn't support TRIM
        case ERROR_OFFSET_ALIGNMENT_VIOLATION:    // 327 – offset not aligned (FILE_FLAG_NO_BUFFERING)
        case ERROR_NOT_REDUNDANT_STORAGE:  // 333 – storage is not redundant
        case ERROR_DEVICE_FEATURE_NOT_SUPPORTED:  // 316 – device command not supported
        case ERROR_IMAGE_SUBSYSTEM_NOT_PRESENT:   // 308 – required image subsystem absent
        case ERROR_INVALID_LOCK_RANGE:     // 307 – lock range is invalid for this FS
        case ERROR_SHORT_NAMES_NOT_ENABLED_ON_VOLUME: // 305
            return OS_ERROR_NOT_SUPPORTED;

        case ERROR_INVALID_PARAMETER:      // 87  – generic bad parameter
        case ERROR_BAD_ARGUMENTS:          // 160 – one or more arguments incorrect
        case ERROR_INVALID_HANDLE:         // 6   – handle is closed or wrong type
        case ERROR_INVALID_TARGET_HANDLE:  // 114 – internal file identifier wrong
        case ERROR_INVALID_NAME:           // 123 – path syntax invalid
        case ERROR_BAD_PATHNAME:           // 161 – path semantically invalid
        case ERROR_FILENAME_EXCED_RANGE:   // 206 – path/filename too long
        case ERROR_BUFFER_OVERFLOW:        // 111 – filename too long (different source)
        case ERROR_INVALID_ACCESS:         // 12  – access code is invalid
        case ERROR_INVALID_DATA:           // 13  – data is structurally invalid
        case ERROR_INVALID_FLAG_NUMBER:    // 186 – flag value is invalid
        case ERROR_INSUFFICIENT_BUFFER:    // 122 – caller's buffer too small
        case ERROR_INVALID_FIELD_IN_PARAMETER_LIST: // 328 – IOCTL field invalid
        case ERROR_BAD_LENGTH:             // 24  – command length incorrect
        case ERROR_INVALID_CATEGORY:       // 117 – IOCTL code not valid
        case ERROR_INVALID_EA_NAME:        // 254 – bad extended attribute name
        case ERROR_INVALID_EA_HANDLE:      // 278 – bad EA handle
        case ERROR_DIRECTORY:              // 267 – directory name invalid in context
        case ERROR_PARTIAL_COPY:           // 299 – cross-process copy was incomplete
        case ERROR_INVALID_OPLOCK_PROTOCOL: // 301 – bad oplock acknowledgement
        case ERROR_INVALID_EXCEPTION_HANDLER: // 310
        case ERROR_INVALID_TOKEN:          // 315 – security token invalid
        case ERROR_DUPLICATE_PRIVILEGES:   // 311 – redundant privilege in token
            return OS_ERROR_INVALID_ARGUMENT;

        case ERROR_ARENA_TRASHED:          // 7   – heap control blocks destroyed
        case ERROR_INVALID_BLOCK:          // 9   – heap block address invalid
        case ERROR_BAD_ENVIRONMENT:        // 10  – process environment corrupt
        case ERROR_SECURITY_STREAM_IS_INCONSISTENT: // 306 – NTFS security stream corrupt
        case ERROR_DATA_CHECKSUM_ERROR:    // 323 – file stream data is corrupt
        case ERROR_EA_FILE_CORRUPT:        // 276 – EA file on volume is corrupt
        case ERROR_EA_LIST_INCONSISTENT:   // 255 – EA list internally inconsistent
        case ERROR_INVALID_SEGMENT_NUMBER: // 180 – segment number corrupt
        case ERROR_GEN_FAILURE:            // 31  – device attached to system not functioning
        case ERROR_CRC:                    // 23  – cyclic redundancy check failure (disk corrupt)
        case ERROR_READ_FAULT:             // 30  – device read error
        case ERROR_WRITE_FAULT:            // 29  – device write error
        case ERROR_SECTOR_NOT_FOUND:       // 27  – sector not found on disk
        case ERROR_SEEK:                   // 25  – drive cannot locate track (hardware)
        case ERROR_NOT_DOS_DISK:           // 26  – disk is not accessible/formatted
            return OS_ERROR_INVALID_STATE;

        case ERROR_BAD_EXE_FORMAT:             // 193 – not a valid Win32 PE
        case ERROR_BAD_FORMAT:                 // 11  – program has incorrect format
        case ERROR_INVALID_EXE_SIGNATURE:      // 191 – PE signature wrong
        case ERROR_EXE_MARKED_INVALID:         // 192 – OS has flagged the image invalid
        case ERROR_EXE_MACHINE_TYPE_MISMATCH:  // 216 – wrong architecture (x86 vs x64 etc.)
        case ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY:        // 217 – signed binary
        case ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY: // 218 – strong-signed binary
        case ERROR_INVALID_MODULETYPE:         // 190 – module type wrong for this context
        case ERROR_INVALID_STARTING_CODESEG:   // 188 – code segment invalid
        case ERROR_INVALID_STACKSEG:           // 189 – stack segment invalid
        case ERROR_INVALID_ORDINAL:            // 182 – ordinal in PE is invalid
        case ERROR_ITERATED_DATA_EXCEEDS_64k:  // 194 – iterated data too large
        case ERROR_INVALID_MINALLOCSIZE:       // 195 – minimum allocation size wrong
        case ERROR_AUTODATASEG_EXCEEDS_64k:    // 199 – auto data segment too large
        case ERROR_RELOC_CHAIN_XEEDS_SEGLIM:   // 201 – relocation chain overflow
        case ERROR_INFLOOP_IN_RELOC_CHAIN:     // 202 – circular relocation chain
        case ERROR_DYNLINK_FROM_INVALID_RING:  // 196 – dynlink from wrong privilege ring
        case ERROR_IOPL_NOT_ENABLED:           // 197 – IOPL required but not enabled
        case ERROR_RING2SEG_MUST_BE_MOVABLE:   // 200 – ring-2 segment constraint
            return OS_ERROR_INVALID_FORMAT;

        case ERROR_DLL_INIT_FAILED:            // 1114 – DllMain returned FALSE
        case ERROR_DLL_INIT_FAILED_LOGOFF:     // 1115 – DllMain failed during logoff
            return OS_ERROR_INITIALIZATION_FAILED;

        case ERROR_DEPENDENT_SERVICES_RUNNING: // 1051 – services depend on this one
        case ERROR_DLL_NOT_FOUND:              // 1157 – import DLL missing
            return OS_ERROR_DEPENDENCY_FAILED;

        case ERROR_HANDLE_DISK_FULL:       // 39
        case ERROR_DISK_FULL:              // 112
            return OS_ERROR_INVALID_STATE;

        default:
            return OS_UNKNOWN_ERROR;
    }
}
