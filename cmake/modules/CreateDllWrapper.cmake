## CreateDllWrapper.cmake
##
## Copyright (C) 2006-2020 Christian Schenk
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

macro(create_dll_wrapper _name _libname)
  if(${ARGC} GREATER 2)
    set(_funcname ${ARGV2})
  else()
    string(TOLOWER "${_name}" _funcname)
  endif()
  configure_file(
    ${MIKTEX_ALIAS_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
    COPYONLY
  )
  set(_sources ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp)
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND _sources
      ${MIKTEX_COMMON_MANIFEST}
    )
  endif()
  add_executable(${_name} ${_sources})
  if(MIKTEX_CURRENT_FOLDER)
    set_property(TARGET ${_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  endif()
  if(MIKTEX_NATIVE_WINDOWS)
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
      COMPILE_FLAGS "-DFUNC=${_funcname} -D_UNICODE"
    )
  else()
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
      COMPILE_FLAGS "-DFUNC=${_funcname}"
    )
  endif()
  target_link_libraries(${_name} ${core_dll_name} ${_libname})
  install(TARGETS ${_name} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endmacro()
