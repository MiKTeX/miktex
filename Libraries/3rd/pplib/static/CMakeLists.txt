## CMakeLists.txt
##
## Copyright (C) 2020 Christian Schenk
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

add_library(${pplib_lib_name} STATIC ${pplib_sources})

set_property(TARGET ${pplib_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${pplib_lib_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${pplib_lib_name} PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${pplib_lib_name} PUBLIC ${zlib_dll_name})
endif()

target_link_libraries(${pplib_lib_name}
  PUBLIC
    ${core_dll_name}
    ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${pplib_lib_name}
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()
