## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

set(arctrl_sources
  arctrl-version.h
  arctrl.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND arctrl_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    arctrl.rc)
endif()

add_library(_arctrl STATIC ${arctrl_sources})

set_property(TARGET _arctrl PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(_arctrl
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    miktex-popt-wrapper
)

create_dll_wrapper(arctrl _arctrl arctrlmain)
