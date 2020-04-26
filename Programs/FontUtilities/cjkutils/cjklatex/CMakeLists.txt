## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/cjklatex")

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(cjklatex_sources
  cjklatex-version.h
  cjklatex.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    cjklatex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/cjklatex.rc
  )
  list(APPEND cjklatex_sources
    ${MIKTEX_COMMON_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/cjklatex.rc
  )
endif()

add_executable(cjklatex ${cjklatex_sources})

set_property(TARGET cjklatex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(cjklatex
  ${app_dll_name}
  ${core_dll_name}
  ${mpm_dll_name}
  ${texmf_dll_name}
)

if (USE_SYSTEM_FMT)
  target_link_libraries(cjklatex MiKTeX::Imported::FMT)
else()
  target_link_libraries(cjklatex ${fmt_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(cjklatex
    ${getopt_dll_name}
  )
endif()

install(TARGETS cjklatex
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
