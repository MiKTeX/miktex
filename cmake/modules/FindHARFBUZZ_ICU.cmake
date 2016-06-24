## FindHARFBUZZ_ICU.cmake
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

find_path(HARFBUZZ_ICU_INCLUDE_DIR
  NAMES
    hb-icu.h
  PATH_SUFFIXES
    harfbuzz
)

find_library(HARFBUZZ_ICU_LIBRARY
  NAMES
    harfbuzz-icu
)

find_package_handle_standard_args(HARFBUZZ_ICU DEFAULT_MSG HARFBUZZ_ICU_LIBRARY HARFBUZZ_ICU_INCLUDE_DIR)

if(HARFBUZZ_ICU_FOUND)
  set(HARFBUZZ_ICU_INCLUDE_DIRS ${HARFBUZZ_ICU_INCLUDE_DIR})
  set(HARFBUZZ_ICU_LIBRARIES ${HARFBUZZ_ICU_LIBRARY})
else()
  set(HARFBUZZ_ICU_INCLUDE_DIRS)
  set(HARFBUZZ_ICU_LIBRARIES)
endif()

mark_as_advanced(HARFBUZZ_ICU_LIBRARY HARFBUZZ_ICU_INCLUDE_DIR)
