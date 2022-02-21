## ctangle.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(ctangle_sources
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-ctangle-version.h
    texlive/help.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    PROPERTIES LANGUAGE CXX
)

set_source_files_properties(
    ${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ctangle_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-ctangle.rc
    )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    COMMAND
        initctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctang-w2c.ch
        ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctang-w2c.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
        initctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/common.c
        ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
    COMMAND
        initctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
        ${CMAKE_CURRENT_SOURCE_DIR}/source/comm-w2c.ch
        ${CMAKE_CURRENT_BINARY_DIR}/common.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/comm-w2c.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
        initctangle
    VERBATIM
)

add_executable(${MIKTEX_PREFIX}ctangle ${ctangle_sources})

set_property(TARGET ${MIKTEX_PREFIX}ctangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}ctangle
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

install(
    TARGETS
        ${MIKTEX_PREFIX}ctangle
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)
