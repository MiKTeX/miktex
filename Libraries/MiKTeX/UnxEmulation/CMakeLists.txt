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

hide_symbols()

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/unxemu")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/unxemu-config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/unxemu-config.h
)

set(interface_definitions
  -DHAVE_DIRENT_H=1
  -DHAVE_UNISTD_H=1
)
  
set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(unx_includes
  dirent.h
  unistd.h
)

foreach(inc ${unx_includes})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/unx.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/${inc}
  )
endforeach()

set(public_headers
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/unxemu-config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/unxemu.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/sysexits.h
)

set(lib_sources
  ${public_headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/unxemu-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/unxemu.cpp
)

install(
  FILES
    ${public_headers}
  DESTINATION
    ${MIKTEX_HEADER_DESTINATION_DIR}/miktex
)

foreach(inc ${unx_includes})
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/${inc}
  DESTINATION
    ${MIKTEX_HEADER_DESTINATION_DIR}
  )
endforeach()

add_subdirectory(shared)
add_subdirectory(static)
