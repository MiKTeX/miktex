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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/LuaTeX")

# FIXME: this breaks luatex under xenial:
# set(lua53_target_name ${lua53_dll_name})
if(MIKTEX_NATIVE_WINDOWS)
  set(lua53_target_name ${lua53_dll_name})
else()
  set(lua53_target_name ${lua53_dll_name})
endif()

if(WITH_LUA54TEX)
  # FIXME: this breaks luatex under xenial:
  # set(lua54_target_name ${lua54_dll_name})
  if(MIKTEX_NATIVE_WINDOWS)
    set(lua54_target_name ${lua54_dll_name})
  else()
    set(lua54_target_name ${lua54_lib_name})
  endif()
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_DIR}
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

include(miktex.cmake)
include(luasocket.cmake)
include(luamisc.cmake)
include(luapplib.cmake)
include(libunilib.cmake)
include(luafontforge.cmake)
include(luaffi.cmake)
include(libluatex.cmake)

###############################################################################
## luatex
###############################################################################

set(luatex_sources
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MP_DIR}/source/lmplib.c
  luatex-version.h
  source/luatex.c
  source/luatex.h
  source/luatex_svnversion.h
)

set(lua53tex_program_sources ${luatex_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/luatex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/luatex.rc
    )
  list(APPEND lua53tex_program_sources
    ${CMAKE_CURRENT_BINARY_DIR}/luatex.rc
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

set(libs
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${md5_dll_name}
  ${metapost_dll_name}
  ${w2cemu_dll_name}
  luatex-miktex-objects
  luatex-unilib-objects
)

if(USE_SYSTEM_PNG)
  list(APPEND libs MiKTeX::Imported::PNG)
else()
  list(APPEND libs ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  list(APPEND libs MiKTeX::Imported::ZLIB)
else()
  list(APPEND libs ${zlib_dll_name})
endif()

if(USE_SYSTEM_ZZIP)
  list(APPEND libs MiKTeX::Imported::ZZIP)
else()
  list(APPEND libs ${zzip_dll_name})
endif()

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
  luatex-lua53fontforge-objects
  luatex-lua53ffi-objects
  luatex-lua53misc-objects
  luatex-lua53pplib-objects
  luatex-lua53socket-objects
  lua53tex-objects
)

if(WITH_LUA54TEX)
  set(lua54tex_program_sources ${luatex_sources})
  if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/lua54tex.rc.in
      ${CMAKE_CURRENT_BINARY_DIR}/lua54tex.rc
    )
    list(APPEND lua54tex_program_sources
      ${CMAKE_CURRENT_BINARY_DIR}/lua54tex.rc
      ${MIKTEX_COMPATIBILITY_MANIFEST}
    )
  endif()
endif()

if(WITH_LUA54TEX)
  set(libs54 ${libs})
  list(APPEND libs54
    ${lua54_target_name}
    luatex-lua54fontforge-objects
    luatex-lua54ffi-objects
    luatex-lua54misc-objects
    luatex-lua54pplib-objects
    luatex-lua54socket-objects
    lua54tex-objects
  )
endif()

set(program_libs53 ${libs53})

if(WITH_LUA54TEX)
  set(program_libs54 ${libs54})
endif()

add_executable(${MIKTEX_PREFIX}luatex ${lua53tex_program_sources} ${MIKTEX_LIBRARY_WRAPPER})
set_property(TARGET ${MIKTEX_PREFIX}luatex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
target_link_libraries(${MIKTEX_PREFIX}luatex
  ${app_dll_name}
  ${core_dll_name}
  ${program_libs53}
)
install(TARGETS ${MIKTEX_PREFIX}luatex DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")

if(WITH_LUA54TEX)
  add_executable(${MIKTEX_PREFIX}lua54tex ${lua54tex_program_sources} ${MIKTEX_LIBRARY_WRAPPER})
  set_property(TARGET ${MIKTEX_PREFIX}lua54tex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(${MIKTEX_PREFIX}lua54tex
    ${app_dll_name}
    ${core_dll_name}
    ${program_libs54}
  )
  install(TARGETS ${MIKTEX_PREFIX}lua54tex DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
endif()

###############################################################################
## runtexlua
###############################################################################

add_executable(runtexlua ${lua53tex_program_sources} runtexlua.cpp)
set_property(TARGET runtexlua PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
if(MIKTEX_MACOS_BUNDLE)
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(APPLE)
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "@loader_path/../../${CMAKE_INSTALL_LIBDIR}")
else()
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")
endif()
target_link_libraries(runtexlua
  ${app_dll_name}
  ${core_dll_name}
  ${program_libs53}
)
install(TARGETS runtexlua DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")

if(WITH_LUA54TEX)
  add_executable(runtexlua54 ${lua54tex_program_sources} runtexlua.cpp)
  set_property(TARGET runtexlua54 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  if(MIKTEX_MACOS_BUNDLE)
    set_target_properties(runtexlua54 PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  elseif(APPLE)
    set_target_properties(runtexlua54 PROPERTIES INSTALL_RPATH "@loader_path/../../${CMAKE_INSTALL_LIBDIR}")
  else()
    set_target_properties(runtexlua54 PROPERTIES INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")
  endif()
  target_link_libraries(runtexlua54
    ${app_dll_name}
    ${core_dll_name}
    ${program_libs54}
  )
  install(TARGETS runtexlua54 DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")
endif()

###############################################################################
## runmtxrun
###############################################################################

if(WITH_CONTEXT_SUPPORT)
  add_executable(runmtxrun ${luatex_sources} runmtxrun.cpp)
  set_property(TARGET runmtxrun PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(runmtxrun
    ${app_dll_name}
    ${core_dll_name}
    ${program_libs}
  )
  install(TARGETS runmtxrun DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")
endif()

###############################################################################
## run tests
###############################################################################

add_subdirectory(test)
