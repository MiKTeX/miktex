## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/lzma2")

include_directories(BEFORE  
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}

    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/api
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/rangecoder
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/common
)

if(MIKTEX_NATIVE_WINDOWS)
    include_directories(BEFORE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/windows/vs2019
    )
endif()

configure_file(
    miktex-liblzma-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblzma-version.h
)

list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblzma-version.h)

add_definitions(
    -DTUKLIB_SYMBOL_PREFIX=lzma_
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
    lzma.h
    lzma/base.h
    lzma/bcj.h
    lzma/block.h
    lzma/check.h
    lzma/container.h
    lzma/delta.h
    lzma/filter.h
    lzma/hardware.h
    lzma/index.h
    lzma/index_hash.h
    lzma/lzma12.h
    lzma/stream_flags.h
    lzma/version.h
    lzma/vli.h
)

foreach(h ${public_headers})
    configure_file(
        source/src/liblzma/api/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
    list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
endforeach()

include(sources.cmake)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
