## CMakeLists.txt
##
## Copyright (C) 2013-2022 Christian Schenk
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

add_definitions(
    -DHAVE_CONFIG_H
    -DHAVE_FALLBACK
    -DHB_NO_MT
)

set(HAVE_ROUND 1)

set(HAVE_GRAPHITE2 1)
set(HAVE_OT 1)

set(HAVE_CXX11 1)

if(MIKTEX_NATIVE_WINDOWS)
    set(HAVE_WINDOWS_H 1)
endif()

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
    ${HB_BASE_sources}
    ${HB_BASE_RAGEL_GENERATED_sources}

    ${HB_BASE_headers}

    ${HB_GRAPHITE2_sources}
    ${HB_GRAPHITE2_headers}
)

set(CMAKE_C_VISIBILITY_PRESET protected)
set(CMAKE_CXX_VISIBILITY_PRESET protected)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)
