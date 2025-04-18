## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2016-2022 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/tdsutil")

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()  

set(tdsutil_sources
  Recipe.cpp
  Recipe.h
  TDS.h
  internal.h
  tdsutil-version.h
  tdsutil.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND tdsutil_sources
    ${MIKTEX_COMMON_MANIFEST}
    tdsutil.rc
  )
endif()

add_executable(${MIKTEX_PROG_NAME_TDSUTIL} ${tdsutil_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_TDSUTIL} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PROG_NAME_TDSUTIL}
  ${app_dll_name}
  ${core_dll_name}
  miktex-popt-wrapper
)

if (USE_SYSTEM_FMT)
  target_link_libraries(${MIKTEX_PROG_NAME_TDSUTIL} MiKTeX::Imported::FMT)
else()
  target_link_libraries(${MIKTEX_PROG_NAME_TDSUTIL} ${fmt_dll_name})
endif()

install(TARGETS ${MIKTEX_PROG_NAME_TDSUTIL} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
