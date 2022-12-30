## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/getopt")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/posix
)

set(public_include_directories ${CMAKE_CURRENT_BINARY_DIR}/include)

set(public_headers
    bits/getopt_core.h
    bits/getopt_ext.h
    getopt.h
)

foreach(h ${public_headers})
    configure_file(
        source/posix/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()

configure_file(
    miktex-getopt-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-getopt-version.h
)

set(getopt_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-getopt-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt_int.h
)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
