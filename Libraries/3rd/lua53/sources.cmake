## sources.cmake
##
## Copyright (C) 2022-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

list(APPEND liblua_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblua-version.h

    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lapi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lauxlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lauxlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lbaselib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lbitlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lcode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lcode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lcorolib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lctype.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lctype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ldblib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ldebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ldebug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ldo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ldo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ldump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lfunc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lgc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lgc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/linit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liolib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/llex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/llex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/llimits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lmathlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lmem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/loadlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lobject.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lopcodes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lopcodes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/loslib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lparser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lprefix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lstate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lstate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lstring.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lstring.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lstrlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ltable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ltable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ltablib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ltm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ltm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lua.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/luaconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lualib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lundump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lundump.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lutf8lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lvm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lvm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lzio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lzio.h
    ${configured_headers}
)

list(APPEND lua_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lua.c
    ${configured_headers}
)

configure_file(
    miktex-liblua-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-lua-version.h
)

list(APPEND lua_sources ${CMAKE_CURRENT_BINARY_DIR}/miktex-lua-version.h)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND lua_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-lua.rc
    )
endif()

list(APPEND luac_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/luac.c
    ${configured_headers}
)

configure_file(
    miktex-liblua-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-luac-version.h
)

list(APPEND luac_sources ${CMAKE_CURRENT_BINARY_DIR}/miktex-lua-version.h)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND luac_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-luac.rc
    )
endif()
