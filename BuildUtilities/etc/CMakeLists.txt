## CMakeLists.txt					-*- CMake -*-
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

add_executable(bintoc bintoc.cpp)

set_property(TARGET bintoc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
  add_executable(headerguard headerguard.cpp)
  set_property(TARGET headerguard PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(headerguard Rpcrt4)
endif()

add_executable(j2000 j2000.cpp)

set_property(TARGET j2000 PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_executable(web-n web-n.cpp)

set_property(TARGET web-n PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

configure_file(
  hhc.cmd.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/hhc.cmd
)
