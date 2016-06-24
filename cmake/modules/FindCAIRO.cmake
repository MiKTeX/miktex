## FindCAIRO.cmake
##
## Copyright (C) 2016 Christian Schenk
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

find_path(CAIRO_INCLUDE_DIR
  NAMES
    cairo.h
  PATH_SUFFIXES
    cairo
)

find_library(CAIRO_LIBRARY
  NAMES
    cairo
)

find_package_handle_standard_args(CAIRO DEFAULT_MSG CAIRO_LIBRARY CAIRO_INCLUDE_DIR)

if(CAIRO_FOUND)
  set(CAIRO_INCLUDE_DIRS ${CAIRO_INCLUDE_DIR})
  set(CAIRO_LIBRARIES ${CAIRO_LIBRARY})
else()
  set(CAIRO_INCLUDE_DIRS)
  set(CAIRO_LIBRARIES)
endif()

mark_as_advanced(CAIRO_LIBRARY CAIRO_INCLUDE_DIR)
