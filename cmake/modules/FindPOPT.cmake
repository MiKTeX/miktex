## FindPOPT.cmake
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

find_path(POPT_INCLUDE_DIR
  NAMES
    popt.h
)

find_library(POPT_LIBRARY
  NAMES
    popt
)

find_package_handle_standard_args(POPT DEFAULT_MSG POPT_LIBRARY POPT_INCLUDE_DIR)

if(POPT_FOUND)
  set(POPT_INCLUDE_DIRS ${POPT_INCLUDE_DIR})
  set(POPT_LIBRARIES ${POPT_LIBRARY})
else()
  set(POPT_INCLUDE_DIRS)
  set(POPT_LIBRARIES)
endif()

mark_as_advanced(POPT_LIBRARY POPT_INCLUDE_DIR)
