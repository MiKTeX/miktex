## CMakeLists.txt
##
## Copyright (C) 2010-2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/w2cemu")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/W2C/Emulation
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/W2C/Emulation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/W2C/pre.h
)

configure_file(
    include/w2c/c-auto.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/w2c/c-auto.h
)

configure_file(
    w2cemu-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/w2cemu-version.h
)

set(w2cemu_sources
    ${CMAKE_CURRENT_BINARY_DIR}/include/w2c/c-auto.h
    ${CMAKE_CURRENT_BINARY_DIR}/w2cemu-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/c-auto.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cpascal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/lib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/w2c/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/w2cemu.cpp
    ${public_headers}
)

if(INSTALL_MIKTEX_HEADERS)
    install(
        FILES
            ${public_headers}
        DESTINATION
            ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/W2C
    )
endif()

source_group(Public FILES ${public_headers})

add_subdirectory(shared)
