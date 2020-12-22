## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2020 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/pplib")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include/util
)

add_definitions(
  -DHAVE_CONFIG_H
)

configure_file(
  pplib-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/pplib-version.h
)


set(public_headers
  ppapi.h
  pparray.h
  ppconf.h
  ppcrypt.h
  ppdict.h
  ppfilter.h
  ppheap.h
  pplib.h
  ppload.h
  ppstream.h
  ppxref.h
  util/utilbasexx.h
  util/utilcrypt.h
  util/utilcryptdef.h
  util/utildecl.h
  util/utilflate.h
  util/utilfpred.h
  util/utiliof.h
  util/utillog.h
  util/utillzw.h
  util/utilmd5.h
  util/utilmem.h
  util/utilmemallc.h
  util/utilmemallh.h
  util/utilmemheap.h
  util/utilmemheapiof.h
  util/utilmeminfo.h
  util/utilnumber.h
  util/utilplat.h
  util/utilsha.h
)

foreach(h ${public_headers})
  configure_file(
    source/src/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

set(pplib_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/pparray.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/pparray.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppconf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppcrypt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppcrypt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppdict.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppdict.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppfilter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppheap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppheap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/pplib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppload.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppload.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppstream.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppstream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppxref.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/ppxref.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilbasexx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilbasexx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilcrypt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilcrypt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilcryptdef.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utildecl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilflate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilflate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilfpred.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilfpred.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utiliof.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utiliof.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utillog.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utillog.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utillzw.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utillzw.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmd5.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmd5.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmemallc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmemallh.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmemheap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmemheap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmemheapiof.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmemheapiof.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmeminfo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilmeminfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilnumber.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilnumber.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilplat.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilsha.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/utilsha.h
)

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(static)
