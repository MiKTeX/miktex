## webify.cmake
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
                ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/source/bibtex.web
                ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/miktex-bibtex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/source/bibtex.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/miktex-bibtex.ch
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pre-pbibtex.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/pre-pbibtex.web
            ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
            ${CMAKE_CURRENT_SOURCE_DIR}/miktex-pbibtex-adapter.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-bibtex.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-pbibtex-adapter.ch
        ${MIKTEX_PREFIX}tie
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pre-pbibtex.web)

add_custom_command(
    OUTPUT
         ${CMAKE_CURRENT_BINARY_DIR}/pbibtex-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m  ${CMAKE_CURRENT_BINARY_DIR}/pbibtex-final.web
                ${CMAKE_CURRENT_BINARY_DIR}/pre-pbibtex.web
                ${projdir}/source/pbibtex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/pre-pbibtex.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${projdir}/source/pbibtex.ch
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pbibtex-final.web)

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

add_custom_target(pbibtex-dev ALL DEPENDS ${web_n_files})

set_property(TARGET pbibtex-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
