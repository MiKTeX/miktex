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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/png")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(interface_definitions
  -DHAVE_LIBPNG=1
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

add_definitions(
  -DPNG_CONFIGURE_LIBPNG
  -DPNG_NO_CONSOLE_IO
)

ignore_warnings()

configure_file(
  source/scripts/pnglibconf.h.prebuilt
  ${CMAKE_CURRENT_BINARY_DIR}/include/pnglibconf.h
)

set(png_sources
  ${CMAKE_CURRENT_BINARY_DIR}/include/pnglibconf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/png.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/png.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngconf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngerror.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngget.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngmem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngpread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngpriv.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngrio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngrtran.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngrutil.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngtrans.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwrite.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwtran.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/pngwutil.c

  ${CMAKE_CURRENT_SOURCE_DIR}/libpng-version.h
)

set(${png_dll_name}_sources ${sources})

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
