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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/Maker")

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()  

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(common_sources
  MakeUtility.h
)

set(exes
  makebase
  makefmt
  makemf
  makepk
  maketfm
)

foreach(t ${exes})
  set(${t}_sources ${t}.cpp ${t}-version.h ${common_sources})
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ${t}_sources
      ${t}.rc
    )
  endif()
  add_executable(${MIKTEX_PREFIX}${t} ${${t}_sources})
  set_property(TARGET ${MIKTEX_PREFIX}${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(${MIKTEX_PREFIX}${t} MiKTeX::Imported::LOG4CXX)
  else()
    target_link_libraries(${MIKTEX_PREFIX}${t} ${log4cxx_dll_name})
  endif()
  target_link_libraries(${MIKTEX_PREFIX}${t}
    ${app_dll_name}
    ${core_dll_name}
    ${texmf_dll_name}
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}${t}
      ${getopt_dll_name}
    )
  endif()
  install(TARGETS ${MIKTEX_PREFIX}${t}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endforeach()
