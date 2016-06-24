## CreateSymbolicLinks.cmake
##
## Copyright (C) 2016 Christian Schenk
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

macro(create_symbolic_links _target)
  foreach(l ${ARGN})
    add_custom_target(${l} ALL
      COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:${_target}> ${l}
      DEPENDS ${_target}
      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${l} DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
  endforeach()
endmacro()
