## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017-2020 Christian Schenk
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

set(${gmp_dll_name}_sources ${gmp_sources})

set_source_files_properties(
  ${generated_gmp_sources}
  PROPERTIES GENERATED TRUE
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../gmp.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/gmp.rc
  )
  list(APPEND ${gmp_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/gmp.rc
  )
endif()

add_library(${gmp_dll_name} SHARED ${${gmp_dll_name}_sources})

add_dependencies(${gmp_dll_name} gen-gmp-sources)

set_property(TARGET ${gmp_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${gmp_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${gmp_dll_name} PUBLIC ${public_include_directories})

rebase(${gmp_dll_name})

install(TARGETS ${gmp_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
