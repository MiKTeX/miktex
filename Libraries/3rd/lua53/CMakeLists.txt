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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/lua53")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_compile_definitions
  -DLUAI_HASHLIMIT=6
  -DLUA_COMPAT_5_2
  -DLUA_COMPAT_MODULE
)

if(MIKTEX_UNIX_ALIKE)
  list(APPEND public_compile_definitions
    -DLUA_USE_DLOPEN
    -DLUA_USE_POSIX
  )
endif()

configure_file(
  liblua-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/liblua-version.h
)

set(public_headers
  lapi.h
  lauxlib.h
  lcode.h
  lctype.h
  ldebug.h
  ldo.h
  lfunc.h
  lgc.h
  llex.h
  llimits.h
  lmem.h
  lobject.h
  lopcodes.h
  lparser.h
  lstate.h
  lstring.h
  ltable.h
  ltm.h
  lua.h
  lua.hpp
  luaconf.h
  lualib.h
  lundump.h
  lvm.h
  lzio.h
)

foreach(h ${public_headers})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

include(sources.cmake)

set(CMAKE_C_VISIBILITY_PRESET hidden)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
  set(lua53_target_library ${lua53_dll_name})
else()
  set(lua53_target_library ${lua53_lib_name})
endif()

add_subdirectory(static)

generate_export_header(${lua53_target_library}
  BASE_NAME MIKTEX_LUA
  EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/lua/export.h
  STATIC_DEFINE MIKTEX_LUA_STATIC
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND lua_sources
    ${MIKTEX_COMMON_MANIFEST}
    lua.rc
  )
  list(APPEND luac_sources
    ${MIKTEX_COMMON_MANIFEST}
    luac.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}lua ${lua_sources})
set_property(TARGET ${MIKTEX_PREFIX}lua PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PREFIX}lua ${lua53_target_library})
install(TARGETS ${MIKTEX_PREFIX}lua DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_executable(${MIKTEX_PREFIX}luac ${luac_sources})
set_property(TARGET ${MIKTEX_PREFIX}luac PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PREFIX}luac ${lua53_target_library})
install(TARGETS ${MIKTEX_PREFIX}luac DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

add_subdirectory(test)
