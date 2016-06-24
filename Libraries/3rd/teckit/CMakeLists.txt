## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2007-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/teckit")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/source/Public-headers
)

ignore_warnings()

set(sources
  source/source/Engine.cpp
  source/source/Engine.h
  source/source/Public-headers/TECkit_Common.h
  source/source/Public-headers/TECkit_Engine.h
  source/source/TECkit_Format.h
  teckit-version.h
)

set(${teckit_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/teckit.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/teckit.rc
  )
  set(${teckit_dll_name}_sources ${${teckit_dll_name}_sources}
    teckit.def
    ${CMAKE_CURRENT_BINARY_DIR}/teckit.rc
  )
endif()

add_library(${teckit_dll_name} SHARED ${${teckit_dll_name}_sources})

set_property(TARGET ${teckit_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${teckit_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_include_directories(${teckit_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${teckit_dll_name}
  PRIVATE
    ${zlib_dll_name}
)

delay_load(${teckit_dll_name}
  ${zlib_dll_name}
)

rebase(${teckit_dll_name})

install(TARGETS ${teckit_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
