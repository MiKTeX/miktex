## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2019 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/SyncTeX")

configure_file(
  synctex-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/synctex-version.h
)

configure_file(
  synctex.rc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/synctex.rc
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(libsynctex_sources
  source/synctex_parser.c
  source/synctex_parser_advanced.h
  source/synctex_parser_local.h
  source/synctex_parser_utils.c
)

set(synctex_sources
  source/synctex_main.c
)

list(APPEND synctex_sources
  ${CMAKE_CURRENT_BINARY_DIR}/synctex-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND synctex_sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/synctex.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}synctex ${synctex_sources} ${libsynctex_sources} ${MIKTEX_LIBRARY_WRAPPER})

set_property(TARGET ${MIKTEX_PREFIX}synctex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}synctex
  ${app_dll_name}
  ${w2cemu_dll_name}
)

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${MIKTEX_PREFIX}synctex MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${MIKTEX_PREFIX}synctex ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}synctex
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}synctex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
