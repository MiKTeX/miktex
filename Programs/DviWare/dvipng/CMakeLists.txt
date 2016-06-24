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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_DVIWARE_FOLDER}/dvipng")

## cannot compile as C++: too many implicit casts
set(DVIPNG_COMPILE_AS_CXX 0)

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(dvipng_c_sources
  source/color.c
  source/draw.c
  source/dvi.c
  source/dvipng.c
  source/enc.c
  source/font.c
  source/fontmap.c
  source/ft.c
  source/misc.c
  source/papersiz.c
  source/pk.c
  source/ppagelist.c
  source/set.c
  source/sfd.c
  source/special.c
  source/tfm.c
  source/vf.c
)

set(dvipng_sources
  ${dvipng_c_sources}
  ${MIKTEX_LIBRARY_WRAPPER}
  config.h
  dvipng-version.h
  source/commands.h
  source/dvipng.h
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -D_UNICODE
    -DUNICODE
  )
endif()

ignore_warnings()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dvipng_sources
    dvipng.rc
  )
endif()

if(DVIPNG_COMPILE_AS_CXX)
  set_source_files_properties(${dvipng_c_sources}
    PROPERTIES
      LANGUAGE CXX
  )
  set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES
      COMPILE_FLAGS "-DCPLUSPLUSMAIN -DBEQUIET"
  )
else()
  set_source_files_properties(${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES
      COMPILE_FLAGS "-DBEQUIET"
  )
endif()

add_executable(dvipng ${dvipng_sources})

set_property(TARGET dvipng PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

merge_trustinfo_manifest(dvipng asInvoker)
merge_compatibility_manifest(dvipng)

target_link_libraries(dvipng
  ${app_dll_name}
  ${core_dll_name}
  ${freetype2_dll_name}
  ${gd_dll_name}
  ${kpsemu_dll_name}
  ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(dvipng
    ${getopt_dll_name}
    ${unxemu_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

install(TARGETS dvipng DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
