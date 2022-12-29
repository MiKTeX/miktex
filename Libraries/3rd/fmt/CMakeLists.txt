## CMakeLists.txt
##
## Copyright (C) 2018-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/fmt")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

set(fmt_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/args.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/chrono.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/color.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/compile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/format-inl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/format.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/os.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/ostream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/printf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/ranges.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/std.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/fmt/xchar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/format.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/os.cc
)

if(FALSE)
    list(ADD fmt_sources ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fmt.cc)
endif()

configure_file(
    miktex-fmt-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-fmt-version.h
)

list(APPEND fmt_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-fmt-version.h
)

add_subdirectory(shared)
add_subdirectory(static)
