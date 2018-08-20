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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/initexmf")

if(LINK_EVERYTHING_STATICALLY)
  use_static_crt()
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DHAVE_CONFIG_H=1
)

if(NOT USE_SYSTEM_POPPLER)
  add_definitions(
    -DWITH_POPPLER_UTILS=1
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()  

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(initexmf_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  StdAfx.h
  aoption_setup.h
  aoption_update.h
  aoption_user.h
  initexmf-version.h
  initexmf.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND initexmf_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    initexmf.rc
  )
endif()

add_executable(initexmf ${initexmf_sources})

set_property(TARGET initexmf PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS initexmf DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

if (USE_SYSTEM_LOG4CXX)
  target_link_libraries(initexmf MiKTeX::Imported::LOG4CXX)
else()
  target_link_libraries(initexmf ${log4cxx_dll_name})
endif()

target_link_libraries(initexmf
  ${core_dll_name}
  ${mpm_dll_name}
  ${setup_dll_name}
  miktex-popt-wrapper
)

target_link_libraries(initexmf
  ${texmf_dll_name}
)
