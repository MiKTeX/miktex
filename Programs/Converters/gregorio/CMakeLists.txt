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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_CONVERTERS_FOLDER}/gregorio")

# FIXME: add checks
set(ALIGNOF_UINT32_T 4)
set(FUNC_REALPATH_WORKS TRUE)

set(USE_KPSE TRUE)

set(PACKAGE_URL "http://gregorio-project.github.io")

configure_file(
  config_.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config_.h
)

configure_file(
  gregorio-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/gregorio-version.h
)

configure_file(
  gregorio.rc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/gregorio.rc
  )

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  source/src
  source/src/gabc
  source/src/dump
  source/src/gregoriotex
)

add_definitions(
  -Dmain=Main
  -DUSE_MIKTEX_EXIT=1
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(gregorio_c_sources
  source/src/characters.c
  source/src/gregorio-utils.c
  source/src/messages.c
  source/src/sha1.c
  source/src/struct.c
  source/src/support.c
  source/src/unicode.c
  source/src/dump/dump.c
  source/src/gabc/gabc-elements-determination.c
  source/src/gabc/gabc-glyphs-determination.c
  source/src/gabc/gabc-notes-determination-l.c
  source/src/gabc/gabc-score-determination.c
  source/src/gabc/gabc-score-determination-l.c
  source/src/gabc/gabc-score-determination-y.c
  source/src/gabc/gabc-write.c
  source/src/gregoriotex/gregoriotex-position.c
  source/src/gregoriotex/gregoriotex-write.c
  source/src/vowel/vowel.c
  source/src/vowel/vowel-rules-l.c
  source/src/vowel/vowel-rules-y.c

  source/src/bool.h
  source/src/characters.h
  source/src/config.h
  source/src/enum_generator.h
  source/src/gabc/gabc-score-determination-l.h
  source/src/gabc/gabc-score-determination-y.h
  source/src/gabc/gabc-score-determination.h
  source/src/gabc/gabc.h
  source/src/gregoriotex/gregoriotex.h
  source/src/messages.h
  source/src/plugins.h
  source/src/sha1.h
  source/src/struct.h
  source/src/struct_iter.h
  source/src/support.h
  source/src/unicode.h
  source/src/utf8strings.h
  source/src/vowel/vowel-rules-l.h
  source/src/vowel/vowel-rules-y.h
  source/src/vowel/vowel-rules.h
  source/src/vowel/vowel.h
)

set(gregorio_sources ${gregorio_c_sources})

list(APPEND gregorio_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${CMAKE_CURRENT_BINARY_DIR}/config_.h
  ${CMAKE_CURRENT_BINARY_DIR}/gregorio-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND gregorio_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/gregorio.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}gregorio ${gregorio_sources})

set_property(TARGET ${MIKTEX_PREFIX}gregorio PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}gregorio
  ${app_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}gregorio
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}gregorio DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
