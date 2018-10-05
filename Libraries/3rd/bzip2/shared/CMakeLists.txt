## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2018 Christian Schenk
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

set(${bzip2_dll_name}_sources ${bzip2_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../libbz2.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libbz2.rc
  )
  list(APPEND ${bzip2_dll_name}_sources
    ../libbz2.def
    ${CMAKE_CURRENT_BINARY_DIR}/libbz2.rc
  )
endif()

add_library(${bzip2_dll_name} SHARED ${${bzip2_dll_name}_sources})

set_property(TARGET ${bzip2_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${bzip2_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_include_directories(${bzip2_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${bzip2_dll_name}
  PRIVATE
    ${utf8wrap_dll_name}
)

install(TARGETS ${bzip2_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
