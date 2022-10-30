## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/pdfTeX")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

if(MSVC)
    add_definitions(-D_UNICODE -DUNICODE)
endif()

add_subdirectory(pdftex)
add_subdirectory(pdftosrc)
add_subdirectory(ttf2afm)
