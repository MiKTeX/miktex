## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2014-2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETUP_FOLDER}")

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(miktexsetup_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/miktexsetup-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/miktexsetup.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND miktexsetup_sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_SOURCE_DIR}/miktexsetup.rc
  )
endif()

add_subdirectory(integrated)
add_subdirectory(standalone)
