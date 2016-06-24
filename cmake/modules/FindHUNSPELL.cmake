## FindHUNSPELL.cmake
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

find_path(HUNSPELL_INCLUDE_DIR
  NAMES
    hunspell.h
  PATH_SUFFIXES
    hunspell
)

find_library(HUNSPELL_LIBRARY
  NAMES
    hunspell
    hunspell-1.3
)

find_package_handle_standard_args(HUNSPELL DEFAULT_MSG HUNSPELL_LIBRARY HUNSPELL_INCLUDE_DIR)

if(HUNSPELL_FOUND)
  set(HUNSPELL_INCLUDE_DIRS ${HUNSPELL_INCLUDE_DIR})
  set(HUNSPELL_LIBRARIES ${HUNSPELL_LIBRARY})
else()
  set(HUNSPELL_INCLUDE_DIRS)
  set(HUNSPELL_LIBRARIES)
endif()

mark_as_advanced(HUNSPELL_LIBRARY HUNSPELL_INCLUDE_DIR)
