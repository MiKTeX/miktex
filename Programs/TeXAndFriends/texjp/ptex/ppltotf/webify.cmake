## webify.cmake
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-pltotf.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/miktex-pltotf.web
                ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEXWARE_DIR}/source/pltotf.web
                ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEXWARE_DIR}/pltotf-miktex.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEXWARE_DIR}/source/pltotf.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_TEXWARE_DIR}/pltotf-miktex.ch
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/miktex-pltotf.web)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pre-ppltotf.web
    COMMAND
        ${MIKTEX_PREFIX}tie
        -m ${CMAKE_CURRENT_BINARY_DIR}/pre-ppltotf.web
            ${CMAKE_CURRENT_BINARY_DIR}/miktex-pltotf.web
            ${CMAKE_CURRENT_SOURCE_DIR}/miktex-ppltotf-adapter.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-pltotf.web
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/miktex-ppltotf-adapter.ch
        ${MIKTEX_PREFIX}tie
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/pre-ppltotf.web)

add_custom_command(
    OUTPUT
         ${CMAKE_CURRENT_BINARY_DIR}/ppltotf-final.web
    COMMAND
        ${MIKTEX_PREFIX}tie
            -m ${CMAKE_CURRENT_BINARY_DIR}/ppltotf-final.web
                ${CMAKE_CURRENT_BINARY_DIR}/pre-ppltotf.web
                ${projdir}/source/ppltotf.ch
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/pre-ppltotf.web
    DEPENDS
        ${MIKTEX_PREFIX}tie
        ${projdir}/source/ppltotf.ch
    VERBATIM
)

list(APPEND web_files ${CMAKE_CURRENT_BINARY_DIR}/ppltotf-final.web)

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

add_custom_target(ppltotf-dev ALL DEPENDS ${web_n_files})

set_property(TARGET ppltotf-dev PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
