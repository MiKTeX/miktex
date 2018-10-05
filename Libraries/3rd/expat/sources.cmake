## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2001-2018 Christian Schenk
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

set(expat_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/ascii.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/asciitab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/expat.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/expat_external.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/iasciitab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/latin1tab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/nametab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/siphash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/utf8tab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmlparse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmlrole.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmlrole.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmltok.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmltok.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmltok_impl.h

  ${CMAKE_CURRENT_SOURCE_DIR}/expat-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND expat_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/loadlibrary.c
  )
endif()

configure_file(
  source/expat_config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/expat_config.h
)

list(APPEND expat_sources
  ${CMAKE_CURRENT_BINARY_DIR}/expat_config.h
)
