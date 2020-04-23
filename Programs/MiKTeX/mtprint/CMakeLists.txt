## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2018 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/${MIKTEX_PROG_NAME_MTPRINT}")

if(MSVC)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

set(mtprint_sources
  BitmapPrinter.cpp
  BitmapPrinter.h
  Printer.cpp
  Printer.h
  StdAfx.h
  internal.h
  mtprint-version.h
  mtprint.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mtprint_sources
    ${MIKTEX_COMMON_MANIFEST}
    mtprint.rc
  )
endif()

add_executable(${MIKTEX_PROG_NAME_MTPRINT} ${mtprint_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_MTPRINT} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PROG_NAME_MTPRINT}
  ${app_dll_name}
  ${core_dll_name}
  ${core_dll_name}
  ${dib_dll_name}
  ${texmf_dll_name}
  miktex-popt-wrapper
)

install(TARGETS ${MIKTEX_PROG_NAME_MTPRINT} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
