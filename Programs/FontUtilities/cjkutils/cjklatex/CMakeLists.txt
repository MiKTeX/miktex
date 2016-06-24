## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/cjklatex")

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(${cjklatex_dll_name}_sources
  cjklatex-version.h
  cjklatex.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    cjklatex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/cjklatex.rc
  )
  list(APPEND ${cjklatex_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/cjklatex.rc
  )
endif()

add_library(${cjklatex_dll_name} SHARED ${${cjklatex_dll_name}_sources})

set_property(TARGET ${cjklatex_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${cjklatex_dll_name}
  PRIVATE
    ${app_dll_name}
    ${core_dll_name}
    ${mpm_dll_name}
    ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${cjklatex_dll_name}
    PRIVATE
      ${getopt_dll_name}
  )
endif()

set(exes
  bg5pluslatex
  bg5pluspdflatex
  bg5latex
  bg5pdflatex
  bg5platex
  bg5ppdflatex
  cef5latex
  cef5pdflatex
  ceflatex
  cefpdflatex
  cefslatex
  cefspdflatex
  cjklatex
  gbklatex
  gbkpdflatex
  sjislatex
  sjispdflatex
)

foreach(t ${exes})
  create_dll_wrapper(${t} ${cjklatex_dll_name} cjklatexmain)
  set_property(TARGET ${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
endforeach()

install(TARGETS ${cjklatex_dll_name} ${exes} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
