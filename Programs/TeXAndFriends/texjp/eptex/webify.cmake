## webify.cmake: create final e-pTeX web file
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
    ${SHOWSTREAM_CH}
    ${MIKTEX_TEX_WEB2C_CH}
    ${MIKTEX_TEX_FINISH_CH}
)

set(eptex_ch_synctex
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-def.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-ep-mem.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-mem.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-e-mem.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-ep-mem.ch1
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-p-rec.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-rec.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-rec.ch1
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-ep-rec.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-e-rec.ch0
	${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-p-rec.ch1
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-etex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-etex.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/etex.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/etex-miktex-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/tex.ch0
            ${miktex_tex_change_files}
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/miktex-w2c-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/tex.ech
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
    DEPENDS
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/etex-miktex-adapter.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/miktex-w2c-adapter.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/etex.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/tex.ch0
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/tex.ech
        ${MIKTEX_PREFIX}tie
        ${miktex_tex_change_files}
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/miktex-etex.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pre-eptex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/pre-eptex.web
            ${CMAKE_CURRENT_BINARY_DIR}/miktex-etex.web
            ${CMAKE_CURRENT_SOURCE_DIR}/miktex-eptex-adapter.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-etex.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-eptex-adapter.ch
        ${MIKTEX_PREFIX}tie
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pre-eptex.web)

add_custom_command(
    OUTPUT
         ${CMAKE_CURRENT_BINARY_DIR}/eptex-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/eptex-final.web
            ${CMAKE_CURRENT_BINARY_DIR}/pre-eptex.web
            ${CMAKE_CURRENT_SOURCE_DIR}/source/eptex-base.ch
            ${CMAKE_CURRENT_SOURCE_DIR}/source/etex.ch0
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/ptex-base.ch
	        ${CMAKE_CURRENT_SOURCE_DIR}/source/eptex.ech
	        ${CMAKE_CURRENT_SOURCE_DIR}/source/etex.ch1
            ${eptex_ch_synctex}
            ${CMAKE_CURRENT_SOURCE_DIR}/source/fam256.ch
            ${CMAKE_CURRENT_SOURCE_DIR}/source/pdfutils.ch
            ${CMAKE_CURRENT_SOURCE_DIR}/source/char-warning-eptex.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/miktex-etex.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/pre-eptex.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/char-warning-eptex.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/eptex-base.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/eptex.ech
        ${CMAKE_CURRENT_SOURCE_DIR}/source/etex.ch0
        ${CMAKE_CURRENT_SOURCE_DIR}/source/etex.ch1
        ${CMAKE_CURRENT_SOURCE_DIR}/source/fam256.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/pdfutils.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/miktex-etex.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex-adapter.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/ptex-base.ch
        ${MIKTEX_PREFIX}tie
        ${eptex_ch_synctex}
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/eptex-final.web)

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

add_custom_target(eptex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET eptex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
