cmake_minimum_required(VERSION 3.29)
project(libos C)

include(GenerateExportHeader)

set(HEADERS
    include/libos/_/windows_helpers.h
    include/libos/defines.h
    include/libos/error.h
    include/libos/init.h
    include/libos/memory.h
)

set(COMMON_SOURCES
    src/common/error.c
    src/common/init.c
)

set(WINDOWS_SOURCES
    src/windows/error.c
    src/windows/helpers.c
    src/windows/memory.c
)

set(UNIX_SOURCES)

if(WIN32)
    set(SOURCES ${COMMON_SOURCES} ${WINDOWS_SOURCES})
elseif(UNIX)
    set(SOURCES ${COMMON_SOURCES} ${UNIX_SOURCES})
else()
    message(FATAL_ERROR "This library can't compile for whatever fuckass platform you're on")
endif()

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(libos-shared SHARED ${SOURCES} ${HEADERS})
add_library(libos-static STATIC ${SOURCES} ${HEADERS})

generate_export_header(libos-shared
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/libos/api.h
    EXPORT_MACRO_NAME LIBOS_EXPORT
    NO_EXPORT_MACRO_NAME LIBOS_NO_EXPORT
    STATIC_DEFINE LIBOS_STATIC_DEFINE
)

target_compile_definitions(libos-static PUBLIC LIBOS_STATIC_DEFINE)

target_include_directories(libos-shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(libos-static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(libos-shared PUBLIC c_std_11)
target_compile_features(libos-static PUBLIC c_std_11)

set_target_properties(libos-shared PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

set_target_properties(libos-static PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

if(WIN32)
    target_compile_definitions(libos-shared PUBLIC PLATFORM_WINDOWS)
    target_compile_definitions(libos-static PUBLIC PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(libos-shared PUBLIC PLATFORM_MACOS)
    target_compile_definitions(libos-static PUBLIC PLATFORM_MACOS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(libos-shared PUBLIC PLATFORM_LINUX)
    target_compile_definitions(libos-static PUBLIC PLATFORM_LINUX)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(libos-shared PRIVATE BITS_64)
    target_compile_definitions(libos-static PRIVATE BITS_64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_compile_definitions(libos-shared PRIVATE BITS_32)
    target_compile_definitions(libos-static PRIVATE BITS_32)
endif()