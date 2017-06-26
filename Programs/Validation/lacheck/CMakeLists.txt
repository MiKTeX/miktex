## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_VALIDATION_FOLDER}/lacheck")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DUSE_MIKTEX_EXIT
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(lacheck_c_sources
  ${CMAKE_CURRENT_BINARY_DIR}/lacheck.c
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/lacheck.c
  COMMAND
    ${FLEX_EXECUTABLE}
    --noline
    -o${CMAKE_CURRENT_BINARY_DIR}/lacheck.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lacheck.l
  DEPENDS
    source/lacheck.l
  VERBATIM
)

set(lacheck_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${lacheck_c_sources}
  lacheck-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND lacheck_sources
    lacheck.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}lacheck ${lacheck_sources})

set_property(TARGET ${MIKTEX_PREFIX}lacheck PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}lacheck asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}lacheck)

target_link_libraries(${MIKTEX_PREFIX}lacheck
  ${app_dll_name}
  ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}lacheck
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}lacheck DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
