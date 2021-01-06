## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2021 Christian Schenk
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

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(${util_dll_name}_sources
  ${util_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../util.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/util.rc
  )
  list(APPEND ${util_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/util.rc
  )
  list(APPEND resource_files
    ${CMAKE_CURRENT_BINARY_DIR}/util.rc
  )
endif()

add_library(${util_dll_name} SHARED ${${util_dll_name}_sources})

set_property(TARGET ${util_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${util_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${util_dll_name}
  PUBLIC
    -DMIKTEX_UTIL_SHARED
)

target_include_directories(${util_dll_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_FMT)
  target_link_libraries(${util_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${util_dll_name} PRIVATE ${fmt_dll_name})
endif()

install(TARGETS ${util_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Resources FILES ${resource_files})
source_group(Public FILES ${public_headers})
