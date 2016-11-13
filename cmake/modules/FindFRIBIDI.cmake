## FindFRIBIDI.cmake
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

find_path(FRIBIDI_INCLUDE_DIR
  NAMES
    fribidi.h
  PATH_SUFFIXES
    fribidi
)

find_library(FRIBIDI_LIBRARY
  NAMES
    fribidi
)

find_package_handle_standard_args(FRIBIDI DEFAULT_MSG FRIBIDI_LIBRARY FRIBIDI_INCLUDE_DIR)

if(FRIBIDI_FOUND)
  set(FRIBIDI_INCLUDE_DIRS ${FRIBIDI_INCLUDE_DIR})
  set(FRIBIDI_LIBRARIES ${FRIBIDI_LIBRARY})
else()
  set(FRIBIDI_INCLUDE_DIRS)
  set(FRIBIDI_LIBRARIES)
endif()

mark_as_advanced(FRIBIDI_LIBRARY FRIBIDI_INCLUDE_DIR)
