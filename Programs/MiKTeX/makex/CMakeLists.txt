## CMakeLists.txt                                       -*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/MakeUtilities")

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

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(common_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
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
      ${MIKTEX_COMPATIBILITY_MANIFEST}
      ${t}.rc
    )
  endif()
  string(TOUPPER "${t}" _prog_name_u)
  add_executable(${MIKTEX_PROG_NAME_${_prog_name_u}} ${${t}_sources})
  set_property(TARGET ${MIKTEX_PROG_NAME_${_prog_name_u}} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  if(USE_SYSTEM_FMT)
    target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} MiKTeX::Imported::FMT)
  else()
    target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} ${fmt_dll_name})
  endif()
  if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} MiKTeX::Imported::LOG4CXX)
  else()
    target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}} ${log4cxx_dll_name})
  endif()
  target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}}
    ${app_dll_name}
    ${core_dll_name}
    ${texmf_dll_name}
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PROG_NAME_${_prog_name_u}}
      ${getopt_dll_name}
    )
  endif()
  install(TARGETS ${MIKTEX_PROG_NAME_${_prog_name_u}}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endforeach()
