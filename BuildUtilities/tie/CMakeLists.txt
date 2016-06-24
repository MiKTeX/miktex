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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}/Tie")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(tie_sources ${CMAKE_CURRENT_BINARY_DIR}/tie.c)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tie.c
  COMMAND
    ctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tie.w
    ${CMAKE_CURRENT_SOURCE_DIR}/tie-miktex.ch 
    ${CMAKE_CURRENT_BINARY_DIR}/tie.c
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  MAIN_DEPENDENCY
    source/tie.w
  DEPENDS
    ctangle
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tie.w
    ${CMAKE_CURRENT_SOURCE_DIR}/tie-miktex.ch
  VERBATIM
)

add_executable(${MIKTEX_PREFIX}tie ${tie_sources})

set_property(TARGET ${MIKTEX_PREFIX}tie PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}tie asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}tie)

install(TARGETS ${MIKTEX_PREFIX}tie DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
