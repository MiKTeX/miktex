## CMakeLists.txt
##
## Copyright (C) 2013-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/pixman")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman
)

set(PIXMAN_VERSION_MAJOR ${MIKTEX_COMP_ORIG_MAJOR_VERSION})
set(PIXMAN_VERSION_MINOR ${MIKTEX_COMP_ORIG_MINOR_VERSION})
set(PIXMAN_VERSION_MICRO ${MIKTEX_COMP_ORIG_PATCH_VERSION})

set(pixman_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-access.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-access-accessors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-bits-image.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-combine32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-combine-float.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-conical-gradient.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-filter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-x86.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-mips.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-arm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-ppc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-edge.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-edge-accessors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-fast-path.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-glyph.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-general.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-gradient-walker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-image.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-implementation.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-linear-gradient.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-matrix.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-noop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-radial-gradient.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-region16.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-region32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-solid-fill.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-trap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-utils.c

    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-mmx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-sse2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-ssse3.c

    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/dither/blue-noise-64x64.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-accessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-combine32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-compiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-edge-imp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-inlines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman.h
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/pixman-version.h
)

configure_file(
    pixman-config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/pixman-config.h
)

configure_file(
    miktex-pixman-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-pixman-version.h
)

list(APPEND pixman_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-pixman-version.h
)

add_subdirectory(shared)
