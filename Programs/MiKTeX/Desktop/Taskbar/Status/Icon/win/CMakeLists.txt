## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2009-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_TASKBAR_FOLDER}")

use_static_crt()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -D_UNICODE
  -DUNICODE
)

configure_file(
  res/dialog.manifest.in
  ${CMAKE_CURRENT_BINARY_DIR}/res/dialog.manifest
)

configure_file(
  dialog.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/dialog.rc
)


add_executable(${MIKTEX_PREFIX}taskbar-icon WIN32
  ${CMAKE_CURRENT_BINARY_DIR}/dialog.rc
  dialog.cpp
  resource.h
  StdAfx.h
)

set_property(TARGET ${MIKTEX_PREFIX}taskbar-icon PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}taskbar-icon
  ${core_lib_name}
  comctl32.lib
)

merge_trustinfo_manifest(${MIKTEX_PREFIX}taskbar-icon asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}taskbar-icon)

install(
  TARGETS ${MIKTEX_PREFIX}taskbar-icon
  DESTINATION "${MIKTEX_INTERNAL_BINARY_DIR}"
)
