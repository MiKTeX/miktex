## trapmf.cmake                                         -*- CMake -*-
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

set(trapmf_dll_name "trapmf_")

set(trapmf_web_file ${CMAKE_CURRENT_SOURCE_DIR}/source/mf.web)
set(trapmf_change_file ${CMAKE_CURRENT_BINARY_DIR}/trapmf-miktex.ch)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/trapmf-miktex.ch
  COMMAND
    ${MIKTEX_PREFIX}tie
    -c ${CMAKE_CURRENT_BINARY_DIR}/trapmf-miktex.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mf.web
    ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex-pool.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex-trap.ch
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mf.web
    ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex-pool.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex-trap.ch
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/trapmf-screen.cpp
  COMMAND
    ${CMAKE_COMMAND}
    -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/screen.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/trapmf-screen.cpp
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/screen.cpp
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/trapmf_pool.cpp
  COMMAND
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/trapmf.pool
    mf-miktex.h
    > ${CMAKE_CURRENT_BINARY_DIR}/trapmf_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/trapmf.pool
)

set(${trapmf_dll_name}_sources
  ${CMAKE_BINARY_DIR}/include/miktex/mf.defaults.h
  ${CMAKE_CURRENT_BINARY_DIR}/trapmf_pool.cpp
  screen.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ${trapmf_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/trapmf-screen.cpp
  )
endif()

create_web_app(METAFONT trapmf)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/trapmf-screen.cpp
  PROPERTIES COMPILE_FLAGS
    "-DMIKTEX_METAFONT -DMIKTEX_TRAPMF -DTRAPMFCLASS=${trapmf_class} -DTRAPMFAPP=${trapmf_app} -DTRAPMFDATA=${trapmf_data}"
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/trapmf_pool.cpp
  PROPERTIES COMPILE_FLAGS
    "-DMIKTEX_TRAPMF"
)
