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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/mthelp")

if(LINK_EVERYTHING_STATICALLY)
  use_static_crt()
endif()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(MSVC)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()

if(LINK_EVERYTHING_STATICALLY)
  set(mthelp_target_name ${mthelp_lib_name})
else()
  set(mthelp_target_name ${mthelp_dll_name})
endif()

set(${mthelp_target_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/template.html.h
  mthelp.cpp
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/template.html.h
  COMMAND
    bintoc
    templateHtml
    < ${CMAKE_CURRENT_SOURCE_DIR}/template.html
    > ${CMAKE_CURRENT_BINARY_DIR}/template.html.h
  MAIN_DEPENDENCY
    template.html
  DEPENDS
    bintoc
  VERBATIM
)

if(LINK_EVERYTHING_STATICALLY)
  add_library(${mthelp_target_name} STATIC ${${mthelp_target_name}_sources})
  set_property(TARGET ${mthelp_target_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
else()
  if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ${mthelp_target_name}_sources
      mthelp.rc)
  endif()
  add_library(${mthelp_target_name} SHARED ${${mthelp_target_name}_sources})
  set_property(TARGET ${mthelp_target_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(${mthelp_target_name}
    PRIVATE
      ${app_dll_name}
      ${core_dll_name}
      ${mpm_dll_name}
      miktex-popt-wrapper
  )
  rebase(${mthelp_target_name})
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  install(TARGETS ${mthelp_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endif()

create_dll_wrapper(mthelp ${mthelp_target_name} mthelp)
