## FindPOTRACE.cmake
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

find_path(POTRACE_INCLUDE_DIR
  NAMES
    potracelib.h
)

find_library(POTRACE_LIBRARY
  NAMES
    potrace
)

find_package_handle_standard_args(POTRACE DEFAULT_MSG POTRACE_LIBRARY POTRACE_INCLUDE_DIR)

if(POTRACE_FOUND)
  set(POTRACE_INCLUDE_DIRS ${POTRACE_INCLUDE_DIR})
  set(POTRACE_LIBRARIES ${POTRACE_LIBRARY})
else()
  set(POTRACE_INCLUDE_DIRS)
  set(POTRACE_LIBRARIES)
endif()

mark_as_advanced(POTRACE_LIBRARY POTRACE_INCLUDE_DIR)
