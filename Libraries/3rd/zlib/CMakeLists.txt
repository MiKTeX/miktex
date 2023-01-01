## CMakeLists.txt
##
## Copyright (C) 2006-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/zlib")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_definitions(
    -DHAVE_STDDEF_H
    -DHAVE_STDINT_H
    -DHAVE_SYS_TYPES_H
    -D_LARGEFILE64_SOURCE=1
)

set(interface_definitions
    -DHAVE_LIBZ=1
    -DHAVE_ZLIB=1
    -DHAVE_ZLIB_COMPRESS2=1
    -DHAVE_ZLIB_H=1
)

set(public_headers
    zlib.h
    zconf.h
)

foreach(h ${public_headers})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()

configure_file(
    miktex-zlib-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-zlib-version.h
)

set(zlib_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/adler32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/crc32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/deflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/deflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/gzclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/gzguts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/gzlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/gzread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/gzwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/infback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inffast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inffast.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inffixed.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inftrees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inftrees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/trees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/trees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/uncompr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zutil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/zconf.h

    ${CMAKE_CURRENT_BINARY_DIR}/miktex-zlib-version.h
)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
