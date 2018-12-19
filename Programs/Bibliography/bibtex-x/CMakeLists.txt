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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BIBLIOGRAPHY_FOLDER}/bibtex-x")

set(BIBTEX8_COMPILE_AS_CXX 1)

default_char_type_is_unsigned()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
  -DHAVE_CONFIG_H
  -DKPATHSEA
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

if(BIBTEX8_COMPILE_AS_CXX)
  set_source_files_properties(
    source/bibtex-1.c
    source/bibtex-2.c
    source/bibtex-3.c
    source/bibtex-4.c
    source/bibtex.c
    source/utils.c
    PROPERTIES LANGUAGE CXX
  )
  add_definitions(
    -DCPLUSPLUSMAIN
  )
endif()

set(common_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  bibtex-x-version.h
  source/bibtex-1.c
  source/bibtex-2.c
  source/bibtex-3.c
  source/bibtex-4.c
  source/bibtex.c
  source/bibtex.h
  source/datatype.h
  source/gblprocs.h
  source/gblvars.h
  source/sysdep.h
  source/utils.c
  source/utils.h
  source/version.h
)

set(bibtex8_sources ${common_sources})

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND bibtex8_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    bibtex8.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}bibtex8 ${bibtex8_sources})

set_property(TARGET ${MIKTEX_PREFIX}bibtex8 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${MIKTEX_PREFIX}bibtex8 PROPERTIES COMPILE_DEFINITIONS "SUPPORT_8BIT")

target_link_libraries(${MIKTEX_PREFIX}bibtex8
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}bibtex8
    ${getopt_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}bibtex8 DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(bibtexu_sources ${common_sources})

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND bibtexu_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    bibtexu.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}bibtexu ${bibtexu_sources})

set_property(TARGET ${MIKTEX_PREFIX}bibtexu PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${MIKTEX_PREFIX}bibtexu PROPERTIES COMPILE_DEFINITIONS "UTF_8")

target_include_directories(${MIKTEX_PREFIX}bibtexu
  PRIVATE
    ${ICU_INCLUDE_DIRS}
)

target_link_libraries(${MIKTEX_PREFIX}bibtexu
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${ICU_LIBRARIES}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}bibtexu
    ${getopt_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}bibtexu DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
