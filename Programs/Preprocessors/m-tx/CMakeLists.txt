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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_PREPROCESSORS_FOLDER}/m-tx")

add_definitions(
  -DNO_DECLARE_MALLOC
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/libp2c
)

set(prepmx_sources
  source/analyze.c
  source/analyze.h
  source/cfuncs.c
  source/cfuncs.h
  source/control.c
  source/control.h
  source/files.c
  source/files.h
  source/globals.c
  source/globals.h
  source/libp2c/p2c.h
  source/lyrics.c
  source/lyrics.h
  source/mtx.c
  source/mtx.h
  source/mtxline.c
  source/mtxline.h
  source/multfile.c
  source/multfile.h
  source/notes.c
  source/notes.h
  source/preamble.c
  source/preamble.h
  source/prepmx.c
  source/status.c
  source/status.h
  source/strings.h
  source/uptext.c
  source/uptext.h
  source/utility.c
  source/utility.h
)

list(APPEND prepmx_sources
  prepmx-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND prepmx_sources
    ${MIKTEX_COMMON_MANIFEST}
    prepmx.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}prepmx ${prepmx_sources})

set_property(TARGET ${MIKTEX_PREFIX}prepmx PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS ${MIKTEX_PREFIX}prepmx DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
