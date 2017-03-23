## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_PREPROCESSORS_FOLDER}/fribidixetex")

ignore_warnings()

set(fribidixetex_sources
  fribidixetex-version.h
  source/src/fribidixetex-bidi.c
  source/src/fribidixetex-dict.c
  source/src/fribidixetex-ignore.c
  source/src/fribidixetex-io.c
  source/src/fribidixetex-util.c
  source/src/fribidixetex.c
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND fribidixetex_sources
    fribidixetex.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}fribidixetex ${fribidixetex_sources})

set_property(TARGET ${MIKTEX_PREFIX}fribidixetex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(${MIKTEX_PREFIX}fribidixetex asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}fribidixetex)

if(USE_SYSTEM_FRIBIDI)
  target_link_libraries(${MIKTEX_PREFIX}fribidixetex MiKTeX::Imported::FRIBIDI)
else()
  target_link_libraries(${MIKTEX_PREFIX}fribidixetex ${fribidi_lib_name})
endif()
  
install(TARGETS ${MIKTEX_PREFIX}fribidixetex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
