## CMakeLists.txt					-*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

use_static_crt()

set(md5walk_sources
  md5walk-version.h
  md5walk.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND md5walk_sources md5walk.rc)
endif()

add_executable(md5walk ${md5walk_sources})

set_property(TARGET md5walk PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(md5walk
  ${core_lib_name}
  miktex-popt-wrapper-static
)

merge_trustinfo_manifest(md5walk asInvoker)
merge_compatibility_manifest(md5walk)

install(TARGETS md5walk DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
