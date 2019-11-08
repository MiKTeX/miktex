## luamisc.cmake
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

set(luamisc_sources
  source/luafilesystem/src/lfs.c
  source/luafilesystem/src/lfs.h
  source/luamd5/luamd5.h
  source/luamd5/md5.c
  source/luamd5/md5lib.c
  source/luapeg/lpeg.c
  source/luapeg/lpeg.h
  source/luazip/src/luazip.c
  source/luazip/src/luazip.h
  source/luazlib/lgzip.c
  source/luazlib/lzlib.c
  source/slnunicode/slnunico.c
)

add_library(luatex-lua53misc-objects OBJECT ${luamisc_sources})

set_property(TARGET luatex-lua53misc-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(luatex-lua53misc-objects
  PRIVATE
    -DLUAZIP_API=
)

if(USE_SYSTEM_ZLIB)
  target_link_libraries(luatex-lua53misc-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(luatex-lua53misc-objects PUBLIC ${zlib_dll_name})
endif()

if(USE_SYSTEM_ZZIP)
  target_link_libraries(luatex-lua53misc-objects PUBLIC MiKTeX::Imported::ZZIP)
else()
  target_link_libraries(luatex-lua53misc-objects PUBLIC ${zzip_dll_name})
endif()

target_link_libraries(luatex-lua53misc-objects
  PUBLIC
    ${core_dll_name}
    ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(luatex-lua53misc-objects
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()
