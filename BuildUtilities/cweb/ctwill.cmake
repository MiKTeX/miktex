## ctwill.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(ctwill_sources
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/ctwill.c
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-ctwill-version.h
    texlive/help.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/ctwill.c
    PROPERTIES LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ctwill_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-ctwill.rc
    )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/ctwill.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctwill-w2c.ch
        ${CMAKE_CURRENT_BINARY_DIR}/ctwill.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/comm-w2c.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctwill-w2c.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/prod-twill.w
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_executable(${MIKTEX_PREFIX}ctwill ${ctwill_sources})

set_property(TARGET ${MIKTEX_PREFIX}ctwill PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}ctwill
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}ctwill ${utf8wrap_dll_name})
endif()

install(
    TARGETS
        ${MIKTEX_PREFIX}ctwill
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)
