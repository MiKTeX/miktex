## triptex.cmake                                        -*- CMake -*-
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

set(triptex_dll_name "triptex_")

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/triptex-miktex.ch
  COMMAND
    ${MIKTEX_PREFIX}tie
    -c ${CMAKE_CURRENT_BINARY_DIR}/triptex-miktex.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
    ${tex_changefiles}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    source/tex.web
    ${tex_changefiles}
  VERBATIM
)

set(triptex_web_file ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web)
set(triptex_change_file ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.ch)

if(LINK_EVERYTHING_STATICALLY)
  set(triptex_target_name ${triptex_lib_name})
else()
  set(triptex_target_name ${triptex_dll_name})
endif()

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
  COMMAND
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/triptex.pool
    miktex-tex.h
    > ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/triptex.pool
)

set(${triptex_target_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
)

create_web_app(TripTeX)

if(LINK_EVERYTHING_STATICALLY)
  target_link_libraries(${triptex_target_name}
    ${w2cemu_lib_name}
  )
else()
  target_link_libraries(${triptex_target_name}
    PRIVATE
      ${w2cemu_dll_name}
  )
endif()

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
  PROPERTIES COMPILE_FLAGS
    "-DMIKTEX_TRIPTEX"
)
