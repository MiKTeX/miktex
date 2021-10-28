## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2021 Christian Schenk
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
  ${MIKTEX_TEX_MLTEX_CH}
  ${MIKTEX_TEX_CONSTANTS_CH}
  ${MIKTEX_TEX_CH}
  ${MIKTEX_TEX_HASH_CH}
  ${MIKTEX_TEX_HYPH_CH}
  ${MIKTEX_TEX_POOL_CH}
  ${MIKTEX_TEX_QUIET_CH}
  ${MIKTEX_TEX_SRC_CH}
  ${MIKTEX_TEX_STAT_CH}
  ${MIKTEX_TEX_WRITE18_CH}
  ${ENCTEX1_CH}
  ${ENCTEX_TEX_CH}
  ${ENCTEX2_CH}
  ${MIKTEX_TEX_ENCTEX_CH}
  ${MIKTEX_TEX_WEB2C_CH}
  ${MIKTEX_TEX_FINISH_CH}
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex-final.ch
  COMMAND
    ${MIKTEX_PREFIX}tie
      -c ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex-final.ch
      ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
      ${tex_changefiles}
  MAIN_DEPENDENCY
    source/tex.web
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${tex_changefiles}
  VERBATIM
)

set(tex_change_file ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex-final.ch)

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
      miktex-tex.h
      ${tex_progclass}
      ${tex_prog}
    > ${CMAKE_CURRENT_BINARY_DIR}/tex_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_BINARY_DIR}/tex.pool
  DEPENDS
    inipool
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
  MAIN_DEPENDENCY
    source/tex.web
  DEPENDS
    web-n
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex-n.web
  COMMAND
    ${MIKTEX_PREFIX}tie
      -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web
      ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
      ${tex_change_file}
  COMMAND
    web-n
      < ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web
      > ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/tex.web
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${tex_change_file}
    web-n
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.tex
  COMMAND
    ${MIKTEX_PREFIX}weave
      ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
      ${tex_change_file}
      ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.tex
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/tex.web
  DEPENDS
    ${MIKTEX_PREFIX}weave
    ${tex_change_file}
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tex.tex
  COMMAND
    ${MIKTEX_PREFIX}weave
      ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
      ${MIKTEX_DEV_NULL}
      ${CMAKE_CURRENT_BINARY_DIR}/tex.tex
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/tex.web
  DEPENDS
    ${MIKTEX_PREFIX}weave
  VERBATIM
)

add_custom_target(tex-dev ALL
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex-n.web
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.tex
    ${CMAKE_CURRENT_BINARY_DIR}/tex-n.web
    ${CMAKE_CURRENT_BINARY_DIR}/tex.tex
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
