## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2020 Christian Schenk
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

if(MIKTEX_LINUX_DIST_CODE_NAME STREQUAL "xenial")
  set(lua53_target_name ${lua53_lib_name})
else()
  set(lua53_target_name ${lua53_dll_name})
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}
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

include(libluatex.cmake)
include(libunilib.cmake)
include(luaffi.cmake)
include(luafontforge.cmake)
include(luaharfbuzz.cmake)
include(luamisc.cmake)
include(luasocket.cmake)
include(miktex.cmake)

set(common_engine_sources
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MP_DIR}/source/lmplib.c
  luatex-version.h
  source/luatex.c
  source/luatex.h
  source/luatex_svnversion.h
)

set(common_engine_libs
  luatex-miktex-objects
  luatex-unilib-objects
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND common_engine_libs
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
    ws2_32.lib
    wsock32.lib
  )
endif()

list(APPEND common_engine_libs
  ${lua53_target_name}
  ${pplib_lib_name}
  luatex-common-engine-objects
  luatex-luafontforge-objects
  luatex-luaffi-objects
  luatex-luamisc-objects
  luatex-luasocket-objects
)

###############################################################################
## luatex
###############################################################################

set(luatex_sources ${common_engine_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/luatex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/luatex.rc
    )
  list(APPEND luatex_sources
    ${CMAKE_CURRENT_BINARY_DIR}/luatex.rc
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

set(luatex_libs
  ${common_engine_libs}
  luatex-engine-objects
)

add_executable(${MIKTEX_PREFIX}luatex ${luatex_sources} ${MIKTEX_LIBRARY_WRAPPER})

set_property(TARGET ${MIKTEX_PREFIX}luatex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}luatex
  ${luatex_libs}
)

install(TARGETS ${MIKTEX_PREFIX}luatex DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")

###############################################################################
## luahbtex
###############################################################################

set(luahbtex_sources ${common_engine_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/luahbtex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/luahbtex.rc
    )
  list(APPEND luahbtex_sources
    ${CMAKE_CURRENT_BINARY_DIR}/luahbtex.rc
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

set(luahbtex_libs
  ${common_engine_libs}
  luahbtex-engine-objects
  luahbtex-luaharfbuzz-objects
)

add_executable(${MIKTEX_PREFIX}luahbtex ${luahbtex_sources} ${MIKTEX_LIBRARY_WRAPPER})

set_property(TARGET ${MIKTEX_PREFIX}luahbtex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${MIKTEX_PREFIX}luahbtex
  PRIVATE
    -DLUATEX_HARFBUZZ_ENABLED
)

target_link_libraries(${MIKTEX_PREFIX}luahbtex
  ${luahbtex_libs}
)

install(TARGETS ${MIKTEX_PREFIX}luahbtex DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")

###############################################################################
## runtexlua
###############################################################################

add_executable(runtexlua ${luatex_sources} runtexlua.cpp)

set_property(TARGET runtexlua PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_MACOS_BUNDLE)
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(APPLE)
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "@loader_path/../../${CMAKE_INSTALL_LIBDIR}")
else()
  set_target_properties(runtexlua PROPERTIES INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")
endif()

target_link_libraries(runtexlua
  ${luatex_libs}
)

install(TARGETS runtexlua DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")

###############################################################################
## runmtxrun
###############################################################################

if(WITH_CONTEXT_SUPPORT)
  add_executable(runmtxrun ${common_engine_sources} runmtxrun.cpp)
  set_property(TARGET runmtxrun PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(runmtxrun
    ${luatex_libs}
  )
  install(TARGETS runmtxrun DESTINATION "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}")
endif()

###############################################################################
## run tests
###############################################################################

add_subdirectory(test)
