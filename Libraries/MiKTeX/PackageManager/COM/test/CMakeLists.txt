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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/packagemanager/comtest")

# TODO: 
include_directories(
  ${CMAKE_BINARY_DIR}/Libraries/MiKTeX/PackageManager/include
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  StdAfx.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h
)  

set(listpackages_sources
  ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h
  listpackages.cpp
)

add_executable(listpackages ${listpackages_sources})

set_property(TARGET listpackages PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(listpackages gen-mpm-com)
