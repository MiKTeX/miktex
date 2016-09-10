## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/nlohmann-json")

set(interface_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

configure_file(
  source/src/json.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/include/nlohmann/json.hpp
  COPYONLY
)

add_library(${nlohmann_json_dll_name} INTERFACE)

target_include_directories(${nlohmann_json_dll_name}
  INTERFACE
    ${interface_include_directories}
)

add_library(${nlohmann_json_lib_name} INTERFACE)

target_include_directories(${nlohmann_json_lib_name}
  INTERFACE
    ${interface_include_directories}
)
