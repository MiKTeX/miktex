## FindGRAPHITE2.cmake
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

find_path(GRAPHITE2_INCLUDE_DIR
  NAMES
    graphite2/Font.h
)

find_library(GRAPHITE2_LIBRARY
  NAMES
    graphite2
)

find_package_handle_standard_args(GRAPHITE2 DEFAULT_MSG GRAPHITE2_LIBRARY GRAPHITE2_INCLUDE_DIR)

if(GRAPHITE2_FOUND)
  set(GRAPHITE2_INCLUDE_DIRS ${GRAPHITE2_INCLUDE_DIR})
  set(GRAPHITE2_LIBRARIES ${GRAPHITE2_LIBRARY})
else()
  set(GRAPHITE2_INCLUDE_DIRS)
  set(GRAPHITE2_LIBRARIES)
endif()

mark_as_advanced(GRAPHITE2_LIBRARY GRAPHITE2_INCLUDE_DIR)
