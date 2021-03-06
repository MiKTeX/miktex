## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2018-2020 Christian Schenk
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

set(${freeglut_dll_name}_sources ${freeglut_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../freeglut.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/freeglut.rc
  )
  list(APPEND ${freeglut_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/freeglut.rc
  )
endif()

add_library(${freeglut_dll_name} SHARED ${${freeglut_dll_name}_sources})

set_property(TARGET ${freeglut_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${freeglut_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version_properties(
  ${freeglut_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${freeglut_dll_name} PUBLIC ${public_include_directories})

if(USE_SYSTEM_OPENGL AND MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${freeglut_dll_name} PUBLIC opengl32)
else()
  target_link_libraries(${freeglut_dll_name} PUBLIC MiKTeX::Imported::OPENGL)
endif()

target_link_libraries(${freeglut_dll_name}
  PUBLIC
    ${core_dll_name}
    ${util_dll_name}
)
      
if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${freeglut_dll_name}
    PUBLIC
      winmm
  )
endif()

install(TARGETS ${freeglut_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(MiKTeX FILES ${miktex_sources})
