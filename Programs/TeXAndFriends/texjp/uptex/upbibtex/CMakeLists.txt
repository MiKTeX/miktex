## CMakeLists.txt:
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

set(MIKTEX_CURRENT_FOLDER ${MIKTEX_CURRENT_FOLDER}/upBibTeX)

default_char_type_is_unsigned()

list(APPEND C4P_FLAGS
  --chars-are-unsigned
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(upbibtex_target_name upbibtex)

configure_file(
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_BIBTEX_DIR}/dyn.sed
    ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed
    COPYONLY
)

include(webify.cmake)

set(upbibtex_web_file ${CMAKE_CURRENT_BINARY_DIR}/upbibtex-final.web)

create_web_app(upBibTeX)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}upbibtex
        texjp-ukanji
    )
else()
    target_link_libraries(${upbibtex_target_name}
        PRIVATE
            texjp-ukanji
    )
endif()