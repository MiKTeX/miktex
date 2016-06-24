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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/arctrl")

add_definitions(
  -D_UNICODE -DUNICODE
)

set(${arctrl_dll_name}_sources
  arctrl-version.h
  arctrl.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND ${arctrl_dll_name}_sources
    arctrl.rc)
endif()

add_library(${arctrl_dll_name} SHARED ${${arctrl_dll_name}_sources})

set_property(TARGET ${arctrl_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${arctrl_dll_name}
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    miktex-popt-wrapper
)

rebase(${arctrl_dll_name})

install(TARGETS ${arctrl_dll_name} ${exes} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

create_dll_wrapper(arctrl ${arctrl_dll_name} arctrlmain)
