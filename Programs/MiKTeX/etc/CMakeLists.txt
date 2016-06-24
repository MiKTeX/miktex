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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/etc")

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

set(executables
  runjava
  runperl
  runpython
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND executables
    runbat
  )
endif()

foreach(t ${executables})
  add_executable(${t} ${t}.cpp)
  set_property(TARGET ${t} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(${t}
    ${app_dll_name}
    ${core_dll_name}
  )
  merge_trustinfo_manifest(${t} asInvoker)
  merge_compatibility_manifest(${t})
  install(TARGETS ${t} DESTINATION "${MIKTEX_INTERNAL_BINARY_DIR}")
endforeach()
