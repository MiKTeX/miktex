## CMakeLists.txt
##
## Copyright (C) 2020-2021 Christian Schenk
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

set(${loc_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../win/loc.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/loc.rc
  )
  list(APPEND ${loc_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/loc.rc
  )
endif()

add_library(${loc_dll_name} SHARED ${${loc_dll_name}_sources})

set_property(TARGET ${loc_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${loc_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${loc_dll_name}
  PUBLIC
    -DMIKTEX_LOC_SHARED
)

target_include_directories(${loc_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${loc_dll_name}
  PRIVATE
    ${util_dll_name}
  PUBLIC
    ${config_dll_name}
    ${res_dll_name}
)

if(MIKTEX_MACOS_BUNDLE)
  target_link_libraries(${loc_dll_name}
    PRIVATE
      ${CoreFoundation}
  )
endif()

if(USE_SYSTEM_BOOST)
  target_link_libraries(${loc_dll_name} PRIVATE Boost::locale)
endif()

install(TARGETS ${loc_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
