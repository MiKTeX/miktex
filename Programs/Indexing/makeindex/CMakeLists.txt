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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_INDEXING_FOLDER}/MakeIndex")

add_definitions(
  -DHAVE_CTYPE_H
  -DHAVE_LOCALE_H
  -DHAVE_SETLOCALE
  -DHAVE_STRCHR
  -DHAVE_STRINGS_H
  -DHAVE_STRRCHR
  -UDEBUG
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DIBM_PC_MICROSOFT
    -DOS_WIN32
    -DUNICODE
    -D_UNICODE
  )
endif()

ignore_warnings()

set(makeindex_c_sources
  source/genind.c
  source/mkind.c
  source/qsort.c
  source/scanid.c
  source/scanst.c
  source/sortid.c
)

set(makeindex_sources
  ${makeindex_c_sources}
  ${MIKTEX_LIBRARY_WRAPPER}
  makeindex-version.h
  source/genind.h
  source/mkind.h
  source/scanid.h
  source/scanst.h
)

set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
  PROPERTIES
    COMPILE_FLAGS "-DCPLUSPLUSMAIN -DBEQUIET"
)

set_source_files_properties(${makeindex_c_sources}
  PROPERTIES
    LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND makeindex_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    makeindex.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}makeindex ${makeindex_sources})

set_property(TARGET ${MIKTEX_PREFIX}makeindex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}makeindex
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}makeindex
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS ${MIKTEX_PREFIX}makeindex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
