## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/harfbuzz")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/source/src
)

set(public_include_directories ${CMAKE_CURRENT_SOURCE_DIR}/source/src)

add_definitions(
  -DHAVE_CONFIG_H
  -DHAVE_FALLBACK
  -DHB_NO_MT
  -DHB_NO_UNICODE_FUNCS
)

ignore_warnings()

set(HAVE_GRAPHITE2 1)
set(HAVE_ICU 1)
set(HAVE_OT 1)

if(MIKTEX_NATIVE_WINDOWS)
  set(HAVE_WINDOWS_H 1)
endif()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

list(APPEND harfbuzz_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  harfbuzz-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz-version.h
)

list(APPEND harfbuzz_sources
  ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz-version.h
)

## common
list(APPEND harfbuzz_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-atomic-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-blob.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer-deserialize-json.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer-deserialize-text.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer-serialize.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-common.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-face-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-face.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-fallback-shape.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-font-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-font.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-mutex-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-object-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-open-file-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-open-type-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-cbdt-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-head-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-hhea-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-hmtx-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-maxp-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-name-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-os2-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-post-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-tag.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set-digest-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape-plan-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape-plan.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper-impl-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper-list.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-unicode-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-unicode.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-utf-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-warning.cc
)

## OpenType
list(APPEND harfbuzz_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-font.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-font.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-common-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gdef-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gpos-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gsubgpos-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gsub-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-jstf-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-map.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-math.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-math-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-map-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-math.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic-fallback.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-default.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-hangul.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-hebrew.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic-machine.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic-table.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-myanmar.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-myanmar-machine.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-thai.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-tibetan.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use-machine.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use-table.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-fallback.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-fallback-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-normalize.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-normalize-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-private.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-avar-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-fvar-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-hvar-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-mvar-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var.h
)

## ICU
list(APPEND harfbuzz_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-icu.cc
)

## Graphite2
list(APPEND harfbuzz_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-graphite2.cc
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
