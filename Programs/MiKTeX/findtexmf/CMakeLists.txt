## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2018 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/${MIKTEX_PROG_NAME_FINDTEXMF}")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(findtexmf_sources
  findtexmf-version.h
  findtexmf.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    findtexmf.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/findtexmf.rc
  )
  list(APPEND findtexmf_sources
    ${CMAKE_CURRENT_BINARY_DIR}/findtexmf.rc
    ${MIKTEX_COMMON_MANIFEST}
  )
endif()

add_executable(${MIKTEX_PROG_NAME_FINDTEXMF} ${findtexmf_sources})

set_property(TARGET ${MIKTEX_PROG_NAME_FINDTEXMF} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if (USE_SYSTEM_FMT)
  target_link_libraries(${MIKTEX_PROG_NAME_FINDTEXMF} MiKTeX::Imported::FMT)
else()
  target_link_libraries(${MIKTEX_PROG_NAME_FINDTEXMF} ${fmt_dll_name})
endif()

target_link_libraries(${MIKTEX_PROG_NAME_FINDTEXMF}
  ${app_dll_name}
  ${core_dll_name}
  ${mpm_dll_name}
  miktex-popt-wrapper
)

install(TARGETS ${MIKTEX_PROG_NAME_FINDTEXMF}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
