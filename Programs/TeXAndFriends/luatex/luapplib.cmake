## luapplib.cmake
##
## Copyright (C) 2010-2020 Christian Schenk
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

set(luapplib_sources
  source/luapplib/src/ppapi.h
  source/luapplib/src/pparray.c
  source/luapplib/src/pparray.h
  source/luapplib/src/ppconf.h
  source/luapplib/src/ppcrypt.c
  source/luapplib/src/ppcrypt.h
  source/luapplib/src/ppdict.c
  source/luapplib/src/ppdict.h
  source/luapplib/src/ppfilter.h
  source/luapplib/src/ppheap.c
  source/luapplib/src/ppheap.h
  source/luapplib/src/pplib.h
  source/luapplib/src/ppload.c
  source/luapplib/src/ppload.h
  source/luapplib/src/ppstream.c
  source/luapplib/src/ppstream.h
  source/luapplib/src/ppxref.c
  source/luapplib/src/ppxref.h
  source/luapplib/src/util/utilbasexx.c
  source/luapplib/src/util/utilbasexx.h
  source/luapplib/src/util/utilcrypt.c
  source/luapplib/src/util/utilcrypt.h
  source/luapplib/src/util/utilcryptdef.h
  source/luapplib/src/util/utildecl.h
  source/luapplib/src/util/utilflate.c
  source/luapplib/src/util/utilflate.h
  source/luapplib/src/util/utilfpred.c
  source/luapplib/src/util/utilfpred.h
  source/luapplib/src/util/utiliof.c
  source/luapplib/src/util/utiliof.h
  source/luapplib/src/util/utillog.c
  source/luapplib/src/util/utillog.h
  source/luapplib/src/util/utillzw.c
  source/luapplib/src/util/utillzw.h
  source/luapplib/src/util/utilmd5.c
  source/luapplib/src/util/utilmd5.h
  source/luapplib/src/util/utilmem.c
  source/luapplib/src/util/utilmem.h
  source/luapplib/src/util/utilmemallc.h
  source/luapplib/src/util/utilmemallh.h
  source/luapplib/src/util/utilmemheap.c
  source/luapplib/src/util/utilmemheap.h
  source/luapplib/src/util/utilmemheapiof.c
  source/luapplib/src/util/utilmemheapiof.h
  source/luapplib/src/util/utilmeminfo.c
  source/luapplib/src/util/utilmeminfo.h
  source/luapplib/src/util/utilnumber.c
  source/luapplib/src/util/utilnumber.h
  source/luapplib/src/util/utilplat.h
  source/luapplib/src/util/utilsha.c
  source/luapplib/src/util/utilsha.h
)

add_library(luatex-luapplib-objects OBJECT ${luapplib_sources})

set_property(TARGET luatex-luapplib-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-luapplib-objects
  PUBLIC
    source/luapplib/src/util
)

if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-luapplib-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-luapplib-objects PUBLIC ${zlib_dll_name})
endif()

target_link_libraries(luatex-luapplib-objects
  PUBLIC
    ${core_dll_name}
    ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(luatex-luapplib-objects
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()
