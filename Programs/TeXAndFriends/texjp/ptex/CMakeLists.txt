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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/pTeX")

set(projdir ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(C4P_FLAGS
    --declare-c-type=cinttype
)

if(MSVC)
    add_definitions(
        -D_UNICODE -DUNICODE
    )
endif()

include(libkanji.cmake)

add_subdirectory(pbibtex)
add_subdirectory(pdvitype)
add_subdirectory(ppltotf)
add_subdirectory(ptex)
add_subdirectory(ptftopl)