## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2008-2018 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/uriparser")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

add_definitions(
  -DURI_PASS_ANSI
  -DURI_NO_UNICODE
)

set(uriparser_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/uriparser-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriCommon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriCommon.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriCompare.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriEscape.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriFile.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriIp4.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriIp4Base.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriIp4Base.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriNormalize.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriNormalizeBase.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriNormalizeBase.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriParse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriParseBase.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriParseBase.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriQuery.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriRecompose.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriResolve.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/UriShorten.c
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
