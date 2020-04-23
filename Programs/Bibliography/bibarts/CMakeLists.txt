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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_CONVERTERS_FOLDER}/bibarts")

configure_file(
  bibsort-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/bibsort-version.h
)

configure_file(
  bibsort.rc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/bibsort.rc
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

set(bibsort_sources
  source/bibsort.c
)

list(APPEND bibsort_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${CMAKE_CURRENT_BINARY_DIR}/bibsort-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND bibsort_sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/bibsort.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}bibsort ${bibsort_sources})

set_property(TARGET ${MIKTEX_PREFIX}bibsort PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}bibsort
  ${app_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}bibsort
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}bibsort DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
