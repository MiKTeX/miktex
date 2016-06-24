## FindLOG4CXX.cmake
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

find_path(LOG4CXX_INCLUDE_DIR
  NAMES
    log4cxx/log4cxx.h
)

find_library(LOG4CXX_LIBRARY
  NAMES
    log4cxx
)

find_package_handle_standard_args(LOG4CXX DEFAULT_MSG LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)

if(LOG4CXX_FOUND)
  set(LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIR})
  set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARY})
else()
  set(LOG4CXX_INCLUDE_DIRS)
  set(LOG4CXX_LIBRARIES)
endif()

mark_as_advanced(LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)
