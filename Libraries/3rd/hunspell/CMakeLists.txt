## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2009-2020 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/hunspell")

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hunspell
)

add_definitions(-DBUILDING_LIBHUNSPELL)

set(sources
  source/src/hunspell/affentry.cxx
  source/src/hunspell/affentry.hxx
  source/src/hunspell/affixmgr.cxx
  source/src/hunspell/affixmgr.hxx
  source/src/hunspell/atypes.hxx
  source/src/hunspell/baseaffix.hxx
  source/src/hunspell/csutil.cxx
  source/src/hunspell/csutil.hxx
  source/src/hunspell/filemgr.cxx
  source/src/hunspell/filemgr.hxx
  source/src/hunspell/hashmgr.cxx
  source/src/hunspell/hashmgr.hxx
  source/src/hunspell/htypes.hxx
  source/src/hunspell/hunspell.cxx
  source/src/hunspell/hunspell.h
  source/src/hunspell/hunspell.hxx
  source/src/hunspell/hunvisapi.h
  source/src/hunspell/hunzip.cxx
  source/src/hunspell/hunzip.hxx
  source/src/hunspell/langnum.hxx
  source/src/hunspell/phonet.cxx
  source/src/hunspell/phonet.hxx
  source/src/hunspell/replist.cxx
  source/src/hunspell/replist.hxx
  source/src/hunspell/suggestmgr.cxx
  source/src/hunspell/suggestmgr.hxx
  source/src/hunspell/w_char.hxx
)

configure_file(
  hunspell-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/hunspell-version.h
)

set(${hunspell_dll_name}_sources
  ${sources}
  hunspell-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    hunspell.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/hunspell.rc
  )
  set(${hunspell_dll_name}_sources ${${hunspell_dll_name}_sources}
    ${CMAKE_CURRENT_BINARY_DIR}/hunspell.rc
  )
endif()

add_library(${hunspell_dll_name} SHARED ${${hunspell_dll_name}_sources})

set_property(TARGET ${hunspell_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${hunspell_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${hunspell_dll_name}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/hunspell
)

target_link_libraries(${hunspell_dll_name}
  PRIVATE
    ${utf8wrap_dll_name}
)

install(TARGETS ${hunspell_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
