## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/popt")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
  -DHAVE_CONFIG_H
)

ignore_warnings()

set(public_headers
  popt.h
)

foreach(h ${public_headers})
  configure_file(
    source/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

set(popt_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/popt-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/popt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/popt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poptconfig.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/popthelp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poptint.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poptint.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poptparse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/system.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
