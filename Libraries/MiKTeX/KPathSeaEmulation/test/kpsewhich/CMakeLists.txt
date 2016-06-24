## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/kpsemu/kpsewhich")

add_definitions(
  -DUSE_MIKTEX_EXIT
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()  

set(kpsewhich_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  kpsewhich-version.h
  kpsewhich.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND kpsewhich_sources kpsewhich.rc)
endif()

add_executable(${MIKTEX_PREFIX}kpsewhich ${kpsewhich_sources})

set_property(TARGET ${MIKTEX_PREFIX}kpsewhich PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set(libs
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND libs
    ${utf8wrap_dll_name}
  )
endif()

if(NOT HAVE_GETOPT_LONG)
  list(APPEND libs ${getopt_dll_name})
endif()

target_link_libraries(${MIKTEX_PREFIX}kpsewhich ${libs})

merge_trustinfo_manifest(${MIKTEX_PREFIX}kpsewhich asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}kpsewhich)

install(TARGETS ${MIKTEX_PREFIX}kpsewhich DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
