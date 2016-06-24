## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/setup")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService.h
)

set(setup_sources
  ${public_headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/LogFile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SetupService.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/setup-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND setup_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winSetupService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winSetupService.h
  )
else()
  list(APPEND setup_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/unx/unxSetupService.cpp
  )
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
