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

include(MergeCompatibilityManifest)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_SETTINGS_FOLDER}/initexmf")

if(LINK_EVERYTHING_STATICALLY)
  use_static_crt()
endif()

add_definitions(
  -DWITH_KPSEWHICH=1
  -DWITH_MKTEXLSR=1
  -DWITH_TEXHASH=1
  -DWITH_TEXLINKS=1
)

if(MSVC)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()  

set(initexmf_sources
  StdAfx.h
  initexmf-version.h
  initexmf.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND initexmf_sources initexmf.rc)
endif()

add_executable(initexmf ${initexmf_sources})

set_property(TARGET initexmf PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS initexmf DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

target_link_libraries(initexmf
  ${core_dll_name}
  ${log4cxx_dll_name}
  ${mpm_dll_name}
  miktex-popt-wrapper
)

target_link_libraries(initexmf
  ${texmf_dll_name}
)

merge_trustinfo_manifest(initexmf asInvoker)
merge_compatibility_manifest(initexmf)
