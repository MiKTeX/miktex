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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/texmf")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

configure_file(
  include/miktex/C4P/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}//include/miktex/C4P/config.h
)

configure_file(
  include/miktex/TeXAndFriends/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/TeXAndFriends/config.h
)

set(generated_texmf_sources
  ${CMAKE_BINARY_DIR}/include/miktex/mfapp.defaults.h
  ${CMAKE_BINARY_DIR}/include/miktex/texapp.defaults.h
  ${CMAKE_BINARY_DIR}/include/miktex/texmfapp.defaults.h
)

set(public_headers_c4p
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/C4P/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/C4P/C4P
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/C4P/C4P.h
)

set(public_headers_texmf
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/TeXAndFriends/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/ETeXApp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/ETeXApp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/ETeXApp.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/MetafontApp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/MetafontApp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/MetafontApp.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/Prototypes
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/Prototypes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXApp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXApp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXApp.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXMFApp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXMFApp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/TeXMFApp.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebApp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebApp.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebApp.inl
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebAppInputLine
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebAppInputLine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/TeXAndFriends/WebAppInputLine.inl
)

set(public_headers
  ${public_headers_c4p}
  ${public_headers_texmf}
)

set(texmf_sources
  ${public_headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/c4plib.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/c4pstart.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/etexapp.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/inputline.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/mfapp.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/texapp.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/texmf-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/texmfapp.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/texmflib.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/webapp.cpp
  ${generated_texmf_sources}
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      include/miktex/TeXAndFriends/ETeXApp
      include/miktex/TeXAndFriends/ETeXApp.h
      include/miktex/TeXAndFriends/ETeXApp.inl
      include/miktex/TeXAndFriends/MetafontApp
      include/miktex/TeXAndFriends/MetafontApp.h
      include/miktex/TeXAndFriends/MetafontApp.inl
      include/miktex/TeXAndFriends/Prototypes
      include/miktex/TeXAndFriends/Prototypes.h
      include/miktex/TeXAndFriends/TeXApp
      include/miktex/TeXAndFriends/TeXApp.h
      include/miktex/TeXAndFriends/TeXApp.inl
      include/miktex/TeXAndFriends/TeXMFApp
      include/miktex/TeXAndFriends/TeXMFApp.h
      include/miktex/TeXAndFriends/TeXMFApp.inl
      include/miktex/TeXAndFriends/WebApp
      include/miktex/TeXAndFriends/WebApp.h
      include/miktex/TeXAndFriends/WebApp.inl
      include/miktex/TeXAndFriends/WebAppInputLine
      include/miktex/TeXAndFriends/WebAppInputLine.h
      include/miktex/TeXAndFriends/WebAppInputLine.inl
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/TeXAndFriends
  )
endif()
