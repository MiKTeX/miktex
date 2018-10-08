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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/extractor")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Extractor/vi/Version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Extractor/Extractor
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Extractor/Extractor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Extractor/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Extractor/vi/Version
)

configure_file(
  extractor-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/extractor-version.h
)

configure_file(
  include/miktex/Extractor/vi/Version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Extractor/vi/Version.h
)

set(extractor_sources
  ${CMAKE_CURRENT_BINARY_DIR}/extractor-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/CabExtractor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CabExtractor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Extractor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TarBzip2Extractor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TarBzip2Extractor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TarExtractor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TarExtractor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TarLzmaExtractor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TarLzmaExtractor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/vi/Runtime.cpp
  ${public_headers}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND exctractor_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winCabExtractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winCabExtractor.h
  )
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
