## CMakeLists.txt
##
## Copyright (C) 2014-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETUP_FOLDER}")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

configure_file(
    miktexsetup-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktexsetup-version.h
)

set(miktexsetup_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktexsetup-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/miktexsetup.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        miktexsetup.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktexsetup.rc
    )
    list(APPEND miktexsetup_sources
        ${MIKTEX_COMMON_MANIFEST}
        ${CMAKE_CURRENT_BINARY_DIR}/miktexsetup.rc
    )
endif()

add_subdirectory(integrated)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(standalone)
endif()
