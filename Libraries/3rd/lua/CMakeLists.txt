## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/lua")

include_directories(
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_LUA_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_definitions(-DLUA_COMPAT_MODULE)

ignore_warnings()

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

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND lua_sources
      lua.rc
    )
    list(APPEND luac_sources
      luac.rc
    )
  endif()
  add_executable(${MIKTEX_PREFIX}lua ${lua_sources})
  set_property(TARGET ${MIKTEX_PREFIX}lua PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  merge_trustinfo_manifest(${MIKTEX_PREFIX}lua asInvoker)
  merge_compatibility_manifest(${MIKTEX_PREFIX}lua)
  target_link_libraries(${MIKTEX_PREFIX}lua ${lua_dll_name})
  install(TARGETS ${MIKTEX_PREFIX}lua DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endif()

add_subdirectory(static)

add_executable(${MIKTEX_PREFIX}luac ${luac_sources})

set_property(TARGET ${MIKTEX_PREFIX}luac PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}luac asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}luac)

target_link_libraries(${MIKTEX_PREFIX}luac ${lua_lib_name})

install(TARGETS ${MIKTEX_PREFIX}luac DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
