## CMakeLists.txt: miktex-pdftex localization
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

set(MIKTEX_COMP_ID "miktex-pdftex")

list(APPEND l_sources miktex-pdftex.h)

add_custom_target(${MIKTEX_COMP_ID}-pot
    COMMAND
        ${XGETTEXT_EXECUTABLE} --join-existing --keyword=T_ --from-code=UTF-8 --add-comments --output=po/${MIKTEX_COMP_ID}.pot ${l_sources}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET ${MIKTEX_COMP_ID}-pot PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

add_custom_target(gen-${MIKTEX_COMP_ID}-translations)

set_property(TARGET gen-${MIKTEX_COMP_ID}-translations PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

add_dependencies(gen-${MIKTEX_COMP_ID}-translations ${MIKTEX_COMP_ID}-pot)

add_custom_target(${MIKTEX_COMP_ID}-mo ALL)

set_property(TARGET ${MIKTEX_COMP_ID}-mo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

foreach(l ${MIKTEX_TRANSLATION_DONE_LOCALES})
    add_custom_target(${MIKTEX_COMP_ID}-po-${l}
        COMMAND
            ${MSGMERGE_EXECUTABLE} --update --backup=none po/${l}/${MIKTEX_COMP_ID}.po po/${MIKTEX_COMP_ID}.pot
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_property(TARGET ${MIKTEX_COMP_ID}-po-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
    add_dependencies(gen-${MIKTEX_COMP_ID}-translations ${MIKTEX_COMP_ID}-po-${l})
    add_custom_target(${MIKTEX_COMP_ID}-mo-${l} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo)
    set_property(TARGET ${MIKTEX_COMP_ID}-mo-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
    add_dependencies(${MIKTEX_COMP_ID}-mo ${MIKTEX_COMP_ID}-mo-${l})
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}
        COMMAND
            ${MSGFMT_EXECUTABLE} --output-file=locale/${l}/${MIKTEX_COMP_ID}.mo ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/${MIKTEX_COMP_ID}.po
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/${MIKTEX_COMP_ID}.po
    )
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}
        COMMAND
            bintoc messages_${l}
                < ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
                > ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h
        COMMAND
            ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h.tmp
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo
            bintoc
        VERBATIM
    )
    list(APPEND generated_${MIKTEX_COMP_ID}_sources ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h)
    set(resource_includes
        "${resource_includes}#include \"${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/${MIKTEX_COMP_ID}.mo.h\"\n"
    )
    set(resource_adders
        "${resource_adders}R_(\":/${l}/LC_MESSAGES/${MIKTEX_COMP_ID}.mo\", messages_${l});\n"
    )
endforeach()

configure_file(
    PdfTeXResources.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/PdfTeXResources.h
)

add_custom_target(gen-${MIKTEX_COMP_ID}-sources DEPENDS
    ${generated_${MIKTEX_COMP_ID}_sources}
)

set_property(TARGET gen-${MIKTEX_COMP_ID}-sources PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

list(APPEND ${pdftex_target_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/PdfTeXResources.h
    ${generated_${MIKTEX_COMP_ID}_sources}
)

set_source_files_properties(
    ${generated_${MIKTEX_COMP_ID}_sources}
    PROPERTIES
        GENERATED TRUE
)
