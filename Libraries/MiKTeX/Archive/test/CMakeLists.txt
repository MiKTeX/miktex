## CMakeLists.txt
##
## Copyright (C) 2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/test")

set(sandbox "${CMAKE_CURRENT_BINARY_DIR}/sandbox")
set(installroot "${sandbox}/texmf")
set(dataroot "${sandbox}/localtexmf")

set(TEST_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

make_directory(${installroot}/miktex/config)
make_directory(${installroot}/tex/latex/base)
make_directory(${installroot}/tex/test/base)
make_directory(${installroot}/ab/cd/ef/xxx)
make_directory(${installroot}/jk/lm/no/xxx)
make_directory(${dataroot}/miktex/log)

set(test_sources
    ${CMAKE_SOURCE_DIR}/Libraries/MiKTeX/Core/include/miktex/Core/Test.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND test_sources
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

configure_file(
    log4cxx.xml.in
    ${installroot}/miktex/config/log4cxx.xml
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(creator)
add_subdirectory(extractor)
