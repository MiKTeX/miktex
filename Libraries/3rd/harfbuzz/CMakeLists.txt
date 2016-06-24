## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2013-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/harfbuzz")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/source/src
)

add_definitions(-DHAVE_CONFIG_H -DHB_NO_MT -DHB_NO_UNICODE_FUNCS)

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

set(sources
  source/src/hb-atomic-private.hh
  source/src/hb-blob.cc
  source/src/hb-buffer-deserialize-json.hh
  source/src/hb-buffer-deserialize-text.hh
  source/src/hb-buffer-private.hh
  source/src/hb-buffer-serialize.cc
  source/src/hb-buffer.cc
  source/src/hb-cache-private.hh
  source/src/hb-common.cc
  source/src/hb-face-private.hh
  source/src/hb-face.cc
  source/src/hb-fallback-shape.cc
  source/src/hb-font-private.hh
  source/src/hb-font.cc
  source/src/hb-mutex-private.hh
  source/src/hb-object-private.hh
  source/src/hb-open-file-private.hh
  source/src/hb-open-type-private.hh
  source/src/hb-ot-head-table.hh
  source/src/hb-ot-hhea-table.hh
  source/src/hb-ot-hmtx-table.hh
  source/src/hb-ot-maxp-table.hh
  source/src/hb-ot-name-table.hh
  source/src/hb-ot-os2-table.hh
  source/src/hb-ot-tag.cc
  source/src/hb-private.hh
  source/src/hb-set-private.hh
  source/src/hb-set.cc
  source/src/hb-shape.cc
  source/src/hb-shape-plan-private.hh
  source/src/hb-shape-plan.cc
  source/src/hb-shaper-list.hh
  source/src/hb-shaper-impl-private.hh
  source/src/hb-shaper-private.hh
  source/src/hb-shaper.cc
  source/src/hb-unicode-private.hh
  source/src/hb-unicode.cc
  source/src/hb-utf-private.hh
  source/src/hb-warning.cc

  source/src/hb-ot-font.cc
  source/src/hb-ot-font.h
  source/src/hb-ot-layout.cc
  source/src/hb-ot-layout-common-private.hh
  source/src/hb-ot-layout-gdef-table.hh
  source/src/hb-ot-layout-gpos-table.hh
  source/src/hb-ot-layout-gsubgpos-private.hh
  source/src/hb-ot-layout-gsub-table.hh
  source/src/hb-ot-layout-jstf-table.hh
  source/src/hb-ot-layout-private.hh
  source/src/hb-ot-map.cc
  source/src/hb-ot-map-private.hh
  source/src/hb-ot-shape.cc
  source/src/hb-ot-shape-complex-arabic.cc
  source/src/hb-ot-shape-complex-arabic-fallback.hh
  source/src/hb-ot-shape-complex-arabic-table.hh
  source/src/hb-ot-shape-complex-arabic-private.hh
  source/src/hb-ot-shape-complex-default.cc
  source/src/hb-ot-shape-complex-hangul.cc
  source/src/hb-ot-shape-complex-hebrew.cc
  source/src/hb-ot-shape-complex-indic.cc
  source/src/hb-ot-shape-complex-indic-machine.hh
  source/src/hb-ot-shape-complex-indic-private.hh
  source/src/hb-ot-shape-complex-indic-table.cc
  source/src/hb-ot-shape-complex-myanmar.cc
  source/src/hb-ot-shape-complex-myanmar-machine.hh
  source/src/hb-ot-shape-complex-private.hh
  source/src/hb-ot-shape-complex-thai.cc
  source/src/hb-ot-shape-complex-tibetan.cc
  source/src/hb-ot-shape-complex-use.cc
  source/src/hb-ot-shape-complex-use-machine.hh
  source/src/hb-ot-shape-complex-use-private.hh
  source/src/hb-ot-shape-complex-use-table.cc
  source/src/hb-ot-shape-fallback.cc
  source/src/hb-ot-shape-fallback-private.hh
  source/src/hb-ot-shape-normalize.cc
  source/src/hb-ot-shape-normalize-private.hh
  source/src/hb-ot-shape-private.hh

  source/src/hb-icu.cc

  source/src/hb-graphite2.cc
)

add_library(${harfbuzz_lib_name} STATIC ${sources})

set_property(TARGET ${harfbuzz_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${harfbuzz_lib_name}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src
    ${ICU_INCLUDE_DIRS}
)

target_link_libraries(${harfbuzz_lib_name}
  PUBLIC
    ${graphite2_dll_name}
    ${ICU_LIBRARIES}
)
