## CMakeLists.txt
##
## Copyright (C) 2001-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/expat")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_definitions(-DHAVE_EXPAT_CONFIG_H)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(-DCOMPILED_FROM_DSP)
endif()

set(public_headers
    expat.h
    expat_external.h
)

foreach(h ${public_headers})
    configure_file(
        source/lib/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()

include(sources.cmake)

add_subdirectory(shared)
add_subdirectory(static)
