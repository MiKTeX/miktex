## CMakeLists.txt
##
## Copyright (C) 2008-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/uriparser")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

add_definitions(
    -DURI_NO_UNICODE
    -DURI_PASS_ANSI
)

configure_file(
    source/src/UriConfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/UriConfig.h
)

configure_file(
    miktex-uriparser-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-uriparser-version.h
)

set(uriparser_sources
    ${CMAKE_CURRENT_BINARY_DIR}/UriConfig.h
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-uriparser-version.h

    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/uriparser/Uri.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/uriparser/UriBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/uriparser/UriDefsAnsi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/uriparser/UriDefsConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/uriparser/UriDefsUnicode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/uriparser/UriIp4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriCommon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriCommon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriCompare.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriEscape.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriFile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriIp4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriIp4Base.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriIp4Base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriMemory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriMemory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriNormalize.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriNormalizeBase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriNormalizeBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriParse.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriParseBase.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriParseBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriQuery.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriRecompose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriResolve.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriShorten.c
)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
