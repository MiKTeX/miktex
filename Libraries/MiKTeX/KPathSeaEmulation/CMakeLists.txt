## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/kpsemu")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(public_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/KPSE/Emulation
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/KPSE/Emulation.h
)

set(kpathsea_includes
    absolute.h
    c-auto.h
    c-ctype.h
    c-dir.h
    c-errno.h
    c-fopen.h
    c-limits.h
    c-memstr.h
    c-minmax.h
    c-pathch.h
    c-pathmx.h
    c-proto.h
    c-stat.h
    c-std.h
    c-unistd.h
    c-vararg.h
    cnf.h
    concatn.h
    config.h
    debug.h
    dirent.h
    expand.h
    getopt.h
    kpathsea.h
    lib.h
    line.h
    magstep.h
    paths.h
    pathsearch.h
    proginit.h
    progname.h
    readable.h
    recorder.h
    simpletypes.h
    str-list.h
    systypes.h
    tex-file.h
    tex-glyph.h
    tex-hush.h
    tex-make.h
    types.h
    variable.h
    version.h
)

foreach(h ${kpathsea_includes})
    configure_file(
        include/kpathsea/kpathsea.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/kpathsea/${h}
    )
endforeach()

if(INSTALL_MIKTEX_HEADERS)
    install(
        FILES
        include/miktex/KPSE/Emulation
        include/miktex/KPSE/Emulation.h
        DESTINATION
        ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/KPSE
    )
endif()

configure_file(
    kpsemu-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/kpsemu-version.h
)

set(kpsemu_sources
    ${public_headers}
    ${CMAKE_CURRENT_BINARY_DIR}/kpsemu-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kpsemu.cpp
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)
