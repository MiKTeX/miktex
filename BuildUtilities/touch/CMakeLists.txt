## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2018 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

use_static_crt()

set(touch_sources
  touch-version.h
  touch.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND touch_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    touch.rc
  )
endif()

add_executable(${MIKTEX_PREFIX}touch ${touch_sources})

set_property(TARGET ${MIKTEX_PREFIX}touch PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}touch
  ${core_lib_name}
  miktex-popt-wrapper-static
)
