## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/unxemu")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/unxemu-config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/unxemu-config.h
)

set(interface_definitions
    -DHAVE_DIRENT_H=1
    -DHAVE_UNISTD_H=1
)
  
set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(unx_includes
    dirent.h
    unistd.h
    sys/time.h
)

foreach(inc ${unx_includes})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/unx.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/${inc}
    )
endforeach()

configure_file(
    unxemu-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/unxemu-version.h
)

set(public_headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/unxemu-config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/unxemu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sysexits.h
)

set(lib_sources
    ${public_headers}
    ${CMAKE_CURRENT_BINARY_DIR}/unxemu-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/unxemu.cpp
)

install(
    FILES
        ${public_headers}
    DESTINATION
        ${MIKTEX_HEADER_DESTINATION_DIR}/miktex
)

foreach(inc ${unx_includes})
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/include/${inc}
    DESTINATION
        ${MIKTEX_HEADER_DESTINATION_DIR}
    )
endforeach()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
