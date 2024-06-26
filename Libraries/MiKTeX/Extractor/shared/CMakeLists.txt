## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2007-2020 Christian Schenk
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

set(${extractor_dll_name}_sources ${extractor_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../extractor.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/extractor.rc
  )
  list(APPEND ${extractor_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/extractor.rc
  )
endif()

add_library(${extractor_dll_name} SHARED ${${extractor_dll_name}_sources})

set_property(TARGET ${extractor_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
  ${extractor_dll_name}
  ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
  ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${extractor_dll_name}
  PUBLIC
    -DMIKTEX_EXTRACTOR_SHARED
)

target_include_directories(${extractor_dll_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_FMT)
  target_link_libraries(${extractor_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
  target_link_libraries(${extractor_dll_name} PRIVATE ${fmt_dll_name})
endif()

if(USE_SYSTEM_MSPACK)
  target_link_libraries(${extractor_dll_name} PRIVATE MiKTeX::Imported::MSPACK)
else()
  target_link_libraries(${extractor_dll_name} PRIVATE ${mspack_dll_name})
endif()

target_link_libraries(${extractor_dll_name}
  PRIVATE
    ${core_dll_name}
)

delay_load(${extractor_dll_name}
  ${bzip2_dll_name}
  ${mspack_dll_name}
)

install(TARGETS ${extractor_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
