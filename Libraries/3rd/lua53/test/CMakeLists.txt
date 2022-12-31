## CMakeLists.txt
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/test")

add_test(
    NAME lua_1
    COMMAND $<TARGET_FILE:${MIKTEX_PREFIX}lua> ${CMAKE_CURRENT_SOURCE_DIR}/1.lua
)

add_test(lua_1_okay ${DIFF_EXECUTABLE} 1.out ${CMAKE_CURRENT_SOURCE_DIR}/1.good.out)
