## libunilib.cmake
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

set(unilib_sources
  source/unilib/ArabicForms.c
  source/unilib/alphabet.c
  source/unilib/char.c
  source/unilib/chardata.h
  source/unilib/charset.h
  source/unilib/cjk.c
  source/unilib/combiners.h
  source/unilib/gwwiconv.c
  source/unilib/gwwiconv.h
  source/unilib/ucharmap.c
  source/unilib/unialt.c
  source/unilib/unibasics.h
  source/unilib/usprintf.c
  source/unilib/ustring.c
  source/unilib/ustring.h
  source/unilib/utype.c
  source/unilib/utype.h
)

add_library(luatex-unilib-objects OBJECT ${unilib_sources})

set_property(TARGET luatex-unilib-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-unilib-objects
  PUBLIC
    source/unilib
)

target_link_libraries(luatex-unilib-objects
  PUBLIC
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)
