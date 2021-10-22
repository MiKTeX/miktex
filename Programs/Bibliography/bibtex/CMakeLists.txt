## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2021 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BIBLIOGRAPHY_FOLDER}/BibTeX")

default_char_type_is_unsigned()

set(C4P_FLAGS
  --chars-are-unsigned
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(bibtex_sources
  ${CMAKE_BINARY_DIR}/include/miktex/bibtex.defaults.h
)

create_web_app(BibTeX)

add_dependencies(bibtex gen-defaults)

set_source_files_properties(
  ${CMAKE_BINARY_DIR}/include/miktex/bibtex.defaults.h
  PROPERTIES
    GENERATED TRUE
)

if(LINK_EVERYTHING_STATICALLY)
  target_link_libraries(${MIKTEX_PREFIX}bibtex
    ${w2cemu_lib_name}
  )
else()
  target_link_libraries(bibtex
    PRIVATE
      ${w2cemu_dll_name}
  )
endif()
