## CMakeLists.txt
##
## Copyright (C) 2021 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/config")

set(interface_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

configure_file(
  include/miktex/Configuration/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Configuration/config.h
)

configure_file(
  include/miktex/Configuration/ConfigNames.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Configuration/ConfigNames.h
)

set(headers_no_ext
  miktex/Configuration/ConfigNames
  miktex/Configuration/ConfigValue
  miktex/Configuration/ConfigurationProvider
  miktex/Configuration/HasNamedValues
  miktex/Configuration/TriState
)

foreach(h ${headers_no_ext})
  set(_path ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
  list(APPEND public_headers_no_ext ${_path})
  if (NOT EXISTS ${_path})
    file(WRITE ${_path}
      "#pragma once\n#include <${h}.h>\n"
    )
  endif()
endforeach()

set(public_headers
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Configuration/ConfigNames.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Configuration/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Configuration/ConfigValue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Configuration/ConfigurationProvider.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Configuration/HasNamedValues.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Configuration/TriState.h  
  ${public_headers_no_ext}
)

add_library(${config_dll_name} INTERFACE)

target_include_directories(${config_dll_name}
  INTERFACE
    ${interface_include_directories}
)

target_link_libraries(${config_dll_name}
  INTERFACE
    ${util_dll_name}
)

add_library(${config_lib_name} INTERFACE)

target_include_directories(${config_lib_name}
  INTERFACE
    ${interface_include_directories}
)

target_link_libraries(${config_lib_name}
  INTERFACE
    ${util_lib_name}
)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      ${public_headers}
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/Configuration
  )
endif()

source_group(Public FILES ${public_headers})
