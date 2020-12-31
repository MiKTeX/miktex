## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2020 Christian Schenk
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

set(${setup_dll_name}_sources ${setup_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../win/setup.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.rc
  )
  list(APPEND ${setup_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/setup.rc
  )
endif()

add_library(${setup_dll_name} SHARED ${${setup_dll_name}_sources})

set_property(TARGET ${setup_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${setup_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${setup_dll_name}
  PUBLIC
    -DMIKTEX_SETUP_SHARED
)

target_include_directories(${setup_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${setup_dll_name}
  PUBLIC
    ${core_dll_name}
    ${loc_dll_name}
    ${mpm_dll_name}
    ${nlohmann_json_dll_name}
)

add_dependencies(${setup_dll_name} gen-${MIKTEX_COMP_ID}-sources)

set_source_files_properties(
  ${generated_${MIKTEX_COMP_ID}_sources}
  PROPERTIES
    GENERATED TRUE
)

if(USE_SYSTEM_BOOST)
  target_link_libraries(${setup_dll_name} PRIVATE Boost::locale)
endif()

if(USE_SYSTEM_FMT)
  target_link_libraries(${setup_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${setup_dll_name} PRIVATE ${fmt_dll_name})
endif()

target_link_libraries(${setup_dll_name}
  PRIVATE
    ${extractor_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${setup_dll_name}
    PRIVATE
      version
  )
endif()

install(TARGETS ${setup_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
