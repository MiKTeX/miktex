## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_PREPROCESSORS_FOLDER}/devnag")

add_definitions(
  -DCPLUSPLUSMAIN
  -DUSE_MIKTEX_EXIT
  -Dmain=Main
)

ignore_warnings()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

set_source_files_properties(
  source/src/devnag.c
  PROPERTIES LANGUAGE CXX
)

set(devnag_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/src/devnag.c
  devnag-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND devnag_sources
    devnag.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}devnag ${devnag_sources})

set_property(TARGET ${MIKTEX_PREFIX}devnag PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}devnag asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}devnag)

target_link_libraries(${MIKTEX_PREFIX}devnag
  ${app_dll_name}
  ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}devnag
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}devnag DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
