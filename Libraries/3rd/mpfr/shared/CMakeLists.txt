## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2017 Christian Schenk
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

set(${mpfr_dll_name}_sources ${mpfr_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../mpfr.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mpfr.rc
  )
  list(APPEND ${mpfr_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/mpfr.rc
  )
endif()

add_library(${mpfr_dll_name} SHARED ${${mpfr_dll_name}_sources})

set_property(TARGET ${mpfr_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${mpfr_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version(
  ${mpfr_dll_name}
  ${MIKTEX_COMP_INTERFACE_VERSION}
  ${MIKTEX_COMP_J2000_VERSION}
)

target_include_directories(${mpfr_dll_name} PUBLIC ${public_include_directories})

if(USE_SYSTEM_GMP)
  target_link_libraries(${mpfr_dll_name} PUNLIC MiKTeX::Imported::GMP)
else()
  target_link_libraries(${mpfr_dll_name} PUBLIC ${gmp_lib_name})
endif()

install(TARGETS ${mpfr_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
