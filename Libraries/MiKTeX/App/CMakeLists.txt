## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2019 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/app")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(headers_no_ext
  miktex/App/Application
  miktex/App/vi/Version
)

foreach(h ${headers_no_ext})
  set(_path ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
  list(APPEND public_headers_no_ext ${_path})
  if (NOT EXISTS ${_path})
    file(WRITE ${_path}
      "#pragma once\n#include <${h}.h>\n"
    )
  endif()
endforeach()

set(public_headers
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/App/vi/Version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/App/Application.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/App/config.h
  ${public_headers_no_ext}
)

configure_file(
  app-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/app-version.h
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  include/miktex/App/vi/Version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/App/vi/Version.h
)

set(app_sources
  ${CMAKE_CURRENT_BINARY_DIR}/app-version.h
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/vi/Runtime.cpp
  ${public_headers}
)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      ${public_headers}
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/App
  )
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
