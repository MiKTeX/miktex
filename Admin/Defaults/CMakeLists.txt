## CMakeLists.txt:                                      -*- CMake -*-
##
## Copyright (C) 2015-2019 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/Defaults")

set(all_defaults_cmake
  mf.defaults.in
  mfapp.defaults.in
  mpost.defaults.in
  pdftex.defaults.in
  texapp.defaults.in
  texmfapp.defaults.in
)

if(WITH_OMEGA)
  list(APPEND all_defaults_cmake omega.defaults.in)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/miktex)

foreach(d ${all_defaults_cmake})
  get_filename_component(fn ${d} NAME_WE)
  configure_file(
    ${d}
    ${CMAKE_CURRENT_BINARY_DIR}/${fn}.defaults
    NEWLINE_STYLE UNIX
  )
  add_custom_command(
    OUTPUT
      ${CMAKE_BINARY_DIR}/include/miktex/${fn}.defaults.h
    COMMAND
      cfg
      --print-classes
      ${CMAKE_CURRENT_BINARY_DIR}/${fn}.defaults
      > ${fn}.defaults.h
    WORKING_DIRECTORY
      ${CMAKE_BINARY_DIR}/include/miktex
    MAIN_DEPENDENCY
      ${CMAKE_CURRENT_BINARY_DIR}/${fn}.defaults
    DEPENDS
      cfg
    VERBATIM
  )
  list(APPEND generated_sources ${CMAKE_BINARY_DIR}/include/miktex/${fn}.defaults.h)
endforeach()

add_custom_target(gen-defaults
  DEPENDS
    ${generated_sources}
)

set_property(TARGET gen-defaults PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
