## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_CONVERTERS_FOLDER}/axohelp")

configure_file(
  axohelp-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/axohelp-version.h
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -Dmain=Main
  -DUSE_MIKTEX_EXIT=1
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(axohelp_c_sources
  source/axohelp.c
)

set(axohelp_sources ${axohelp_c_sources})

list(APPEND axohelp_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${CMAKE_CURRENT_BINARY_DIR}/axohelp-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    axohelp.rc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/axohelp.rc
  )
  list(APPEND axohelp_sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/axohelp.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}axohelp ${axohelp_sources})

set_property(TARGET ${MIKTEX_PREFIX}axohelp PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}axohelp
  ${app_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}axohelp
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}axohelp DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
