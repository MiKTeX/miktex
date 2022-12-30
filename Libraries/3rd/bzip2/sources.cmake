## sources.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(bzip2_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/blocksort.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bzlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bzlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bzlib_private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/crctable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/decompress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/huffman.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/randtable.c

    ${CMAKE_CURRENT_BINARY_DIR}/miktex-libbz2-version.h
)
