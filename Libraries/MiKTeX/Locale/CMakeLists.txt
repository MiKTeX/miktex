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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/loc")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Locale/Translator
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Locale/Translator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Locale/config.h
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

set(sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Translator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Locale/config.h
  ${public_headers}
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
