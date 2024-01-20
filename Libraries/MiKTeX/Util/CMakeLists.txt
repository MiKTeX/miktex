## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/util")

set(util_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(util_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(headers_no_ext
    miktex/Util/CharBuffer
    miktex/Util/DateUtil
    miktex/Util/OptionSet
    miktex/Util/PathName
    miktex/Util/PathNameParser
    miktex/Util/PathNameUtil
    miktex/Util/StringUtil
    miktex/Util/Tokenizer
)

foreach(h ${headers_no_ext})
    set(_path ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
    list(APPEND public_headers_no_ext ${_path})
    if (NOT EXISTS ${_path})
        file(WRITE ${_path}
            "#pragma once\n#include <${h}.h>\n"
        )
    endif()
endforeach()

set(public_headers
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Util/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/CharBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/DateUtil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/OptionSet.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/PathName.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/PathNameParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/PathNameUtil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/StringUtil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/Tokenizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/inliners.h
    ${public_headers_no_ext}
)

set(MIKTEX_UTIL_PATHNAME_SIZE 260)

configure_file(
    include/miktex/Util/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Util/config.h
)

configure_file(
    util-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/util-version.h
)

set(component_sources
    ${CMAKE_CURRENT_BINARY_DIR}/util-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PathName/PathName.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PathNameParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PathNameUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StringUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Tokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${public_headers}
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND component_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/PathName/win/winPathName.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/winHelpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/winPathNameUtil.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/winStringUtil.cpp
    )
else()
    list(APPEND component_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/PathName/unx/unxPathName.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/unx/unxHelpers.cpp
    )
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
