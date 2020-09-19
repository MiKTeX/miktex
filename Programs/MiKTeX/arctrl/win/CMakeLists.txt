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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/${MIKTEX_PROG_NAME_ARCTRL}")

add_definitions(
  -D_UNICODE -DUNICODE
)

set(arctrl_sources
  arctrl-version.h
  arctrl.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND arctrl_sources
    ${MIKTEX_COMMON_MANIFEST}
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

if (USE_SYSTEM_FMT)
  target_link_libraries(_arctrl PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(_arctrl PRIVATE ${fmt_dll_name})
endif()

create_dll_wrapper(${MIKTEX_PROG_NAME_ARCTRL} _arctrl arctrlmain)
