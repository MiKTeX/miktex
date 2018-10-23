## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2018 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/lzma2")

include_directories(BEFORE
  
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}

  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/api
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/rangecoder
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/common
)

if(MIKTEX_NATIVE_WINDOWS)
  include_directories(BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/source/windows/vs2017
  )
endif()

add_definitions(
  -DTUKLIB_SYMBOL_PREFIX=lzma_
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
  lzma.h
  lzma/base.h
  lzma/bcj.h
  lzma/block.h
  lzma/check.h
  lzma/container.h
  lzma/delta.h
  lzma/filter.h
  lzma/hardware.h
  lzma/index.h
  lzma/index_hash.h
  lzma/lzma12.h
  lzma/stream_flags.h
  lzma/version.h
  lzma/vli.h
)

foreach(h ${public_headers})
  configure_file(
    source/src/liblzma/api/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

include(sources.cmake)

ignore_warnings()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
