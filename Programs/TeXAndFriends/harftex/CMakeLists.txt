## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2019 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/HarfTeX")

if(MIKTEX_LINUX_DIST_CODE_NAME STREQUAL "xenial")
  set(lua53_target_name ${lua53_lib_name})
else()
  set(lua53_target_name ${lua53_dll_name})
endif()

include_directories(BEFORE
  ${CMAKE_BINARY_DIR}/${MIKTEX_REL_LUATEX_DIR}
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_LUATEX_DIR}
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_LUATEX_DIR}/source
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

add_definitions(
  -DAPPTAG=luatex
  -DMIKTEX_LUATEX
  -DNO_DUMP_SHARE
  -DSYNCTEX_ENGINE_H="synctex-luatex.h"
  -DUSE_MIKTEX_EXIT
  -DpdfTeX
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
    -DWIN32_LEAN_AND_MEAN
  )
endif()

if(WITH_CONTEXT_SUPPORT)
  add_definitions(-DWITH_CONTEXT_SUPPORT=1)
endif()

include(libharftex.cmake)
include(luafontforge.cmake)
include(luaharfbuzz.cmake)

###############################################################################
## harftex
###############################################################################

set(harftex_sources
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MP_DIR}/source/lmplib.c
  harftex-version.h
  source/luatex.c
  source/luatex.h
)

set(harftex_program_sources ${harftex_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/harftex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/harftex.rc
    )
  list(APPEND harftex_program_sources
    ${CMAKE_CURRENT_BINARY_DIR}/harftex.rc
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

set(libs
  luatex-miktex-objects
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32.lib
    wsock32.lib
  )
endif()

set(libs53 ${libs})
list(APPEND libs53
  ${lua53_target_name}
  harftex-lua53fontforge-objects
  harftex-lua53harfbuzz-objects
  harftex-lua53tex-objects
  luatex-lua53ffi-objects
  luatex-lua53misc-objects
  luatex-lua53pplib-objects
  luatex-lua53socket-objects
)

set(program_libs53 ${libs53})

add_executable(${MIKTEX_PREFIX}harftex ${harftex_program_sources} ${MIKTEX_LIBRARY_WRAPPER})

set_property(TARGET ${MIKTEX_PREFIX}harftex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}harftex
  ${program_libs53}
)
install(TARGETS ${MIKTEX_PREFIX}harftex DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
