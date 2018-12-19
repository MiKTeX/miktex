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

set(folder "${MIKTEX_IDE_GRAPHICS_UTILITIES_FOLDER}/PSUtils")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
  -Dmain=Main
)

set(psutils_lib_sources
  miktex.cpp
  source/psspec.c
  source/psutil.c
  verror.h
)

add_library(psutils STATIC ${psutils_lib_sources})

set_property(TARGET psutils PROPERTY FOLDER ${folder})

target_link_libraries(psutils
  PUBLIC
    ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(psutils
    PUBLIC
      ${getopt_dll_name}
  )
endif()

set(executables
  epsffit
  psbook
  psnup
  psresize
  psselect
  pstops
)

foreach(x ${executables})
  add_executable(${MIKTEX_PREFIX}${x}
    ${MIKTEX_COMPATIBILITY_MANIFEST}
    ${MIKTEX_LIBRARY_WRAPPER}
    source/${x}.c
  )
  target_link_libraries(${MIKTEX_PREFIX}${x}
    ${app_dll_name}
    ${core_dll_name}
    psutils
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}${x}
      ${unxemu_dll_name}
    )
  endif()
  set_property(TARGET ${MIKTEX_PREFIX}${x} PROPERTY FOLDER ${folder})
  install(TARGETS ${MIKTEX_PREFIX}${x} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endforeach()

if(WITH_SCRIPTS)
  install(
    FILES
      source/extractres
      source/psjoin
    DESTINATION
      ${MIKTEX_DIST_DIR}/scripts/psutils
  )
endif()
