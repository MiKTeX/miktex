## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

use_static_crt()

add_definitions(
  -Dalloca=_alloca
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(c4p_sources
  ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/gram.h
  ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp
  c4p-version.h
  common.h
  main.cpp
  misc.cpp
  output.cpp
  output.h
  subrange.cpp
  symtab.cpp
  type.cpp
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/gram.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/gram.h
  COMMAND
    ${BISON_EXECUTABLE}
    --output=${CMAKE_CURRENT_BINARY_DIR}/gram.cpp
    --defines=${CMAKE_CURRENT_BINARY_DIR}/gram.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gram.y
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/gram.y
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/scan.cpp
  COMMAND
    ${FLEX_EXECUTABLE}
    -o${CMAKE_CURRENT_BINARY_DIR}/scan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scan.l
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/scan.l
  VERBATIM
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND c4p_sources
    ${MIKTEX_COMMON_MANIFEST}
    c4p.rc
  )
endif()

add_executable(c4p ${c4p_sources})

set_property(TARGET c4p PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(NOT HAVE_GETOPT_LONG)
  target_link_libraries(c4p ${getopt_lib_name})
endif()
