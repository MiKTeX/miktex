## CMakeLists.txt:
##
## Copyright (C) 2021 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/ptexenc")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(ptexenc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/ptexenc-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/jisx0208.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanjicnv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ptexenc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/unicode-jp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/unicode.c
)

set(headers
    kanjicnv.h
    ptexenc.h
    unicode-jp.h
    unicode.h
)

foreach(hdr ${headers})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ptexenc/${hdr}
        ${CMAKE_CURRENT_BINARY_DIR}/include/ptexenc/${hdr}
        COPYONLY
    )
    list(APPEND ptexenc_sources ${CMAKE_CURRENT_BINARY_DIR}/include/ptexenc/${hdr})
endforeach()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/c-auto.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/ptexenc/c-auto.h
)

configure_file(
    ptexenc-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/ptexenc-version.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
    add_subdirectory(shared)
endif()

add_subdirectory(static)
