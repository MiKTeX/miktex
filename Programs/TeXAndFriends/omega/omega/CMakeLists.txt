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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_OMEGA_FOLDER}/omega")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

ignore_warnings()

set(miktex_changefiles
  ${CMAKE_CURRENT_SOURCE_DIR}/omega-miktex-fmt.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/omega-miktex-misc.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/omega-miktex-pool.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/omega-miktex.ch
)

set(omfiles
  ${CMAKE_CURRENT_SOURCE_DIR}/source/om16bit.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omstr.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omfont.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omchar.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omfi.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/ompar.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omocp.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omfilter.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omtrans.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omdir.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/source/omxml.ch
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/omega.web
  COMMAND
    ${MIKTEX_PREFIX}tie
    -m ${CMAKE_CURRENT_BINARY_DIR}/omega.web
    ${CMAKE_CURRENT_SOURCE_DIR}/tex98/tex.web
    ${omfiles}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    tex98/tex.web
    ${omfiles}
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/omega-miktex-all.ch
  COMMAND
    ${MIKTEX_PREFIX}tie
    -c ${CMAKE_CURRENT_BINARY_DIR}/omega-miktex-all.ch
    ${CMAKE_CURRENT_BINARY_DIR}/omega.web
    ${miktex_changefiles}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${CMAKE_CURRENT_BINARY_DIR}/omega.web
    ${miktex_changefiles}
  VERBATIM
)

set(omega_change_file ${CMAKE_CURRENT_BINARY_DIR}/omega-miktex-all.ch)

set(${omega_dll_name}_sources
  ${CMAKE_BINARY_DIR}/include/miktex/omega.defaults.h
  source/omegabis.c
  omega-miktex.cc
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/omega_pool.cpp
  COMMAND
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/omega.pool
    omega-miktex.h
    > ${CMAKE_CURRENT_BINARY_DIR}/omega_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/omega.pool
)

list(APPEND ${omega_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/omega_pool.cpp
)

set_source_files_properties(
  source/omegabis.c
  PROPERTIES LANGUAGE CXX
)

set_source_files_properties(
  ${${omega_dll_name}_sources}
  PROPERTIES COMPILE_FLAGS
    "-DC4PEXTERN=extern -DMIKTEX_OMEGA"

)

file(READ ${MIKTEX_DYN_TEX_SCRIPT} tex_dyn_sed_contents)
file(READ ${MIKTEX_DYN_OMEGA_SCRIPT} omega_dyn_sed_contents)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${tex_dyn_sed_contents}")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${omega_dyn_sed_contents}")
endif()

set(TANGLE_FLAGS -omega)

create_web_app(Omega)

add_dependencies(${omega_dll_name} gen-defaults)

set_source_files_properties(
  ${CMAKE_BINARY_DIR}/include/miktex/omega.defaults.h
  PROPERTIES
    GENERATED TRUE
)

target_link_libraries(${omega_dll_name}
  PRIVATE
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${omega_dll_name}
    PRIVATE
      ${getopt_dll_name}
      ${unxemu_dll_name}
  )
endif()

## dev targets

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/omega-n.web
  COMMAND
    web-n
    < ${omega_web_file}
    > ${CMAKE_CURRENT_BINARY_DIR}/omega-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    web-n
    ${omega_web_file}
  VERBATIM
)

add_custom_target(omega-dev ALL
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/omega-n.web
)

set_property(TARGET omega-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
