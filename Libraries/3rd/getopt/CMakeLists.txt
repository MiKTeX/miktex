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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/getopt")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories ${CMAKE_CURRENT_BINARY_DIR}/include)

set(public_headers
  getopt.h
)

foreach(h ${public_headers})
  configure_file(
    source/posix/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

set(getopt_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/getopt-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/posix/getopt_int.h
)

add_subdirectory(shared)

add_subdirectory(static)
