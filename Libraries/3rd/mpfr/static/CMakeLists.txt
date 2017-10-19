## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017 Christian Schenk
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

add_library(${mpfr_lib_name} STATIC ${mpfr_sources})

set_property(TARGET ${mpfr_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_GMP)
  target_link_libraries(${mpfr_lib_name} PUBLIC MiKTeX::Imported::GMP)
else()
  target_link_libraries(${mpfr_lib_name} PUBLIC ${gmp_lib_name})
endif()

target_include_directories(${mpfr_lib_name}
  PUBLIC
    ${public_include_directories}
)
