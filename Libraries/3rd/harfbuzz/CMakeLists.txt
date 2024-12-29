## CMakeLists.txt
##
## Copyright (C) 2013-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)
include(sources.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/harfbuzz")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/source/src
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

add_definitions(
    -DHAVE_CONFIG_H
    -DHB_NO_MT
)

set(HAVE_GRAPHITE2 1)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

list(APPEND harfbuzz_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    miktex-harfbuzz-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-harfbuzz-version.h
)

list(APPEND harfbuzz_sources
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-harfbuzz-version.h
)

list(APPEND harfbuzz_sources
    ${project_sources}
    ${project_headers}
)

set(CMAKE_C_VISIBILITY_PRESET protected)
set(CMAKE_CXX_VISIBILITY_PRESET protected)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)
