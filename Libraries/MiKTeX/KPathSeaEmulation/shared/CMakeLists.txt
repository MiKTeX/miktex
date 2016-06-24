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

hide_symbols()

set(${kpsemu_dll_name}_sources ${kpsemu_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../kpsemu.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/kpsemu.rc
  )
  list(APPEND ${kpsemu_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/kpsemu.rc
  )
endif()

add_library(${kpsemu_dll_name} SHARED ${${kpsemu_dll_name}_sources})

set_property(TARGET ${kpsemu_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${kpsemu_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_compile_definitions(${kpsemu_dll_name}
  PUBLIC
    -DMIKTEX_KPSEMU_SHARED
)

target_include_directories(${kpsemu_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${kpsemu_dll_name}
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${kpsemu_dll_name}
    PUBLIC
      ${getopt_dll_name}
    INTERFACE
      ${unxemu_dll_name}
  )
endif()

rebase(${kpsemu_dll_name})

install(TARGETS ${kpsemu_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
