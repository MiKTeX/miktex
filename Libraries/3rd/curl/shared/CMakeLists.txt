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

set(${curl_dll_name}_sources ${curl_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../curl.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/curl.rc
  )
  list(APPEND ${curl_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/curl.rc
  )
endif()

add_library(${curl_dll_name} SHARED ${${curl_dll_name}_sources})

set_property(TARGET ${curl_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${curl_dll_name}
  PUBLIC
    ${public_include_directories}
)

set_target_properties(${curl_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${curl_dll_name}
    PRIVATE
      winmm
      wsock32
      ws2_32
      Wldap32
  )
endif()

rebase(${curl_dll_name})

install(TARGETS ${curl_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
