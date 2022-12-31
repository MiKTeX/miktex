## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/popt")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
    -DHAVE_CONFIG_H
)

configure_file(
    miktex-popt-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-popt-version.h
)


set(public_headers
    popt.h
)

foreach(h ${public_headers})
    configure_file(
        source/src/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()

set(popt_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/popt-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/popt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/popt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/poptconfig.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/popthelp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/poptint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/poptint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/poptparse.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/system.h
)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
