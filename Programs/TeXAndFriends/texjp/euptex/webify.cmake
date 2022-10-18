## webify.cmake
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

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
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_UPTEX_DIR}/uptex/miktex-uptex.ch
            ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/miktex-etex.ch
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
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_ETEX_DIR}/miktex-etex.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex-adapter.ch
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_PTEX_DIR}/ptex/miktex-ptex.ch
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
