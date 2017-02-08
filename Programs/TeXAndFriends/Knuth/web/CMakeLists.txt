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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_KNUTH_FOLDER}/web")

if(LINK_EVERYTHING_STATICALLY)
  use_static_crt()
endif()
  
include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/.tangle_bootstrap
    ${CMAKE_CURRENT_BINARY_DIR}/tangle.p
    ${CMAKE_CURRENT_BINARY_DIR}/tangle.pool
  COMMAND
    ${CMAKE_COMMAND}
    -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/inittangle.p
    ${CMAKE_CURRENT_BINARY_DIR}/tangle.p
  COMMAND
    ${CMAKE_COMMAND}
    -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/inittangle.pool
    ${CMAKE_CURRENT_BINARY_DIR}/tangle.pool
  COMMAND
    ${CMAKE_COMMAND}
    -E touch
    ${CMAKE_CURRENT_BINARY_DIR}/.tangle_bootstrap
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    ${CMAKE_CURRENT_SOURCE_DIR}/inittangle.p
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/inittangle.pool
  VERBATIM
)

add_custom_target(
  tangle_bootstrap
  ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/.tangle_bootstrap
)

set_property(TARGET tangle_bootstrap PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

create_web_app(Tangle)
create_web_app(Weave)

if(LINK_EVERYTHING_STATICALLY)
  add_dependencies(${tangle_lib_name} tangle_bootstrap)
else()
  add_dependencies(${tangle_dll_name} tangle_bootstrap)
endif()
