## CMakeLists.txt                                       -*- CMake -*-
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/dvi")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(makebitcounts_sources makebitcounts.cpp)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND makebitcounts_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()
  
add_executable(makebitcounts EXCLUDE_FROM_ALL ${makebitcounts_sources})

set_property(TARGET makebitcounts PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set(public_headers
  include/miktex/DVI/Dvi
  include/miktex/DVI/Dvi.h
)

set(${dvi_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/bitcounts.h
  ${public_headers}
  Dib.cpp
  Dib.h
  Dvi.cpp
  DviChar.cpp
  DviChar.h
  DviFont.cpp
  DviFont.h
  DviPage.cpp
  Ghostscript.cpp
  Ghostscript.h
  PkChar.cpp
  PkChar.h
  PkFont.cpp
  PkFont.h
  PostScript.cpp
  PostScript.h
  Tfm.cpp
  Tfm.h
  VFont.cpp
  VFont.h
  VfChar.cpp
  VfChar.h
  color.cpp
  dvi-version.h
  graphics.cpp
  hypertex.cpp
  inliners.h
  internal.h
  misc.cpp
  ps.cpp
  special.cpp
  src.cpp
  tpic.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    dvi.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/dvi.rc
  )
  list(APPEND ${dvi_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/dvi.rc
  )
endif()

set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/bitcounts.h
  PROPERTIES GENERATED TRUE
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/bitcounts.h
  COMMAND
    makebitcounts > bitcounts.h
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS
    makebitcounts
  VERBATIM
)

add_library(${dvi_dll_name} SHARED ${${dvi_dll_name}_sources})

set_property(TARGET ${dvi_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${dvi_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_include_directories(${dvi_dll_name}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_DVI_DIR}/include
)

target_link_libraries(${dvi_dll_name}
  PRIVATE
    ${core_dll_name}
  PUBLIC
    ${dib_dll_name}
)

if(USE_SYSTEM_FMT)
  target_link_libraries(${dvi_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${dvi_dll_name} PRIVATE ${fmt_dll_name})
endif()

set(dviscan_sources dviscan.cpp)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND dviscan_sources
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

add_executable(dviscan ${dviscan_sources})

set_property(TARGET dviscan PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(dviscan ${dvi_dll_name})

target_link_libraries(dviscan
  ${app_dll_name}
  ${core_dll_name}
  ${dvi_dll_name}
  ${getopt_dll_name}
  miktex-popt-wrapper
)

install(TARGETS ${dvi_dll_name} dviscan
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

install(
  FILES
    ${public_headers}
  DESTINATION
    ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/DVI
)

source_group(Public FILES ${public_headers})
