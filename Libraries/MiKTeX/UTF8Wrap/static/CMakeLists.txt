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

add_library(${utf8wrap_lib_name} STATIC ${lib_sources})

set_property(TARGET ${utf8wrap_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${utf8wrap_lib_name}
  PUBLIC
    -DMIKTEX_UTF8WRAP_STATIC
)

target_include_directories(${utf8wrap_lib_name}
  PUBLIC
    ${public_include_directories}
)

set(utf8wrap_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8wrap.cpp
)

add_executable(utf8wrap ${utf8wrap_sources})

set_property(TARGET utf8wrap PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(utf8wrap asInvoker)
merge_compatibility_manifest(utf8wrap)

target_link_libraries(utf8wrap
  ${utf8wrap_lib_name}
)

install(TARGETS utf8wrap DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

source_group(Public FILES ${public_headers})
