## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2019 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_EDITORS_FOLDER}/TeXworks/synctex")

hide_symbols()

set(synctex_sources
  ${projdir}/source/modules/synctex/synctex_parser.c
  ${projdir}/source/modules/synctex/synctex_parser_utils.c
  ${projdir}/source/modules/synctex/synctex_parser.h
  ${projdir}/source/modules/synctex/synctex_parser_advanced.h
  ${projdir}/source/modules/synctex/synctex_parser_utils.h
  ${projdir}/source/modules/synctex/synctex_version.h
)

add_library(synctex-static STATIC
  ${synctex_sources}
)

if(USE_SYSTEM_ZLIB)
  target_link_libraries(synctex-static PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(synctex-static PUBLIC ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(synctex-static PUBLIC ${utf8wrap_dll_name})
endif()

set_property(TARGET synctex-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(synctex-static
  PUBLIC
    ${projdir}/source/modules/synctex
)
