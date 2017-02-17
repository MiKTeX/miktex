## FindHARFBUZZ_ICU.cmake
##
## Copyright (C) 2017 Christian Schenk
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

find_path(HARFBUZZ_INCLUDE_DIR
  NAMES
    hb.h
  PATH_SUFFIXES
    harfbuzz
)

find_library(HARFBUZZ_LIBRARY
  NAMES
    harfbuzz
)

find_package_handle_standard_args(HARFBUZZ DEFAULT_MSG HARFBUZZ_LIBRARY HARFBUZZ_INCLUDE_DIR)

if(HARFBUZZ_FOUND)
  set(HARFBUZZ_INCLUDE_DIRS ${HARFBUZZ_INCLUDE_DIR})
  set(HARFBUZZ_LIBRARIES ${HARFBUZZ_LIBRARY})
else()
  set(HARFBUZZ_INCLUDE_DIRS)
  set(HARFBUZZ_LIBRARIES)
endif()

mark_as_advanced(HARFBUZZ_LIBRARY HARFBUZZ_INCLUDE_DIR)
