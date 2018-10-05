## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2015-2016 Christian Schenk
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

use_static_crt()

add_library(${apr_lib_name} STATIC ${apr_sources})

set_property(TARGET ${apr_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_source_files_properties(
  ${generated_headers}
  PROPERTIES GENERATED TRUE
)

add_dependencies(${apr_lib_name} gen-apr-headers)

target_compile_definitions(${apr_lib_name}
  PUBLIC
    -DAPR_DECLARE_STATIC
)

target_include_directories(${apr_lib_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${apr_lib_name}
  PUBLIC
    ${system_libraries}
)
