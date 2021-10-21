## libkanji.cmake:
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

set(libkanji_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji_dump.c
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji_dump.c
    PROPERTIES LANGUAGE CXX
)

add_library(texjp-kanji STATIC ${libkanji_sources})

target_include_directories(texjp-kanji
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(texjp-kanji
    PUBLIC
        ${ptexenc_dll_name}
        ${w2cemu_dll_name}
        ${zlib_dll_name}
)

target_link_libraries(texjp-kanji
    PRIVATE
        ${web2c_sources_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(texjp-kanji
        PRIVATE
            ${utf8wrap_dll_name}
    )
endif()
