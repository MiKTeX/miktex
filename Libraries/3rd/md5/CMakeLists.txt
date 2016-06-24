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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/md5")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

ignore_warnings()

set(md5_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/md5-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/md5.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/md5.h
)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/source/md5.h
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}
  )
endif()

configure_file(
  source/md5.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/md5.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()


add_subdirectory(static)
