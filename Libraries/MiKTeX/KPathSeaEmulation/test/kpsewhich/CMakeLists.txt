## CMakeLists.txt
##
## Copyright (C) 2015-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/kpsemu/kpsewhich")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
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

configure_file(
    miktex-kpsewhich-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-kpsewhich-version.h
)

set(kpsewhich_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-kpsewhich-version.h
    ${MIKTEX_LIBRARY_WRAPPER}
    source/kpsewhich.c
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-kpsewhich.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-kpsewhich.rc
    )
    list(APPEND kpsewhich_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-kpsewhich.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(${MIKTEX_PREFIX}kpsewhich ${kpsewhich_sources})

set_property(TARGET ${MIKTEX_PREFIX}kpsewhich PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set(libs
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND libs
        ${utf8wrap_dll_name}
    )
endif()

if(NOT HAVE_GETOPT_LONG)
    list(APPEND libs ${getopt_dll_name})
endif()

target_link_libraries(${MIKTEX_PREFIX}kpsewhich ${libs})

install(TARGETS ${MIKTEX_PREFIX}kpsewhich DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
