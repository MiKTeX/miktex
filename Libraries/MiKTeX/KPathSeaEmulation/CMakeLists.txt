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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/kpsemu")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/KPSE/Emulation
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/KPSE/Emulation.h
)

set(kpathsea_includes
  absolute.h
  c-auto.h
  c-ctype.h
  c-dir.h
  c-errno.h
  c-fopen.h
  c-limits.h
  c-memstr.h
  c-minmax.h
  c-pathch.h
  c-pathmx.h
  c-proto.h
  c-stat.h
  c-std.h
  c-unistd.h
  c-vararg.h
  concatn.h
  config.h
  debug.h
  dirent.h
  expand.h
  getopt.h
  kpathsea.h
  lib.h
  line.h
  magstep.h
  paths.h
  pathsearch.h
  proginit.h
  progname.h
  readable.h
  recorder.h
  str-list.h
  systypes.h
  tex-file.h
  tex-glyph.h
  tex-hush.h
  tex-make.h
  types.h
  variable.h
  version.h
)

foreach(h ${kpathsea_includes})
  configure_file(
    include/kpathsea/kpathsea.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/kpathsea/${h}
  )
endforeach()

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      include/miktex/KPSE/Emulation
      include/miktex/KPSE/Emulation.h
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/KPSE
  )
endif()

set(kpsemu_sources
  ${public_headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kpsemu.cpp
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
