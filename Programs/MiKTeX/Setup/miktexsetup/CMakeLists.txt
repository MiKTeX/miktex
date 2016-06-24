## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2014-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETUP_FOLDER}")

use_static_crt()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(miktexsetup_sources
  StdAfx.h
  internal.h
  miktexsetup-version.h
  miktexsetup.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND miktexsetup_sources miktexsetup.rc)
endif()

add_executable(miktexsetup ${miktexsetup_sources})

set_property(TARGET miktexsetup PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(miktexsetup
  ${core_lib_name}
  ${setup_lib_name}
  miktex-popt-wrapper-static
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(miktexsetup
    Wldap32
    shlwapi
    version
    wininet
    winmm
    ws2_32
  )
endif()

merge_trustinfo_manifest(miktexsetup asInvoker)
merge_compatibility_manifest(miktexsetup)

install(TARGETS miktexsetup DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
