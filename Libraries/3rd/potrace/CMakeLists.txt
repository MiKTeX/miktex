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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/potrace")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

ignore_warnings()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  potrace-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/potrace-version.h
)

set(public_headers
  potracelib.h
)

foreach(h ${public_headers})
  configure_file(
    source/src/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

set(potrace_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/curve.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/curve.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/trace.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/trace.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/decompose.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/decompose.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/potracelib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/potracelib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/lists.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/auxiliary.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/bitmap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/progress.h

  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/potrace-version.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
