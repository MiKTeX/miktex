## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/fribidi")

include_directories(BEFORE
  
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}

  ${CMAKE_CURRENT_BINARY_DIR}/include/fribidi
  
  ${CMAKE_CURRENT_SOURCE_DIR}/source/charset
  ${CMAKE_CURRENT_SOURCE_DIR}/source/lib
)

set(FRIBIDI_VERSION "0.19.7")
set(FRIBIDI_MAJOR_VERSION 0)
set(FRIBIDI_MINOR_VERSION 19)
set(FRIBIDI_MICRO_VERSION 7)
set(FRIBIDI_INTERFACE_VERSION 3)
set(FRIBIDI_CHARSETS 1)
set(FRIBIDI_USE_GLIB 0)
set(SIZEOF_INT 4)
set(HAVE_STRINGIZE 1)

configure_file(
  source/lib/fribidi-config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/fribidi/fribidi-config.h
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
  lib/fribidi-arabic.h
  lib/fribidi-begindecls.h
  lib/fribidi-bidi.h
  lib/fribidi-bidi-types.h
  lib/fribidi-bidi-types-list.h
  lib/fribidi-common.h
  lib/fribidi-deprecated.h
  lib/fribidi-enddecls.h
  lib/fribidi-flags.h
  lib/fribidi-joining.h
  lib/fribidi-joining-types.h
  lib/fribidi-joining-types-list.h
  lib/fribidi-mirroring.h
  lib/fribidi-shape.h
  lib/fribidi-types.h
  lib/fribidi-unicode.h
  lib/fribidi-unicode-version.h
  lib/fribidi.h

  charset/fribidi-char-sets.h
  charset/fribidi-char-sets-list.h
)
      
foreach(hdr ${public_headers})
  get_filename_component(h ${hdr} NAME)
  configure_file(
    source/${hdr}
    ${CMAKE_CURRENT_BINARY_DIR}/include/fribidi/${h}
    COPYONLY
  )
endforeach()

include(sources.cmake)

ignore_warnings()

#if(NOT LINK_EVERYTHING_STATICALLY)
#  add_subdirectory(shared)
#endif()

add_subdirectory(static)
