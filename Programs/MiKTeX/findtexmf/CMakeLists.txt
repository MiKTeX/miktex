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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/findtexmf")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(${findtexmf_dll_name}_sources
  findtexmf-version.h
  findtexmf.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    findtexmf.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/findtexmf.rc
  )
  list(APPEND ${findtexmf_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/findtexmf.rc)
endif()

add_library(${findtexmf_dll_name} SHARED ${${findtexmf_dll_name}_sources})

set_property(TARGET ${findtexmf_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${findtexmf_dll_name}
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${mpm_dll_name}
    miktex-popt-wrapper
)

rebase(${findtexmf_dll_name})
  
set(exes findtexmf)

foreach(t ${exes})
  create_dll_wrapper(${t} ${findtexmf_dll_name} findtexmf)
endforeach()

install(TARGETS ${findtexmf_dll_name} ${exes} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
