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

include(component.cmake)
include(sources.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/harfbuzz")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/source/src
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

add_definitions(
  -DHAVE_CONFIG_H
  -DHAVE_FALLBACK
  -DHB_NO_MT
  -DHB_NO_UNICODE_FUNCS
)

set(HAVE_ROUND 1)

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

list(APPEND harfbuzz_sources
  ${HB_BASE_sources}
  ${HB_BASE_RAGEL_GENERATED_sources}

  ${HB_FALLBACK_sources}
  ${HB_OT_sources}
  ${HB_OT_RAGEL_GENERATED_sources}

  ${HB_BASE_headers}
  ${HB_OT_headers}

  ${HB_GRAPHITE2_sources}
  ${HB_GRAPHITE2_headers}

  ${HB_ICU_sources}
  ${HB_ICU_headers}
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
