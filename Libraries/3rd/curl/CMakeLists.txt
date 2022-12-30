## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/curl")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib
)

configure_file(
    miktex-curl-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-curl-version.h
)

list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/miktex-curl-version.h)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

add_definitions(-DBUILDING_LIBCURL)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(-DENABLE_IPV6)
    add_definitions(-DUSE_SCHANNEL)
    add_definitions(-DUSE_WINDOWS_SSPI)
    remove_definitions(-DHAVE_CONFIG_H)
endif()

include(sources.cmake)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
