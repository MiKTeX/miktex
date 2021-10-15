## CMakeLists.txt: miktex-ttf2afm build rules
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

set(ttf2afm_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${projdir}/source/ttf2afm.c
    ttf2afm-version.h
)

set_source_files_properties(
    ${projdir}/source/ttf2afm.c
    PROPERTIES LANGUAGE CXX
)

set_source_files_properties(
  ${MIKTEX_LIBRARY_WRAPPER}
  PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ttf2afm_sources
        ${MIKTEX_COMMON_MANIFEST}
        ttf2afm.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}ttf2afm ${ttf2afm_sources})

set_property(TARGET ${MIKTEX_PREFIX}ttf2afm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/ttf2afm)

target_link_libraries(${MIKTEX_PREFIX}ttf2afm
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}ttf2afm
        ${getopt_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}ttf2afm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
