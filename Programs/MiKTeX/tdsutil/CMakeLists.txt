## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2016 Christian Schenk
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

include(MergeCompatibilityManifest)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETTINGS_FOLDER}/tdsutil")

if(LINK_EVERYTHING_STATICALLY)
  use_static_crt()
endif()

if(MSVC)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()  

set(tdsutil_sources
  Recipe.cpp
  Recipe.h
  TDS.h
  StdAfx.h
  internal.h
  tdsutil-version.h
  tdsutil.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND tdsutil_sources tdsutil.rc)
endif()

add_executable(tdsutil ${tdsutil_sources})

set_property(TARGET tdsutil PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(tdsutil
  ${app_dll_name}
  ${core_dll_name}
  miktex-popt-wrapper
)

merge_trustinfo_manifest(tdsutil asInvoker)
merge_compatibility_manifest(tdsutil)

install(TARGETS tdsutil DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
