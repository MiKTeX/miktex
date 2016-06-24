## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/w2cemu")

hide_symbols()

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_headers
  include/miktex/W2C/Emulation
  include/miktex/W2C/Emulation.h
  include/miktex/W2C/pre.h
)

configure_file(
  include/w2c/c-auto.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/w2c/c-auto.h
)

set(generated_texmf_sources
  ${CMAKE_BINARY_DIR}/include/miktex/texmfapp.defaults.h
)

set(${w2cemu_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/include/w2c/c-auto.h
  ${generated_texmf_sources}
  ${public_headers}
  StdAfx.h
  include/c-auto.h
  include/cpascal.h
  include/lib/lib.h
  include/w2c/config.h
  internal.h
  w2cemu-version.h
  w2cemu.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    w2cemu.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/w2cemu.rc
  )
  set(${w2cemu_dll_name}_sources ${${w2cemu_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/w2cemu.rc
  )
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_library(${w2cemu_dll_name} SHARED ${${w2cemu_dll_name}_sources})

  set_property(TARGET ${w2cemu_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

  add_dependencies(${w2cemu_dll_name} gen-defaults)

  set_source_files_properties(
    ${generated_texmf_sources}
    PROPERTIES
      GENERATED TRUE
  )

  set_target_properties(${w2cemu_dll_name}
    PROPERTIES
      VERSION "${MIKTEX_SERIES_STR}"
      SOVERSION "1"
  )

  target_include_directories(${w2cemu_dll_name}
    PUBLIC
      ${CMAKE_BINARY_DIR}/${MIKTEX_REL_W2CEMU_DIR}/include
      ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_W2CEMU_DIR}/include
  )

  target_link_libraries(${w2cemu_dll_name}
    PUBLIC
      ${core_dll_name}
      ${kpsemu_dll_name}
  )

  if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${w2cemu_dll_name}
      INTERFACE
        ${unxemu_dll_name}
    )
  endif()

  rebase(${w2cemu_dll_name})

  install(TARGETS ${w2cemu_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endif()

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      ${public_headers}
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/W2C
  )
endif()

source_group(Public FILES ${public_headers})
