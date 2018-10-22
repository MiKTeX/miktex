## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2018 Christian Schenk
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

# Base and default-included sources and headers

set(HB_BASE_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-atomic.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-blob.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-blob.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer-serialize.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-cache.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-common.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-debug.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-dsalgs.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-face.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-face.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-font.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-font.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-iter.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-map.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-map.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-machinery.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-mutex.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-null.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-object.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-open-file.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-open-type.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-color-cbdt-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-cmap-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-glyf-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-hdmx-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-head-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-hhea-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-hmtx-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-kern-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-maxp-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-name-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-os2-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-os2-unicode-ranges.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-post-macroman.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-post-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-tag-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-tag.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set-digest.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape-plan.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape-plan.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper-list.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper-impl.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shaper.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-static.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-string-array.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-unicode.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-unicode-emoji-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-unicode.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-vector.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-utf.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-warning.cc
)

set(HB_BASE_RAGEL_GENERATED_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer-deserialize-json.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer-deserialize-text.hh
)

set(HB_BASE_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-blob.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-deprecated.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-face.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-font.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-map.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape-plan.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-unicode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-version.h
)

set(HB_FALLBACK_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-fallback-shape.cc
)

set(HB_OT_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout-common.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout-ankr-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout-bsln-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout-feat-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout-kerx-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout-morx-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout-trak-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-ltag-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-face.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-face.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-font.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-base-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-common.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gdef-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gpos-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gsubgpos.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-gsub-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout-jstf-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-color.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-color-colr-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-color-cpal-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-color-sbix-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-color-svg-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-map.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-map.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-math.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-math-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic-fallback.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-arabic-win1256.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-default.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-hangul.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-hebrew.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic-table.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-khmer.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-khmer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-myanmar.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-myanmar.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-thai.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use-table.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-normalize.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-normalize.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-fallback.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-fallback.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-avar-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-fvar-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-hvar-table.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var-mvar-table.hh
)

set(HB_OT_RAGEL_GENERATED_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-indic-machine.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-khmer-machine.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-myanmar-machine.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape-complex-use-machine.hh
)

set(HB_OT_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-font.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-math.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-name.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-tag.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var.h
)

set(HB_FT_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ft.cc
)

set(HB_FT_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ft.h
)

set(HB_GLIB_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-glib.cc
)

set(HB_GLIB_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-glib.h
)

set(HB_GRAPHITE2_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-graphite2.cc
)

set(HB_GRAPHITE2_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-graphite2.h
)

set(HB_CORETEXT_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-coretext.cc
)

set(HB_CORETEXT_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-coretext.h
)

set(HB_DIRECTWRITE_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-directwrite.cc
)

set(HB_DIRECTWRITE_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-directwrite.h
)

set(HB_UNISCRIBE_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-uniscribe.cc
)

set(HB_UNISCRIBE_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-uniscribe.h
)

set(HB_UCDN_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ucdn.cc
)

set(HB_ICU_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-icu.cc
)

set(HB_ICU_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-icu.h
)

set(HB_SUBSET_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-static.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-glyf.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-glyf.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-input.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-input.hh
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-plan.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-plan.hh
)

set(HB_SUBSET_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset.h
)

set(HB_GOBJECT_DIST_sources
  hb-gobject-structs.cc
)

set(HB_GOBJECT_DIST_headers
  hb-gobject.h hb-gobject-structs.h
)

set(HB_GOBJECT_ENUM_sources
  hb-gobject-enums.cc
)

set(HB_GOBJECT_ENUM_headers
  hb-gobject-enums.h
)

set(HB_GOBJECT_NODIST_sources
  ${HB_GOBJECT_ENUM_sources}
)

set(HB_GOBJECT_NODIST_headers
  ${HB_GOBJECT_ENUM_headers}
)

set(HB_GOBJECT_sources
  ${HB_GOBJECT_DIST_sources}
  ${HB_GOBJECT_NODIST_sources}
)

set(HB_GOBJECT_headers
  ${HB_GOBJECT_DIST_headers}
  ${HB_GOBJECT_NODIST_headers}
)
