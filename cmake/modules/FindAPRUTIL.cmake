## FindAPRUTIL.cmake
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

find_path(APRUTIL_INCLUDE_DIR
  NAMES
    apr_anylock.h
  PATH_SUFFIXES
    apr-1
    apr-1.0
)

find_library(APRUTIL_LIBRARY
  NAMES
    aprutil-1
)

find_package_handle_standard_args(APRUTIL DEFAULT_MSG APRUTIL_LIBRARY APRUTIL_INCLUDE_DIR)

if(APRUTIL_FOUND)
  set(APRUTIL_INCLUDE_DIRS ${APRUTIL_INCLUDE_DIR})
  set(APRUTIL_LIBRARIES ${APRUTIL_LIBRARY})
else()
  set(APRUTIL_INCLUDE_DIRS)
  set(APRUTIL_LIBRARIES)
endif()

mark_as_advanced(APRUTIL_LIBRARY APRUTIL_INCLUDE_DIR)
