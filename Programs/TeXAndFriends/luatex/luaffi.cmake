## luaffi.cmake
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

set(luaffi_sources
  source/luaffi/call.c
  source/luaffi/call_arm.h
  source/luaffi/call_x64.h
  source/luaffi/call_x64win.h
  source/luaffi/call_x86.h
  source/luaffi/ctype.c
  source/luaffi/ffi.c
  source/luaffi/ffi.h
  source/luaffi/parser.c
) 

add_library(luatex-lua53ffi-objects OBJECT ${luaffi_sources})

set_property(TARGET luatex-lua53ffi-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-lua53ffi-objects
  PRIVATE
    source/luaffi/dynasm
)

target_link_libraries(luatex-lua53ffi-objects
  PUBLIC
    ${CMAKE_DL_LIBS}
    ${lua53_target_name}
)
if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-lua53ffi-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-lua53ffi-objects PUBLIC ${zlib_dll_name})
endif()

if(WITH_LUA54TEX)
  add_library(luatex-lua54ffi-objects OBJECT ${luaffi_sources})
  set_property(TARGET luatex-lua54ffi-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_include_directories(luatex-lua54ffi-objects
    PRIVATE
      source/luaffi/dynasm
  )
  target_link_libraries(luatex-lua54ffi-objects
    PUBLIC
      ${lua54_target_name}
  )
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-lua54ffi-objects PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(luatex-lua54ffi-objects PUBLIC ${zlib_dll_name})
  endif()
endif()
