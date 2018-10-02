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

add_library(${core_ps_dll_name} SHARED ${core_ps_sources})

set_property(TARGET ${core_ps_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${core_ps_dll_name} gen-core-com)

set_source_files_properties(
  ${generated_core_ps_sources}
  PROPERTIES
    GENERATED TRUE
)

target_compile_definitions(${core_ps_dll_name}
  PRIVATE
    -DREGISTER_PROXY_DLL
)

target_include_directories(${core_ps_dll_name}
  PRIVATE
    ${public_include_directories}
)

target_link_libraries(${core_ps_dll_name}
  PRIVATE
    oleaut32
    rpcns4
    rpcrt4
    uuid
)

install(TARGETS ${core_ps_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}")
