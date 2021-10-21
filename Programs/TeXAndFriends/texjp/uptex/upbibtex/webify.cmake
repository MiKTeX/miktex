## webify.cmake: create final upBibTeX web file
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

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
                ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/source/bibtex.web
                ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/bibtex-miktex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/source/bibtex.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/bibtex-miktex.ch
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-pbibtex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-pbibtex.web
            ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/pbibtex/miktex-pbibtex-adapter.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/pbibtex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
    DEPENDS
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/pbibtex/miktex-pbibtex-adapter.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/source/pbibtex.ch
        ${MIKTEX_PREFIX}tie
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/miktex-pbibtex.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pre-upbibtex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/pre-upbibtex.web
            ${CMAKE_CURRENT_BINARY_DIR}/miktex-pbibtex.web
            ${CMAKE_CURRENT_SOURCE_DIR}/miktex-upbibtex-adapter.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-pbibtex.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-upbibtex-adapter.ch
        ${MIKTEX_PREFIX}tie
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pre-upbibtex.web)

add_custom_command(
    OUTPUT
         ${CMAKE_CURRENT_BINARY_DIR}/upbibtex-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m  ${CMAKE_CURRENT_BINARY_DIR}/upbibtex-final.web
                ${CMAKE_CURRENT_BINARY_DIR}/pre-upbibtex.web
                ${projdir}/source/upbibtex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/pre-upbibtex.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${projdir}/source/upbibtex.ch
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/upbibtex-final.web)

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

add_custom_target(upbibtex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET upbibtex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
