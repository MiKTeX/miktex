## sources.cmake
##
## Copyright (C) 2020-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(project_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/harfbuzz.cc

	${CMAKE_CURRENT_SOURCE_DIR}/source/src/graph/gsubgpos-context.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-cff-common.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-cff1.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-cff2.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-input.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-instancer-iup.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-instancer-solver.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-plan-layout.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-plan-var.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-plan.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-serialize.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-table-cff.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-table-color.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-table-layout.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-table-other.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset-table-var.cc
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-subset.cc
)

set (project_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat-layout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-aat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-blob.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-cplusplus.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-deprecated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-draw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-face.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-font.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-map.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-color.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-deprecated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-font.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-layout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-math.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-meta.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-metrics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-name.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-shape.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot-var.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-ot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-paint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-set.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape-plan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-shape.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-style.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-unicode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb.hh
)

list(APPEND project_headers ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hb-graphite2.h)
