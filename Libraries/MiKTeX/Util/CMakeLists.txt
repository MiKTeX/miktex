## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/util")

set(util_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(util_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/CharBuffer
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/CharBuffer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/StringUtil
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/StringUtil.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/Tokenizer
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/Tokenizer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Util/inliners.h
)

set(util_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/StringUtil.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Tokenizer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/util-version.h
  ${public_headers}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND util_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winStringUtil.cpp
  )
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
