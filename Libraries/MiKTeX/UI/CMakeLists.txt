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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/ui")

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(miktex-ui INTERFACE)

target_include_directories(miktex-ui
  INTERFACE
    ${public_include_directories}
)

target_link_libraries(miktex-ui
  INTERFACE
    ${ui_mfc_dll_name}
)

target_link_libraries(miktex-ui
  INTERFACE
    ${ui_qt_dll_name}
)

add_library(miktex-ui-static INTERFACE)

target_include_directories(miktex-ui-static
  INTERFACE
    ${public_include_directories}
)

target_link_libraries(miktex-ui-static
  INTERFACE
    ${ui_mfc_lib_name}
)

target_link_libraries(miktex-ui-static
  INTERFACE
    ${ui_qt_lib_name}
)
