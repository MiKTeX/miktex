## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2011-2016 Christian Schenk
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

add_library(${unxemu_lib_name} STATIC ${lib_sources})

set_property(TARGET ${unxemu_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${unxemu_lib_name}
  INTERFACE
    ${interface_definitions}
  PUBLIC
    -DMIKTEX_UNXEMU_STATIC
)

target_include_directories(${unxemu_lib_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${unxemu_lib_name}
  PUBLIC
    ${core_lib_name}
)

source_group(Public FILES ${public_headers})
