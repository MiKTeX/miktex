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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/regex")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories ${CMAKE_CURRENT_BINARY_DIR}/include)

set(sources
  ${CMAKE_CURRENT_SOURCE_DIR}/regex-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/regex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/regex.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/regex_internal.h
)

set(${regex_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/regex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/regex.rc
  )
  set(${regex_dll_name}_sources ${${regex_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/regex.rc
  )
endif()

add_library(${regex_dll_name} SHARED ${${regex_dll_name}_sources})

set_property(TARGET ${regex_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${regex_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_compile_definitions(${regex_dll_name}
  PUBLIC
    -DMIKTEX_REGEX_SHARED
)

target_include_directories(${regex_dll_name}
  PUBLIC
    ${public_include_directories}
)

install(TARGETS ${regex_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

set(public_headers
  regex.h
)

foreach(h ${public_headers})
  configure_file(
    source/posix/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

