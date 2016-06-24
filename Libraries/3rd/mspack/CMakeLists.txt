## CMakeLists.txt					-*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/mspack")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack
)

add_definitions(
  -DHAVE_CONFIG_H
)

ignore_warnings()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(mspack_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/cab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/cabc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/cabd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/chm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/chmc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/chmd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/crc32.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/crc32.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/des.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/hlp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/hlpc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/hlpd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/kwaj.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/kwajc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/kwajd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/litc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/litd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzss.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzssd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzxc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/lzxd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mspack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mszip.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mszipc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/mszipd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/oab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/oabc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/oabd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/qtm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/qtmd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/readbits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/readhuff.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/sha.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/system.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/system.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/szdd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/szddc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mspack/szddd.c

  ${CMAKE_CURRENT_SOURCE_DIR}/mspack-version.h

  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
