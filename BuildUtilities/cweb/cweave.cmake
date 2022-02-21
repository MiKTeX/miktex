## cweave.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(cweave_sources
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-cweave-version.h
    texlive/help.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    PROPERTIES LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND cweave_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-cweave.rc
    )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweav-w2c.ch
        ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweav-w2c.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
        ${CMAKE_CURRENT_SOURCE_DIR}/source/prod.w
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_executable(${MIKTEX_PREFIX}cweave ${cweave_sources})

set_property(TARGET ${MIKTEX_PREFIX}cweave PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}cweave
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

install(
    TARGETS
        ${MIKTEX_PREFIX}cweave
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)
