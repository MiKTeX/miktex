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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}")

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(gsf2pk_sources
  gsf2pk-version.h
  gsf2pk.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND gsf2pk_sources gsf2pk.rc)
endif()

add_executable(gsf2pk ${gsf2pk_sources})

set_property(TARGET gsf2pk PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(gsf2pk
  ${app_dll_name}
  ${core_dll_name}
  ${texmf_dll_name}
  Threads::Threads
  miktex-popt-wrapper
)

merge_trustinfo_manifest(gsf2pk asInvoker)
merge_compatibility_manifest(gsf2pk)

install(TARGETS gsf2pk DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
