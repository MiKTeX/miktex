## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/mkfntmap")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(mkfntmap_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  internal.h
  mkfntmap-version.h
  mkfntmap.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    mkfntmap.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mkfntmap.rc
  )
  list(APPEND mkfntmap_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mkfntmap.rc
    ${MIKTEX_COMPATIBILITY_MANIFEST}
  )
endif()

add_executable(mkfntmap ${mkfntmap_sources})

set_property(TARGET mkfntmap PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_FMT)
  target_link_libraries(mkfntmap MiKTeX::Imported::FMT)
else()
  target_link_libraries(mkfntmap ${fmt_dll_name})
endif()

if(USE_SYSTEM_LOG4CXX)
  target_link_libraries(mkfntmap MiKTeX::Imported::LOG4CXX)
else()
  target_link_libraries(mkfntmap ${log4cxx_dll_name})
endif()

target_link_libraries(mkfntmap
  ${app_dll_name}
  ${core_dll_name}
  miktex-popt-wrapper
)

install(TARGETS mkfntmap
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
