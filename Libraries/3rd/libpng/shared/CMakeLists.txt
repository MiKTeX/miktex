## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2020 Christian Schenk
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

set(${png_dll_name}_sources ${png_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../libpng.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libpng.rc
  )
  list(APPEND ${png_dll_name}_sources
      ${CMAKE_CURRENT_BINARY_DIR}/libpng.rc
    )
endif()

add_library(${png_dll_name} SHARED ${${png_dll_name}_sources})

set_property(TARGET ${png_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${png_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

if(WIN32)
  set_target_properties(${png_dll_name}
    PROPERTIES
      DEFINE_SYMBOL PNG_BUILD_DLL)
endif()

target_compile_definitions(${png_dll_name}
  INTERFACE
    ${interface_definitions}
    -DPNG_USE_DLL
)

target_include_directories(${png_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${png_dll_name}
  PRIVATE
    ${zlib_dll_name}
)

install(TARGETS ${png_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
