## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/TeXjp")

include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(euptex)
add_subdirectory(uptex)
