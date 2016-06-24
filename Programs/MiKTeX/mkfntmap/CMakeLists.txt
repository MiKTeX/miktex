## CMakeLists.txt                                       -*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/mkfntmap")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

set(${mkfntmap_dll_name}_sources
  internal.h
  mkfntmap-version.h
  mkfntmap.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    mkfntmap.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mkfntmap.rc
  )
  list(APPEND ${mkfntmap_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mkfntmap.rc)
endif()

add_library(${mkfntmap_dll_name} SHARED ${${mkfntmap_dll_name}_sources})

set_property(TARGET ${mkfntmap_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${mkfntmap_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_link_libraries(${mkfntmap_dll_name}
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${log4cxx_dll_name}
    miktex-popt-wrapper
)

rebase(${mkfntmap_dll_name})

install(TARGETS ${mkfntmap_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

create_dll_wrapper(mkfntmap ${mkfntmap_dll_name} mkfntmap)

