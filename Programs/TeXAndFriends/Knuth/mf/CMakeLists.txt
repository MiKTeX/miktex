## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_KNUTH_FOLDER}/mf")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

default_char_type_is_unsigned()

list(APPEND C4P_FLAGS
  --chars-are-unsigned
)

set(mf_target_name mf)

set(${mf_target_name}_sources
  ${CMAKE_BINARY_DIR}/include/miktex/mf.defaults.h
  screen.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ${mf_target_name}_sources
    screen.cpp
  )
endif()

set(mf_changefiles
  ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex.ch
  ${CMAKE_CURRENT_SOURCE_DIR}/mf-miktex-pool.ch
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex_2.ch
  COMMAND
    ${MIKTEX_PREFIX}tie
    -c ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex_2.ch
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mf.web
    ${mf_changefiles}   
  DEPENDS
    ${MIKTEX_PREFIX}tie
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mf.web
    ${mf_changefiles}
  VERBATIM
)

set(mf_change_file ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex_2.ch)

list(APPEND ${mf_target_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mf_pool.cpp
)

list(APPEND C4P_FLAGS
  --rename conditional
)

create_web_app(METAFONT ${mf_target_name})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mf_pool.cpp
  COMMAND
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/mf.pool
    mf-miktex.h
    ${mf_progclass}
    ${mf_prog}
    > ${CMAKE_CURRENT_BINARY_DIR}/mf_pool.cpp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    inipool
    ${CMAKE_CURRENT_BINARY_DIR}/mf.pool
)

add_dependencies(${mf_target_name} gen-defaults)

set_source_files_properties(
  ${CMAKE_BINARY_DIR}/include/miktex/mf.defaults.h
  PROPERTIES
    GENERATED TRUE
)

if(USE_SYSTEM_FMT)
  target_link_libraries(${mf_target_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${mf_target_name} PRIVATE ${fmt_dll_name})
endif()

if(WITH_TRAPMF)
  include(trapmf.cmake)
endif()

## dev targets

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mf-n.web
  COMMAND
    web-n < ${mf_web_file} > ${CMAKE_CURRENT_BINARY_DIR}/mf-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    web-n
    ${mf_web_file}
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex.web
    ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex-n.web
  COMMAND
    ${MIKTEX_PREFIX}tie
    -m ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex.web
    ${mf_web_file}
    ${mf_change_file}
  COMMAND
    web-n
    < ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex.web
    > ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex-n.web
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    ${MIKTEX_PREFIX}tie
    web-n
    ${mf_web_file}
    ${mf_change_file}
  VERBATIM
)

add_custom_target(mf-dev ALL
  DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/mf-n.web
  ${CMAKE_CURRENT_BINARY_DIR}/mf-miktex-n.web
)

set_property(TARGET mf-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
