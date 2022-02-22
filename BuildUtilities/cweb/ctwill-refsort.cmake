## ctwill-refsort.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(ctwill_refsort_sources
    ${CMAKE_CURRENT_BINARY_DIR}/ctwill-refsort.c
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-ctwill-refsort-version.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/ctwill-refsort.c
    PROPERTIES LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ctwill_refsort_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-ctwill-refsort.rc
    )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/ctwill-refsort.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/refsort.w
        ${CMAKE_CURRENT_SOURCE_DIR}/source/refsort.ch
        ${CMAKE_CURRENT_BINARY_DIR}/ctwill-refsort.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/refsort.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/refsort.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/refsort.w
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_executable(${MIKTEX_PREFIX}ctwill-refsort ${ctwill_refsort_sources})

set_property(TARGET ${MIKTEX_PREFIX}ctwill-refsort PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}ctwill-refsort
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

install(
    TARGETS
        ${MIKTEX_PREFIX}ctwill-refsort
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)
