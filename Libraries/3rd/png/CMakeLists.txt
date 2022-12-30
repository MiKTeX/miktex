## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/png")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(interface_definitions
    -DHAVE_LIBPNG=1
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

add_definitions(
    -DPNG_NO_CONFIG_H
)

configure_file(
    source/scripts/pnglibconf.h.prebuilt
    ${CMAKE_CURRENT_BINARY_DIR}/include/pnglibconf.h
)

configure_file(
    miktex-libpng-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-libpng-version.h
)

set(png_sources
    ${CMAKE_CURRENT_BINARY_DIR}/include/pnglibconf.h
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-libpng-version.h

    ${CMAKE_CURRENT_SOURCE_DIR}/source/png.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/png.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngerror.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngget.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngpread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngpriv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngrio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngrtran.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngrutil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngtrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwtran.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwutil.c
)

add_subdirectory(shared)
