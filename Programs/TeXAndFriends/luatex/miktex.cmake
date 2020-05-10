## miktex.cmake
##
## Copyright (C) 2010-2020 Christian Schenk
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

set(miktex_sources
  miktex/luatex.h
  miktex/miktex.cpp
)

add_library(luatex-miktex-objects OBJECT ${miktex_sources})

set_property(TARGET luatex-miktex-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(luatex-miktex-objects
  PUBLIC
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
)

if(USE_SYSTEM_FMT)
  target_link_libraries(luatex-miktex-objects PUBLIC MiKTeX::Imported::FMT)
else()
  target_link_libraries(luatex-miktex-objects PUBLIC ${fmt_dll_name})
endif()
