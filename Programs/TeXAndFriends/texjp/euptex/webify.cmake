## webify.cmake: create final eu-pTeX web file
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

set(euptex_ch_synctex
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
        ${CMAKE_CURRENT_BINARY_DIR}/euptex-1.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/euptex-1.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEX_DIR}/source/tex.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/etex.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/etex-miktex-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/tex.ch0
            ${miktex_tex_change_files}
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/miktex-w2c-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/source/tex.ech
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/miktex-eptex-adapter.ch
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
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/miktex-eptex-adapter.ch
        ${MIKTEX_PREFIX}tie
        ${miktex_tex_change_files}
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/euptex-1.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/euptex-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/euptex-final.web
            ${CMAKE_CURRENT_BINARY_DIR}/euptex-1.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/etex.ch0
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/ptex-base.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UPTEX_DIR}/source/uptex-m.ch
	        ${CMAKE_CURRENT_SOURCE_DIR}/source/euptex.ch0
	        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/eptex.ech
	        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/etex.ch1
	        ${CMAKE_CURRENT_SOURCE_DIR}/source/euptex.ch1
	        ${euptex_ch_synctex}
	        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/fam256.ch
            ${CMAKE_CURRENT_SOURCE_DIR}/source/pdfstrcmp-eup-pre.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/pdfutils.ch
            ${CMAKE_CURRENT_SOURCE_DIR}/source/pdfstrcmp-eup-post.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/char-warning-eptex.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UPTEX_DIR}/uptex/miktex-uptex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/euptex-1.web
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/euptex-1.web
        ${CMAKE_CURRENT_SOURCE_DIR}/source/euptex.ch0
        ${CMAKE_CURRENT_SOURCE_DIR}/source/euptex.ch1
        ${CMAKE_CURRENT_SOURCE_DIR}/source/pdfstrcmp-eup-post.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/pdfstrcmp-eup-pre.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/char-warning-eptex.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/eptex.ech
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/etex.ch0
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/etex.ch1
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/fam256.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_EPTEX_DIR}/source/pdfutils.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex-adapter.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/ptex-base.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UPTEX_DIR}/source/uptex-m.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UPTEX_DIR}/uptex/miktex-uptex.ch
        ${MIKTEX_PREFIX}tie
        ${euptex_ch_synctex}
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/euptex-final.web)

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

add_custom_target(euptex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET euptex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
