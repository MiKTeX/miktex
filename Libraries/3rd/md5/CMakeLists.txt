## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/md5")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(md5_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-md5-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/md5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/md5.h
)

if(INSTALL_MIKTEX_HEADERS)
    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/source/miktex-md5.h
        DESTINATION
            ${MIKTEX_HEADER_DESTINATION_DIR}
    )
endif()

configure_file(
    miktex-md5-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-md5-version.h
)

configure_file(
    source/md5.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/md5.h
)

set(CMAKE_C_VISIBILITY_PRESET hidden)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
