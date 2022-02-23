## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/Config Files")

set(unx_config_files
    ${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_UNX}/log4cxx.${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_UNX}.xml
)

set(win_config_files
    ${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_WIN}/log4cxx.${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_WIN}.xml
)

set(inc_ini_names
    bibtex-x
)

if(WITH_CONTEXT_SUPPORT)
    list(APPEND inc_ini_names
        context
        formats-context
    )
endif()

set(main_ini_names
    amstex
    bibtex8
    bibtexu
    cslatex
    csplain
    dvilualatex
    dvilualatex-dev
    dviluatex
    dvipdfm
    dvipdfmx
    elatex
    eplain
    eptex
    etex
    euptex
    fontinst
    formats
    frlatex
    frtex
    ftex
    jadetex
    latex
    latex-dev
    latex209
    luahblatex
    luahblatex-dev
    luahbtex
    lualatex
    lualatex-dev
    luatex
    mllatex
    mltex
    olatex
    pdfamstex
    pdfcslatex
    pdfcsplain
    pdfelatex
    pdfjadetex
    pdflatex
    pdflatex-dev
    pdfplatex
    pdftex
    pdftexinfo
    pdfxlatex
    pdfxmltex
    pdfxtex
    platex
    platex-dev
    ptex
    t4ht
    tex
    tex4ht
    texinfo
    ttf2pk
    ttf2tfm
    uplatex
    uplatex-dev
    uptex
    utf8mex
    xdvipdfmx
    xelatex
    xelatex-dev
    xeplain
    xetex
    xmltex
    yap
)

if(WITH_CONTEXT_SUPPORT)
    list(APPEND main_ini_names
        cont-de
        cont-en
        cont-fr
        cont-it
        cont-nl
        cont-ro
    )
endif()

foreach(i ${main_ini_names} ${inc_ini_names})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${i}.ini.in
        ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini.body
        NEWLINE_STYLE UNIX
    )
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini
        COMMAND
            ${CAT_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
                ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini.body
                > ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini.body
            ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
        VERBATIM
    )
    list(APPEND all_ini_files ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini)
endforeach()

foreach(i texmfapp texapp)
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini
        COMMAND
            ${CAT_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
                ${CMAKE_BINARY_DIR}/${MIKTEX_REL_DEFAULTS_DIR}/${i}.defaults
                > ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS
            ${CMAKE_BINARY_DIR}/${MIKTEX_REL_DEFAULTS_DIR}/${i}.defaults
            ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
        VERBATIM
    )
    list(APPEND all_ini_files ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini)
endforeach()

add_custom_target(gen-config-files ALL
    DEPENDS
        ${all_ini_files}
)

set_property(TARGET gen-config-files PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(
    FILES
        ${all_ini_files}
    DESTINATION
        ${MIKTEX_CONFIG_DESTINATION_DIR}
)

install(
    FILES
        ${unx_config_files}
    DESTINATION
        ${MIKTEX_CONFIG_DESTINATION_DIR}/${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_UNX}
)

install(
    FILES
        ${win_config_files}
    DESTINATION
        ${MIKTEX_CONFIG_DESTINATION_DIR}/${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_WIN}
)
