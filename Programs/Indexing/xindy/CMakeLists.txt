## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_INDEXING_FOLDER}/xindy")

add_definitions(
  -DCPLUSPLUSMAIN
  -DUSE_MIKTEX_EXIT
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

ignore_warnings()

set(tex2xindy_sources
  ${CMAKE_CURRENT_BINARY_DIR}/tex2xindy.cpp
  ${MIKTEX_LIBRARY_WRAPPER}
  tex2xindy-version.h
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tex2xindy.cpp
  COMMAND
    ${FLEX_EXECUTABLE}
    --noline
    -o${CMAKE_CURRENT_BINARY_DIR}/tex2xindy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tex2xindy/tex2xindy.l
  DEPENDS
    source/tex2xindy/tex2xindy.l
  VERBATIM
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND tex2xindy_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    tex2xindy.rc
  )
endif()

add_executable(tex2xindy ${tex2xindy_sources})

set_property(TARGET tex2xindy PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_MACOS_BUNDLE)
  set_target_properties(tex2xindy PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(APPLE)
  set_target_properties(tex2xindy PROPERTIES INSTALL_RPATH "@loader_path/../../${CMAKE_INSTALL_LIBDIR}")
else()
  set_target_properties(tex2xindy PROPERTIES INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")
endif()

target_link_libraries(tex2xindy
  ${app_dll_name}
  ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(tex2xindy
    ${getopt_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS tex2xindy DESTINATION ${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR})
