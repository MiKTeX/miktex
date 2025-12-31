## CMakeLists.txt
##
## Copyright (C) 2007-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/archive")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Archive/vi/Version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Archive/Extractor
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Archive/Extractor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Archive/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Archive/vi/Version
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    archive-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/archive-version.h
)

configure_file(
    include/miktex/Archive/vi/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Archive/vi/Version.h
)

set(archive_sources
    ${CMAKE_CURRENT_BINARY_DIR}/archive-version.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CabExtractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CabExtractor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Extractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TarBzip2Extractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TarBzip2Extractor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TarExtractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TarExtractor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TarLzmaExtractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TarLzmaExtractor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vi/Runtime.cpp
    ${public_headers}
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
