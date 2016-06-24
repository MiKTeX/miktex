## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2016 Christian Schenk
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

add_library(${lua_lib_name} STATIC ${lua_lib_sources})

set_property(TARGET ${lua_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${lua_lib_name}
  PUBLIC
    ${public_include_directories}
)

if(MIKTEX_UNIX_ALIKE)
  target_link_libraries(${lua_lib_name}
    PUBLIC
      ${MATH_LIBRARY}
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${lua_lib_name}
    PUBLIC
      ${utf8wrap_dll_name}
  )
endif()

if(INSTALL_STATIC_LIBRARIES)
  install(TARGETS ${lua_lib_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endif()
