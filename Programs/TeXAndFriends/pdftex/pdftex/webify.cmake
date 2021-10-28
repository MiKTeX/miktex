## webify.cmake: create final pdfTeX web file
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

set(pdftex_ch_synctex
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-def.ch0
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-mem.ch0
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-e-mem.ch0
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-e-mem.ch1
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-rec.ch0
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-rec.ch1
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-e-rec.ch0
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-pdf-rec.ch2
)

set(miktex_synctex_changefiles
    ${TEX_MIKTEX_SYNCTEX_CH}
)

set(miktex_tex_change_files
    ${MIKTEX_TEX_MLTEX_CH}
    ${MIKTEX_TEX_CONSTANTS_CH}
    ${MIKTEX_TEX_CH}
    ${MIKTEX_TEX_HASH_CH}
    ${MIKTEX_TEX_HYPH_CH}
    ${MIKTEX_TEX_POOL_CH}
    ${MIKTEX_TEX_QUIET_CH}
    ${MIKTEX_TEX_SRC_CH}
    ${miktex_synctex_changefiles}
    ${MIKTEX_TEX_STAT_CH}
    ${MIKTEX_TEX_WRITE18_CH}
    ${TRACINGSTACKLEVELS_CH}
    ${PARTOKEN_102_CH}
    ${PARTOKEN_CH}
    ${ENCTEX1_CH}
    ${ENCTEX_PDFTEX_CH}
    ${ENCTEX2_CH}
    ${MIKTEX_TEX_ENCTEX_CH}
    ${MIKTEX_TEX_WEB2C_CH}
    ${MIKTEX_TEX_FINISH_CH}
)

list(APPEND web_files ${projdir}/source/pdftex.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pdftex-1.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/pdftex-1.web
            ${projdir}/source/pdftex.web
            ${ETEX_MIKTEX_ADAPTER_CH}
            ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-miktex-adapter.ch
            ${miktex_tex_change_files}
            ${pdftex_ch_synctex}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${projdir}/source/pdftex.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-miktex-adapter.ch
        ${ETEX_MIKTEX_ADAPTER_CH}
        ${MIKTEX_PREFIX}tie
        ${miktex_tex_change_files}
        ${pdftex_ch_synctex}
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pdftex-1.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pdftex-2.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/pdftex-2.web
            ${CMAKE_CURRENT_BINARY_DIR}/pdftex-1.web
            ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-1to2-adapter.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/pdftex-1.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/pdftex-1to2-adapter.ch
        ${MIKTEX_PREFIX}tie
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pdftex-2.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pdftex-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/pdftex-final.web
            ${CMAKE_CURRENT_BINARY_DIR}/pdftex-2.web
            ${projdir}/source/pdftex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/pdftex-2.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${projdir}/source/pdftex.ch
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pdftex-final.web)

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

add_custom_target(pdftex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET pdftex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
