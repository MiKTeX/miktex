## CreateExeAlias.cmake (obsolete)
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

macro(create_exe_alias _name _exe)
  configure_file(
    ${MIKTEX_ALIAS_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/${_name}alias.cpp
    COPYONLY
  )
  add_executable (${_name} ${CMAKE_CURRENT_BINARY_DIR}/${_name}alias.cpp)
  if(MIKTEX_CURRENT_FOLDER)
    set_property(TARGET ${_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  endif()
  set(_pre_argv "")
  foreach (_a ${ARGN})
    if(_pre_argv STREQUAL "")
      set(_pre_argv "\\\"${_a}\\\"")
    else()
      set(_pre_argv "${_pre_argv},\\\"${_a}\\\"")
    endif()
  endforeach()
  if(_pre_argv STREQUAL "")
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${_name}alias.cpp
      COMPILE_FLAGS "-DREAL_NAME=\\\"${_exe}\\\""
    )
  else()
    if(MSVC)
      set_source_files_properties(
	${CMAKE_CURRENT_BINARY_DIR}/${_name}alias.cpp
	COMPILE_FLAGS "-DREAL_NAME=\\\"${_exe}\\\" -DPRE_ARGV=${_pre_argv} -D_UNICODE"
      )
    else()
      set_source_files_properties(
	${CMAKE_CURRENT_BINARY_DIR}/${_name}alias.cpp
	COMPILE_FLAGS "-DREAL_NAME=\\\"${_exe}\\\" -DPRE_ARGV=${_pre_argv}"
      )
    endif()
  endif()
  target_link_libraries(${_name} ${core_dll_name})
  merge_trustinfo_manifest(${_name} asInvoker)
  install(TARGETS ${_name} DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
endmacro()
