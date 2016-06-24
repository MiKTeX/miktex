## DefaultCharTypeIsUnsigned.cmake
##
## Copyright (C) 2006-2016 Christian Schenk
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

macro(default_char_type_is_unsigned)
  if(MSVC)
    foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
      set(CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}} /J")
      set(CMAKE_CXX_FLAGS${c} "${CMAKE_CXX_FLAGS${c}} /J")
    endforeach()
  else()
    foreach(c "" "_DEBUG" "_RELEASE" "_MINSIZEREL" "_RELWITHDEBINFO")
      set(CMAKE_C_FLAGS${c} "${CMAKE_C_FLAGS${c}} -funsigned-char")
      set(CMAKE_CXX_FLAGS${c} "${CMAKE_CXX_FLAGS${c}} -funsigned-char")
    endforeach()
  endif()
endmacro()
