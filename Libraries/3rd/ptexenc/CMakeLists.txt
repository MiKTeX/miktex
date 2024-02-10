## CMakeLists.txt
##
## Copyright (C) 2021-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/ptexenc")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(ptexenc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptexenc-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jisx0208.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanjicnv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ptexenc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/unicode-jp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/unicode.c
)

set(headers
    kanjicnv.h
    ptexenc.h
    unicode-jp.h
    unicode.h
)

foreach(hdr ${headers})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ptexenc/${hdr}
        ${CMAKE_CURRENT_BINARY_DIR}/include/ptexenc/${hdr}
        COPYONLY
    )
    list(APPEND ptexenc_sources ${CMAKE_CURRENT_BINARY_DIR}/include/ptexenc/${hdr})
endforeach()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/c-auto.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/ptexenc/c-auto.h
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/c-auto.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/c-auto.h
)

configure_file(
    miktex-ptexenc-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptexenc-version.h
)

add_subdirectory(shared)

set(ptekf_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ptekf.c
)

add_executable(${MIKTEX_PREFIX}ptekf ${ptekf_sources})

set_property(TARGET ${MIKTEX_PREFIX}ptekf PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}ptekf
    ${app_dll_name}
    ${kpsemu_dll_name}
    ${ptexenc_dll_name}
)

if(MIKTEX_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}ptekf
        ${unxemu_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}ptekf DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
