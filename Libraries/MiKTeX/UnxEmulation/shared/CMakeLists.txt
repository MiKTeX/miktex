## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2011-2019 Christian Schenk
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

set(${unxemu_dll_name}_sources ${lib_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../unxemu.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/unxemu.rc
  )
  list(APPEND ${unxemu_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/unxemu.rc
  )
endif()

add_library(${unxemu_dll_name} SHARED ${${unxemu_dll_name}_sources})

set_property(TARGET ${unxemu_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version(
  ${unxemu_dll_name}
  ${MIKTEX_COMP_INTERFACE_VERSION}
  ${MIKTEX_COMP_J2000_VERSION}
)

target_compile_definitions(${unxemu_dll_name}
  INTERFACE
    ${interface_definitions}
  PUBLIC
    -DMIKTEX_UNXEMU_SHARED
)

target_include_directories(${unxemu_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${unxemu_dll_name}
  PUBLIC
    ${core_dll_name}
)

install(TARGETS ${unxemu_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
