## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}/CWeb")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive
)

add_definitions(
    -Dmain=Main
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

include(initctangle.cmake)
include(ctangle.cmake)
include(ctwill.cmake)
include(ctwill-refsort.cmake)
include(ctwill-twinx.cmake)
include(cweave.cmake)
