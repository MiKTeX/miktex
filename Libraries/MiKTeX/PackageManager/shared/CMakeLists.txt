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

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(HAVE_ATLBASE_H)
  include(com.cmake)
  add_subdirectory(ps)
endif()

set(${mpm_dll_name}_sources
  ${mpm_com_sources}
  ${mpm_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../mpm.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mpm.rc
  )
  list(APPEND ${mpm_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mpm.rc
  )
endif()

add_library(${mpm_dll_name} SHARED ${${mpm_dll_name}_sources})

set_property(TARGET ${mpm_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(WITH_COM)
  add_dependencies(${mpm_dll_name} gen-core-com)
endif()

set_target_properties(${mpm_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_compile_definitions(${mpm_dll_name}
  PUBLIC
    -DMIKTEX_MPM_SHARED
)

target_include_directories(${mpm_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${mpm_dll_name}
  PRIVATE
    ${core_dll_name}
    ${expat_dll_name}
    ${extractor_dll_name}
    ${md5_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${mpm_dll_name}
    PRIVATE
      ${curl_dll_name}
      Ws2_32
      wininet
  )
else()
  target_link_libraries(${mpm_dll_name}
    PRIVATE
      ${CURL_LIBRARIES}
  )
endif()

delay_load(${mpm_dll_name}
  ${curl_dll_name}
  ${extractor_dll_name}
  Ws2_32
  wininet
)

rebase(${mpm_dll_name})

install(TARGETS ${mpm_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
