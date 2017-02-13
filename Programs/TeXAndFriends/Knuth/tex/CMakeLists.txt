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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_KNUTH_FOLDER}/tex")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DHAVE_C4P_PRE_H
)

default_char_type_is_unsigned()

set(C4P_FLAGS
  --chars-are-unsigned
  --emit-optimize-pragmas
)

set(tex_changefiles
  ${CMAKE_CURRENT_SOURCE_DIR}/mltex-miktex.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/enctex-miktex.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-misc.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-enctex.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-hash.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-hyph.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-pool.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-quiet.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-src.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-stat.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/tex-miktex-write18.ch
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.ch
  COMMAND
    ${MIKTEX_PREFIX}tie
    -c ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
    ${tex_changefiles}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    source/tex.web
    ${tex_changefiles}
  VERBATIM
)

set(tex_change_file ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.ch)

set(tex_target_name tex)

list(APPEND ${tex_target_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/tex_pool.cpp
)

create_web_app(TeX)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tex_pool.cpp
  COMMAND
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/tex.pool
    tex-miktex.h
    ${tex_progclass}
    ${tex_prog}
    > ${CMAKE_CURRENT_BINARY_DIR}/tex_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/tex.pool
)

if(LINK_EVERYTHING_STATICALLY)
  target_link_libraries(${tex_target_name}
    ${w2cemu_lib_name}
  )
else()
  target_link_libraries(${tex_target_name}
    PRIVATE
      ${w2cemu_dll_name}
  )
endif()

if(WITH_TRIPTEX)
  include(triptex.cmake)
endif()

## dev targets

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tex-n.web
  COMMAND
    web-n
    < ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
    > ${CMAKE_CURRENT_BINARY_DIR}/tex-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    web-n
    source/tex.web
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.web
    ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex-n.web
  COMMAND
    ${MIKTEX_PREFIX}tie
    -m ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.web
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
    ${tex_change_file}
  COMMAND
    web-n
    < ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.web
    > ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    web-n
    source/tex.web
    ${tex_change_file}
  VERBATIM
)

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/tex_pool.cpp
  PROPERTIES COMPILE_FLAGS
    "-DMIKTEX_TEX -DTEXPROGCLASS=${tex_progclass} -DTEXPROG=${tex_prog} -DTEXAPPCLASS=${tex_appclass} -DTEXAPP=${tex_app}"
)

add_custom_target(tex-dev ALL
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/tex-n.web
    ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex-n.web
)

set_property(TARGET tex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

###
option(
  MIKTEX_TEX_OPTIMIZATION_CHECKS
  "Create TeX optimization checks."
  FALSE
)

if(MIKTEX_TEX_OPTIMIZATION_CHECKS)
  include(optchecks.cmake)
endif()
