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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/Maker")

if(LINK_EVERYTHING_STATICALLY)
  use_static_crt()
endif()

if(MSVC)
  add_definitions(
    -D_UNICODE -DUNICODE
  )
endif()  

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(LINK_EVERYTHING_STATICALLY)
  set(makex_target_name ${makex_lib_name})
else()
  set(makex_target_name ${makex_dll_name})
endif()

set(${makex_target_name}_sources
  common.cpp
  internal.h
  makebase.cpp
  makefmt.cpp
  makemf.cpp
  makepk.cpp
  maketfm.cpp
  makex-version.h
  makex.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    makex.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/makex.rc
  )
  list(APPEND ${makex_target_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/makex.rc
  )
endif()

if(LINK_EVERYTHING_STATICALLY)
  add_library(${makex_target_name} STATIC ${${makex_target_name}_sources})
  set_property(TARGET ${makex_target_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
else()
  add_library(${makex_target_name} SHARED ${${makex_target_name}_sources})
  set_property(TARGET ${makex_target_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
  target_link_libraries(${makex_target_name}
    PRIVATE
      ${app_dll_name}
      ${core_dll_name}
      ${log4cxx_dll_name}
      ${texmf_dll_name}
  )
  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${makex_target_name}
      PRIVATE
        ${getopt_dll_name}
    )
  endif()
  rebase(${makex_target_name})
endif()

set(exes
  makebase
  makefmt
  makemf
  makepk
  maketfm
)

if(NOT LINK_EVERYTHING_STATICALLY OR INSTALL_STATIC_LIBRARIES)
  install(TARGETS ${makex_target_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endif()

foreach(t ${exes})
  create_dll_wrapper(${MIKTEX_PREFIX}${t} ${makex_target_name} ${t})
  install(TARGETS ${MIKTEX_PREFIX}${t}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endforeach()
