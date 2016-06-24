## CMakeLists.txt					-*- CMake -*-
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

hide_symbols()

set(${texmf_dll_name}_sources ${texmf_sources})

if(MIKTEX_NATIVE_WINDOWS AND MSVC AND NOT CMAKE_CL_64)
  list(APPEND ${texmf_dll_name}_sources
    ../mfmpi386asm.cpp
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../texmf.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/texmf.rc
  )
  list(APPEND ${texmf_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/texmf.rc
  )
endif()

add_library(${texmf_dll_name} SHARED ${${texmf_dll_name}_sources})

set_property(TARGET ${texmf_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${texmf_dll_name} gen-defaults)

set_source_files_properties(
  ${generated_texmf_sources}
  PROPERTIES
    GENERATED TRUE
)

set_target_properties(${texmf_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_compile_definitions(${texmf_dll_name}
  PUBLIC
    -DMIKTEX_TEXMF_SHARED
)

target_include_directories(${texmf_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${texmf_dll_name}
  PRIVATE
    ${app_dll_name}
    ${bzip2_dll_name}
    ${core_dll_name}
    ${zlib_dll_name}
    miktex-popt-wrapper
)

rebase(${texmf_dll_name})

install(TARGETS ${texmf_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public/C4P FILES ${public_headers_c4p})
source_group(Public/TEXMF FILES ${public_headers_texmf})
