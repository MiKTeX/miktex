## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2001-2018 Christian Schenk
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

set(${expat_dll_name}_sources ${expat_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../expat.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/expat.rc
  )
  list(APPEND ${expat_dll_name}_sources
      expat.def
      ${CMAKE_CURRENT_BINARY_DIR}/expat.rc
    )
endif()

add_library(${expat_dll_name} SHARED ${${expat_dll_name}_sources})

set_property(TARGET ${expat_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${expat_dll_name}
  PROPERTIES
  VERSION "${MIKTEX_MAJOR_MINOR_STR}"
  SOVERSION "1"
)

target_include_directories(${expat_dll_name}
  PUBLIC
    ${public_include_directories}
)

install(TARGETS ${expat_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
