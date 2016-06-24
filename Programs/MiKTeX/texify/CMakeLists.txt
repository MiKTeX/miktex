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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/texify")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(${mcd_dll_name}_sources
  mcd-version.h
  mcd.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    mcd.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mcd.rc
  )
  list(APPEND ${mcd_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mcd.rc)
endif()

add_library(${mcd_dll_name} SHARED ${${mcd_dll_name}_sources})

set_property(TARGET ${mcd_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${mcd_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_link_libraries(${mcd_dll_name}
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${texmf_dll_name}
    miktex-popt-wrapper
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${mcd_dll_name}
    PRIVATE
      ${regex_dll_name}
  )
endif()

rebase(${mcd_dll_name})
  
create_dll_wrapper(texify ${mcd_dll_name} mcdmain2)

install(TARGETS ${mcd_dll_name} texify
      RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
      LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
      ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}")
