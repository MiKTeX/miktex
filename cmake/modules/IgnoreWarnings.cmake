## IgnoreWarnings.cmake
##
## Copyright (C) 2015-2018 Christian Schenk
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

# DEPRECATED
macro(ignore_warnings)
  if(MSVC)
    add_definitions(
      -D_CRT_SECURE_NO_WARNINGS
      -D_CRT_NONSTDC_NO_WARNINGS
    )
  endif()
endmacro()

function(prevent_all_warnings target)
  target_compile_options(${target}
    PRIVATE
      $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:-w>
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-w>
  )
endfunction()
