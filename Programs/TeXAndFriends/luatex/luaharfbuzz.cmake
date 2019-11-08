## luaharfbuzz.cmake
##
## Copyright (C) 2019 Christian Schenk
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

set(luaharfbuzz_sources
  source/luaharfbuzz/src/luaharfbuzz/blob.c
  source/luaharfbuzz/src/luaharfbuzz/buffer.c
  source/luaharfbuzz/src/luaharfbuzz/class_utils.c
  source/luaharfbuzz/src/luaharfbuzz/direction.c
  source/luaharfbuzz/src/luaharfbuzz/face.c
  source/luaharfbuzz/src/luaharfbuzz/feature.c
  source/luaharfbuzz/src/luaharfbuzz/font.c
  source/luaharfbuzz/src/luaharfbuzz/language.c
  source/luaharfbuzz/src/luaharfbuzz/luaharfbuzz.c
  source/luaharfbuzz/src/luaharfbuzz/luaharfbuzz.h
  source/luaharfbuzz/src/luaharfbuzz/ot.c
  source/luaharfbuzz/src/luaharfbuzz/script.c
  source/luaharfbuzz/src/luaharfbuzz/tag.c
  source/luaharfbuzz/src/luaharfbuzz/unicode.c
)

add_library(luahbtex-luaharfbuzz-objects OBJECT ${luaharfbuzz_sources})

set_property(TARGET luahbtex-luaharfbuzz-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_GRAPHITE2)
  target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC MiKTeX::Imported::GRAPHITE2)
else()
  target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC ${graphite2_dll_name})
endif()

if(USE_SYSTEM_HARFBUZZ_ICU)
  target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC MiKTeX::Imported::HARFBUZZ_ICU)
  target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC MiKTeX::Imported::HARFBUZZ)
else()
  target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC ${harfbuzz_dll_name})
endif()

target_link_libraries(luahbtex-luaharfbuzz-objects
  PUBLIC
    ${core_dll_name}
    ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(luahbtex-luaharfbuzz-objects
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()
