## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2016 Christian Schenk
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

set(fribidi_sources
  
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/arabic-misc.tab.i
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/arabic-shaping.tab.i
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/bidi-type.tab.i
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/bidi-types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/debug.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-arabic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-bidi-types.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-bidi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-deprecated.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-joining-types.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-joining.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-mem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-mirroring.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-run.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-shape.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-unicode-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/joining-type.tab.i
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/joining-types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/mem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/mirroring.tab.i
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/run.h

  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-cap-rtl.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-cp1255.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-cp1256.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-iso8859-6.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-iso8859-8.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-utf8.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-cap-rtl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-cp1255.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-cp1256.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-iso8859-6.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-iso8859-8.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset/fribidi-char-sets-utf8.h
)
