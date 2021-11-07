## webify.cmake: create final upTeX web file
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

set(tex_miktex_change_files
    ${MIKTEX_TEX_MLTEX_CH}
    ${MIKTEX_TEX_CONSTANTS_CH}
    ${MIKTEX_TEX_CH}
    ${MIKTEX_TEX_HASH_CH}
    ${MIKTEX_TEX_HYPH_CH}
    ${MIKTEX_TEX_POOL_CH}
    ${MIKTEX_TEX_QUIET_CH}
    ${MIKTEX_TEX_SRC_CH}
    ${MIKTEX_TEX_STAT_CH}
    ${MIKTEX_TEX_WRITE18_CH}
    ${TRACINGSTACKLEVELS_CH}
    ${PARTOKEN_CH}
    ${MIKTEX_TEX_WEB2C_CH}
    ${MIKTEX_TEX_FINISH_CH}
)

set(uptex_ch_synctex
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-def.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-p-mem.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-mem.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-p-mem.ch1
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-p-rec.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-rec.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-rec.ch1
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-rec.ch2
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-p-rec.ch1
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
            ${tex_miktex_change_files}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${tex_miktex_change_files}
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptex.web
            ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/ptex-base.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-tex.web
    DEPENDS
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex-adapter.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/ptex-base.ch
        ${MIKTEX_PREFIX}tie
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptex.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pre-uptex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/pre-uptex.web
            ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptex.web
            ${CMAKE_CURRENT_SOURCE_DIR}/miktex-uptex-adapter.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptex.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-uptex-adapter.ch
        ${MIKTEX_PREFIX}tie
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pre-uptex.web)

add_custom_command(
    OUTPUT
         ${CMAKE_CURRENT_BINARY_DIR}/uptex-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/uptex-final.web
            ${CMAKE_CURRENT_BINARY_DIR}/pre-uptex.web
            ${projdir}/source/uptex-m.ch
            ${uptex_ch_synctex}
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/pre-uptex.web
    DEPENDS
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex.ch
        ${MIKTEX_PREFIX}tie
        ${projdir}/source/uptex-m.ch
        ${uptex_ch_synctex}
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/uptex-final.web)

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

add_custom_target(uptex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET uptex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
