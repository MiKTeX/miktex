## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2009-2016 Christian Schenk
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

use_static_crt()

add_library(${kpsemu_lib_name} STATIC ${kpsemu_sources})

set_property(TARGET ${kpsemu_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${kpsemu_lib_name}
  PUBLIC
    -DMIKTEX_KPSEMU_STATIC
)

target_include_directories(${kpsemu_lib_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${kpsemu_lib_name}
  PUBLIC
    ${app_lib_name}
    ${core_lib_name}
    ${texmf_lib_name}
  INTERFACE
    ${getopt_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${kpsemu_lib_name}
    PUBLIC
      ${getopt_lib_name}
    INTERFACE
      ${unxemu_lib_name}
  )
endif()

if(INSTALL_STATIC_LIBRARIES)
  install(TARGETS ${kpsemu_lib_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endif()

source_group(Public FILES ${public_headers})
