## CMakeLists.txt
##
## Copyright (C) 2011-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/utf8wrap")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/utf8wrap-config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/utf8wrap-config.h
)

configure_file(
        utf8wrap-version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/utf8wrap-version.h
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/utf8wrap-config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/utf8wrap.h
)

set(lib_sources
    ${public_headers}
    ${CMAKE_CURRENT_BINARY_DIR}/utf8wrap-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utf8wrap.cpp
)

install(
    FILES
        ${public_headers}
    DESTINATION
        ${MIKTEX_HEADER_DESTINATION_DIR}/miktex
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)
add_subdirectory(static)
