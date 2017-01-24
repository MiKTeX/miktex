## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/Config Files")

set(unx_config_files
  ${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_UNX}/log4cxx.${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_UNX}.xml
)

set(win_config_files
  ${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_WIN}/log4cxx.${MIKTEX_SHORTEST_TARGET_SYSTEM_TAG_WIN}.xml
)

set(inc_ini_files
  ${CMAKE_CURRENT_BINARY_DIR}/bibtex-x.ini
)

if(WITH_CONTEXT_SUPPORT)
  list(APPEND inc_ini_files
    ${CMAKE_CURRENT_BINARY_DIR}/context.ini
    ${CMAKE_CURRENT_BINARY_DIR}/formats-context.ini
  )
endif()

set(main_ini_files
  ${CMAKE_CURRENT_BINARY_DIR}/amstex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/bibtex8.ini
  ${CMAKE_CURRENT_BINARY_DIR}/bibtexu.ini
  ${CMAKE_CURRENT_BINARY_DIR}/cslatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/csplain.ini
  ${CMAKE_CURRENT_BINARY_DIR}/dvilualatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/dviluatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/dvipdfm.ini
  ${CMAKE_CURRENT_BINARY_DIR}/dvipdfmx.ini
  ${CMAKE_CURRENT_BINARY_DIR}/elatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/eplain.ini
  ${CMAKE_CURRENT_BINARY_DIR}/etex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/fontinst.ini
  ${CMAKE_CURRENT_BINARY_DIR}/formats.ini
  ${CMAKE_CURRENT_BINARY_DIR}/frlatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/frtex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/ftex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/jadetex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/lambda.ini
  ${CMAKE_CURRENT_BINARY_DIR}/latex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/latex209.ini
  ${CMAKE_CURRENT_BINARY_DIR}/lualatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/luatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/mex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/mllatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/mltex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/olatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/omega.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfamstex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfcslatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfcsplain.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfelatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfjadetex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdflatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfmex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfplatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdftex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdftexinfo.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfxlatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfxmex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfxmltex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/pdfxtex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/platex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/t4ht.ini
  ${CMAKE_CURRENT_BINARY_DIR}/tex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/tex4ht.ini
  ${CMAKE_CURRENT_BINARY_DIR}/texinfo.ini
  ${CMAKE_CURRENT_BINARY_DIR}/ttf2pk.ini
  ${CMAKE_CURRENT_BINARY_DIR}/ttf2tfm.ini
  ${CMAKE_CURRENT_BINARY_DIR}/utf8mex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/xdvipdfmx.ini
  ${CMAKE_CURRENT_BINARY_DIR}/xelatex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/xeplain.ini
  ${CMAKE_CURRENT_BINARY_DIR}/xetex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/xmltex.ini
  ${CMAKE_CURRENT_BINARY_DIR}/yap.ini
)

if(WITH_CONTEXT_SUPPORT)
  list(APPEND main_ini_files
    ${CMAKE_CURRENT_BINARY_DIR}/cont-de.ini
    ${CMAKE_CURRENT_BINARY_DIR}/cont-en.ini
    ${CMAKE_CURRENT_BINARY_DIR}/cont-fr.ini
    ${CMAKE_CURRENT_BINARY_DIR}/cont-it.ini
    ${CMAKE_CURRENT_BINARY_DIR}/cont-nl.ini
    ${CMAKE_CURRENT_BINARY_DIR}/cont-ro.ini
  )
endif()

foreach(i ${main_ini_files} ${inc_ini_files})
  get_filename_component(ininame ${i} NAME)
  add_custom_command(
    OUTPUT
      ${i}
    COMMAND
      ${CAT_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/${ininame}.body
      > ${i}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/${ininame}.body
    VERBATIM
  )
endforeach()

foreach(i texmfapp texapp)
  add_custom_command(
    OUTPUT
      ${i}.ini
    COMMAND
      ${CAT_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
      ${CMAKE_BINARY_DIR}/${MIKTEX_REL_DEFAULTS_DIR}/${i}.defaults
      > ${i}.ini
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
      ${CMAKE_BINARY_DIR}/${MIKTEX_REL_DEFAULTS_DIR}/${i}.defaults
    VERBATIM
  )
  list(APPEND texmf_ini_files ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini)
endforeach()

add_custom_target(gen-config-files ALL
  DEPENDS
    ${inc_ini_files}
    ${main_ini_files}
    ${texmf_ini_files}
)

set_property(TARGET gen-config-files PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(
  FILES
    ${inc_ini_files}
    ${main_ini_files}
    ${texmf_ini_files}
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
