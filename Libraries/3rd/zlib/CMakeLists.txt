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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/zlib")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

add_definitions(
  -DHAVE_SYS_TYPES_H
  -DHAVE_STDINT_H
  -DHAVE_STDDEF_H
  -D_LARGEFILE64_SOURCE=1
)

set(interface_definitions
    -DHAVE_LIBZ=1
    -DHAVE_ZLIB=1
    -DHAVE_ZLIB_COMPRESS2=1
    -DHAVE_ZLIB_H=1
)

ignore_warnings()

set(public_headers
  zlib.h
  zconf.h
)

foreach(h ${public_headers})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

set(zlib_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/adler32.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/compress.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crc32.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crc32.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/deflate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/deflate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/gzclose.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/gzguts.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/gzlib.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/gzread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/gzwrite.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/infback.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inffast.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inffast.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inffixed.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inflate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inflate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inftrees.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/inftrees.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/trees.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/trees.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/uncompr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zutil.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zutil.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/zconf.h

  ${CMAKE_CURRENT_SOURCE_DIR}/zlib-version.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
