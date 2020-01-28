## CMakeLists.txt                                       -*- CMake -*-
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/glm")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

set(glm_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/glm/detail/glm.cpp
)

configure_file(
  glm-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/glm-version.h
)

list(APPEND glm_sources
  ${CMAKE_CURRENT_BINARY_DIR}/glm-version.h
)

#if(NOT LINK_EVERYTHING_STATICALLY)
#  add_subdirectory(shared)
#endif()

add_subdirectory(static)
