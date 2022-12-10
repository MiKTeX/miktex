## CMakeLists.txt
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_VALIDATION_FOLDER}/lacheck")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    miktex-lacheck-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-lacheck-version.h
)

add_definitions(
    -DUSE_MIKTEX_EXIT
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

set(lacheck_c_sources
    ${CMAKE_CURRENT_BINARY_DIR}/lacheck.c
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/lacheck.c
    COMMAND
        ${FLEX_EXECUTABLE}
            --noline
            -o${CMAKE_CURRENT_BINARY_DIR}/lacheck.c
            ${CMAKE_CURRENT_SOURCE_DIR}/source/lacheck.l
    DEPENDS
        source/lacheck.l
    VERBATIM
)

set(lacheck_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-lacheck-version.h
    ${MIKTEX_LIBRARY_WRAPPER}
    ${lacheck_c_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        miktex-lacheck.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-lacheck.rc
    )
    list(APPEND lacheck_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-lacheck.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}lacheck ${lacheck_sources})

set_property(TARGET ${MIKTEX_PREFIX}lacheck PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}lacheck
    ${app_dll_name}
    ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}lacheck
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}lacheck DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
