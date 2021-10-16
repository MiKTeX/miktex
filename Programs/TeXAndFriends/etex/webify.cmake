## webify.cmake: create final eTeX web file
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

set(miktex_tex_change_files
    ${MLTEX_MIKTEX_CH}
    ${TEX_MIKTEX_CONSTANTS_CH}
    ${TEX_MIKTEX_CH}
    ${TEX_MIKTEX_HASH_CH}
    ${TEX_MIKTEX_HYPH_CH}
    ${TEX_MIKTEX_POOL_CH}
    ${TEX_MIKTEX_QUIET_CH}
    ${TEX_MIKTEX_SRC_CH}
    ${TEX_MIKTEX_STAT_CH}
    ${TEX_MIKTEX_WRITE18_CH}
    ${TRACINGSTACKLEVELS_CH}
    ${PARTOKEN_CH}
    ${TEX_MIKTEX_FINISH_CH}
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/etex-1.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/etex-1.web
                ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
                ${CMAKE_CURRENT_SOURCE_DIR}/source/etex.ch
                ${CMAKE_CURRENT_SOURCE_DIR}/etex-miktex-adapter.ch
                ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.ch0
                ${miktex_tex_change_files}
                ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.ch1
                ${CMAKE_CURRENT_SOURCE_DIR}/miktex-w2c-adapter.ch
                ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.ech
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/etex-miktex-adapter.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-w2c-adapter.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/etex.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.ch0
        ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.ch1
        ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.ech
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
        ${MIKTEX_PREFIX}tie
        ${miktex_tex_change_files}
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/etex-1.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/etex-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/etex-final.web
                ${CMAKE_CURRENT_BINARY_DIR}/etex-1.web
                ${CMAKE_CURRENT_SOURCE_DIR}/miktex-etex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/etex-1.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-etex.ch
        ${MIKTEX_PREFIX}tie
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/etex-final.web)

# Last but not least: developer's convenience

foreach(_path ${web_files})
    get_filename_component(_name ${_path} NAME_WE)
    set(_out ${CMAKE_CURRENT_BINARY_DIR}/${_name}-n.web)
    add_custom_command(
        OUTPUT
            ${_out}
        COMMAND
            web-n < ${_path} > ${_out}
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
        MAIN_DEPENDENCY
            ${_path}
        DEPENDS
            web-n
        VERBATIM
    )
    list(APPEND web_n_files ${_out})
endforeach()

add_custom_target(etex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET etex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
