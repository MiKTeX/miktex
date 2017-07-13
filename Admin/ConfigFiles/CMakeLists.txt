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
  dviluatex
  dvipdfm
  dvipdfmx
  elatex
  eplain
  etex
  fontinst
  formats
  frlatex
  frtex
  ftex
  jadetex
  lambda
  latex
  latex209
  lualatex
  luatex
  mex
  mllatex
  mltex
  olatex
  omega
  pdfamstex
  pdfcslatex
  pdfcsplain
  pdfelatex
  pdfjadetex
  pdflatex
  pdfmex
  pdfplatex
  pdftex
  pdftexinfo
  pdfxlatex
  pdfxmex
  pdfxmltex
  pdfxtex
  platex
  t4ht
  tex
  tex4ht
  texinfo
  ttf2pk
  ttf2tfm
  utf8mex
  xdvipdfmx
  xelatex
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
    ${CMAKE_CURRENT_SOURCE_DIR}/${i}.ini.cmake
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
      ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
      ${CMAKE_CURRENT_BINARY_DIR}/${i}.ini.body
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
      ${CMAKE_CURRENT_SOURCE_DIR}/header.txt
      ${CMAKE_BINARY_DIR}/${MIKTEX_REL_DEFAULTS_DIR}/${i}.defaults
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
