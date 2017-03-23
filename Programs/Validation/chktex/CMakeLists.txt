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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_VALIDATION_FOLDER}/chktex")

if(MIKTEX_NATIVE_WINDOWS)
  set(HAVE_ACCESS 1)
  set(HAVE_OPENDIR 1)
  set(HAVE_READDIR 1)
  set(HAVE_CLOSEDIR 1)
endif()

set(HAVE_POSIX_ERE 1)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DKPATHSEA
  -DTEX_LIVE
  -DUSE_MIKTEX_EXIT
  -Dmain=Main
)

ignore_warnings()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(chktex_c_sources
  source/ChkTeX.c
  source/FindErrs.c
  source/OpSys.c
  source/Resource.c
  source/Utility.c
)

set(chktex_h_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  source/ChkTeX.h
  source/FindErrs.h
  source/OpSys.h
  source/Resource.h
  source/Utility.h
  source/types.h
)

#set_source_files_properties(
#  ${chktex_c_sources}
#  PROPERTIES LANGUAGE CXX
#)

set(chktex_sources
  ${MIKTEX_LIBRARY_WRAPPER}
  ${chktex_c_sources}
  ${chktex_h_sources}
  chktex-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND chktex_sources
    chktex.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}chktex ${chktex_sources})

set_property(TARGET ${MIKTEX_PREFIX}chktex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}chktex asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}chktex)

target_link_libraries(${MIKTEX_PREFIX}chktex
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}chktex
    ${regex_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}chktex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
