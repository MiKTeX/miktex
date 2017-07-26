## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/PoptWrapper")

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(miktex-popt-wrapper INTERFACE)

target_sources(miktex-popt-wrapper
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Wrappers/PoptWrapper
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Wrappers/PoptWrapper.h
)

target_include_directories(miktex-popt-wrapper
  INTERFACE
    ${public_include_directories}
)

if(USE_SYSTEM_POPT)
  target_link_libraries(miktex-popt-wrapper INTERFACE MiKTeX::Imported::POPT)
else()
  target_link_libraries(miktex-popt-wrapper INTERFACE ${popt_dll_name})
endif()

add_library(miktex-popt-wrapper-static INTERFACE)

target_include_directories(miktex-popt-wrapper-static
  INTERFACE
    ${public_include_directories}
)

if(USE_SYSTEM_POPT)
  target_link_libraries(miktex-popt-wrapper-static INTERFACE MiKTeX::Imported::POPT)
else()
  target_link_libraries(miktex-popt-wrapper-static INTERFACE ${popt_lib_name})
endif()
