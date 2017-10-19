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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/pixman")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman
)

add_definitions(
  -DUSE_SSE2
  -DUSE_SSSE3
  -DPACKAGE=pixman-1
  -DPACKAGE_VERSION=""
  -DPACKAGE_BUGREPORT="")

if(NOT CMAKE_CL_64)
  add_definitions(-DUSE_X86_MMX)
endif()

ignore_warnings()

set(pixman_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-access.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-access-accessors.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-bits-image.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-combine32.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-combine-float.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-conical-gradient.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-filter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-x86.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-mips.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-arm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-ppc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-edge.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-edge-accessors.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-fast-path.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-glyph.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-general.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-gradient-walker.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-image.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-implementation.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-linear-gradient.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-matrix.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-noop.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-radial-gradient.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-region16.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-region32.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-solid-fill.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-timer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-trap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-utils.c

  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-mmx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-sse2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-ssse3.c

  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-accessor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-combine32.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-compiler.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-edge-imp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-inlines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pixman/pixman-private.h
)

configure_file(
  miktex-pixman-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-pixman-version.h
)

list(APPEND pixman_sources
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-pixman-version.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
