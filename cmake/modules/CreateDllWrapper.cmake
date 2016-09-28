## CreateDllWrapper.cmake
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

macro(create_dll_wrapper _name _dll)
  if(${ARGC} GREATER 2)
    set(_dllmain ${ARGV2})
  else()
    string(TOLOWER "${_name}" _dllmain)
  endif()
  configure_file(
    ${MIKTEX_ALIAS_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
    COPYONLY
  )
  add_executable(${_name} ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp)
  if(MIKTEX_CURRENT_FOLDER)
    set_property(TARGET ${_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  endif()
  if(MSVC)
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
      COMPILE_FLAGS "-DDLLMAIN=${_dllmain} -D_UNICODE"
    )
  else()
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${_name}wrapper.cpp
      COMPILE_FLAGS "-DDLLMAIN=${_dllmain}"
    )
  endif()
  target_link_libraries(${_name} ${core_dll_name} ${_dll})
  merge_trustinfo_manifest(${_name} asInvoker)
  install(TARGETS ${_name} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endmacro()
