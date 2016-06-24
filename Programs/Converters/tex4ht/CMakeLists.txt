## CMakeLists.txt					-*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_CONVERTERS_FOLDER}/tex4ht")

add_definitions(
  -DANSI
  -DCDECL=__cdecl
  -DCPLUSPLUSMAIN
  -DDISABLE_INSTALLER
  -DKPATHSEA
  -Dmain=Main
  -DUSE_MIKTEX_EXIT
)

ignore_warnings()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DWIN32
    -D_UNICODE -DUNICODE
  )
endif()

set_source_files_properties(
  source/t4ht.c
  source/tex4ht.c
  PROPERTIES LANGUAGE CXX
)

set(t4ht_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/t4ht.c
  t4ht-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND t4ht_sources
    t4ht.rc
  )
endif()

add_executable(t4ht ${t4ht_sources})

set_property(TARGET t4ht PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(t4ht asInvoker)
merge_compatibility_manifest(t4ht)

target_link_libraries(t4ht
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(t4ht
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS t4ht DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(tex4ht_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  source/tex4ht.c
  tex4ht-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND tex4ht_sources
    tex4ht.rc
  )
endif()

add_executable(tex4ht ${tex4ht_sources})

set_property(TARGET tex4ht PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(tex4ht asInvoker)
merge_compatibility_manifest(tex4ht)

target_link_libraries(tex4ht
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(tex4ht
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS tex4ht DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
