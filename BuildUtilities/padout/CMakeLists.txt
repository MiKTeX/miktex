## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2007-2018 Christian Schenk
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

set(padout_sources
  padout.cpp
)

add_executable(padout ${padout_sources})

set_property(TARGET padout PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS padout DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
