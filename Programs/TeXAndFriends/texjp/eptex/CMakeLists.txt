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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/e-pTeX")

if(LINK_EVERYTHING_STATICALLY)
  use_static_crt()
endif()

default_char_type_is_unsigned()

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_INCLUDE_DIR}
)

add_definitions(
    -DHAVE_C4P_PRE_H
    -DSYNCTEX_ENGINE_H="synctex-eptex.h"
    -DTeX
    -D__SyncTeX__
    -DepTeX
)

list(APPEND C4P_FLAGS
    --auto-exit-label=10
    --chars-are-unsigned
    --emit-optimize-pragmas
)

set(eptex_target_name eptex)

include(webify.cmake)

list(APPEND ${eptex_target_name}_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/eptexextra.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/eptexextra.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/eptexextra.c
    PROPERTIES LANGUAGE CXX
)

list(APPEND ${eptex_target_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/eptex_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-eptex.cpp
)

list(APPEND ${eptex_target_name}_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/c4p_pre.h
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-first.h
)

list(APPEND ${eptex_target_name}_sources
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-common.h
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-eptex.h
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.c
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.h
)

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.c
    PROPERTIES LANGUAGE CXX
)

file(READ ${MIKTEX_DYN_TEX_SCRIPT} tex_dyn_sed_contents)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${tex_dyn_sed_contents}")
endif()

set(eptex_web_file ${CMAKE_CURRENT_BINARY_DIR}/eptex-final.web)
set(eptex_header_file ${CMAKE_CURRENT_BINARY_DIR}/eptexd.h)

create_web_app(epTeX)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/eptex_pool.cpp
    COMMAND
        inipool
            ${CMAKE_CURRENT_BINARY_DIR}/eptex.pool
            miktex-eptex.h
            ${eptex_progclass}
            ${eptex_prog}
        > ${CMAKE_CURRENT_BINARY_DIR}/eptex_pool.cpp
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/eptex.pool
    DEPENDS
        inipool
)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}eptex
        ${kpsemu_lib_name}
        ${w2cemu_lib_name}
        ${web2c_sources_lib_name}
        texjp-kanji
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${MIKTEX_PREFIX}eptex
            ${utf8wrap_lib_name}
        )
    endif()
else()
    target_link_libraries(${eptex_target_name}
        PRIVATE
            ${kpsemu_dll_name}
            ${w2cemu_dll_name}
            ${web2c_sources_dll_name}
            texjp-kanji
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${eptex_target_name}
            PRIVATE
                ${utf8wrap_dll_name}
        )
    endif()
endif()
