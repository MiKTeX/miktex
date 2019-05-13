## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2019 Christian Schenk
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

set(${trace_dll_name}_sources
  ${trace_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../trace.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/trace.rc
  )
  list(APPEND ${trace_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/trace.rc
  )
  list(APPEND resource_files
    ${CMAKE_CURRENT_BINARY_DIR}/trace.rc
  )
endif()

add_library(${trace_dll_name} SHARED ${${trace_dll_name}_sources})

set_property(TARGET ${trace_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version(
  ${trace_dll_name}
  ${MIKTEX_COMP_INTERFACE_VERSION}
  ${MIKTEX_COMP_J2000_VERSION}
)

target_compile_definitions(${trace_dll_name}
  PUBLIC
    -DMIKTEX_TRACE_SHARED
)

target_include_directories(${trace_dll_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_FMT)
  target_link_libraries(${trace_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${trace_dll_name} PRIVATE ${fmt_dll_name})
endif()

target_link_libraries(${trace_dll_name}
  PRIVATE
    ${util_dll_name}
)

install(TARGETS ${trace_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Resources FILES ${resource_files})
source_group(Public FILES ${public_headers})
