## luapplib.cmake
##
## Copyright (C) 2010-2019 Christian Schenk
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
  source/luapplib/ppapi.h
  source/luapplib/pparray.c
  source/luapplib/pparray.h
  source/luapplib/ppconf.h
  source/luapplib/ppcrypt.c
  source/luapplib/ppcrypt.h
  source/luapplib/ppdict.c
  source/luapplib/ppdict.h
  source/luapplib/ppfilter.h
  source/luapplib/ppheap.c
  source/luapplib/ppheap.h
  source/luapplib/pplib.h
  source/luapplib/ppload.c
  source/luapplib/ppload.h
  source/luapplib/ppstream.c
  source/luapplib/ppstream.h
  source/luapplib/ppxref.c
  source/luapplib/ppxref.h
  source/luapplib/util/utilarm.h
  source/luapplib/util/utilbasexx.c
  source/luapplib/util/utilbasexx.h
  source/luapplib/util/utilcrypt.c
  source/luapplib/util/utilcrypt.h
  source/luapplib/util/utilcryptdef.h
  source/luapplib/util/utildecl.h
  source/luapplib/util/utilflate.c
  source/luapplib/util/utilflate.h
  source/luapplib/util/utilfpred.c
  source/luapplib/util/utilfpred.h
  source/luapplib/util/utiliof.c
  source/luapplib/util/utiliof.h
  source/luapplib/util/utillog.c
  source/luapplib/util/utillog.h
  source/luapplib/util/utillzw.c
  source/luapplib/util/utillzw.h
  source/luapplib/util/utilmd5.c
  source/luapplib/util/utilmd5.h
  source/luapplib/util/utilmem.c
  source/luapplib/util/utilmem.h
  source/luapplib/util/utilnumber.c
  source/luapplib/util/utilnumber.h
  source/luapplib/util/utilplat.h
  source/luapplib/util/utilsha.c
  source/luapplib/util/utilsha.h
)

add_library(luatex-lua53pplib-objects OBJECT ${luapplib_sources})

set_property(TARGET luatex-lua53pplib-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-lua53pplib-objects
  PUBLIC
    source/luapplib/util
)

if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-lua53pplib-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-lua53pplib-objects PUBLIC ${zlib_dll_name})
endif()

target_link_libraries(luatex-lua53pplib-objects
  PUBLIC
    ${core_dll_name}
    ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(luatex-lua53pplib-objects
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(WITH_LUA54TEX)
  add_library(luatex-lua54pplib-objects OBJECT ${luapplib_sources})
  set_property(TARGET luatex-lua54pplib-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_include_directories(luatex-lua54pplib-objects
    PUBLIC
      source/luapplib/util
  )
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-lua54pplib-objects PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(luatex-lua54pplib-objects PUBLIC ${zlib_dll_name})
  endif()
  target_link_libraries(luatex-lua54pplib-objects
    PUBLIC
      ${core_dll_name}
      ${lua54_target_name}
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(luatex-lua54pplib-objects
      PUBLIC
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
  endif()
endif()
