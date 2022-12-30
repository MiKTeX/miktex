## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/bzip2")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    miktex-libbz2-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-libbz2-version.h
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
    bzlib.h
)

foreach(h ${public_headers})
    configure_file(
        source/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
endforeach()

include(sources.cmake)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
