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

set(${harfbuzz_dll_name}_sources ${harfbuzz_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../harfbuzz.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz.rc
  )
  list(APPEND ${harfbuzz_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz.rc
  )
endif()

add_library(${harfbuzz_dll_name} SHARED ${${harfbuzz_dll_name}_sources})

set_property(TARGET ${harfbuzz_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${harfbuzz_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version(
  ${harfbuzz_dll_name}
  ${MIKTEX_COMP_INTERFACE_VERSION}
  ${MIKTEX_COMP_J2000_VERSION}
)

target_include_directories(${harfbuzz_dll_name} PUBLIC ${public_include_directories})

if(USE_SYSTEM_GRAPHITE2)
  target_link_libraries(${harfbuzz_dll_name} PUBLIC MiKTeX::Imported::GRAPHITE2)
else()
  target_link_libraries(${harfbuzz_dll_name} PUBLIC ${graphite2_dll_name})
endif()

target_link_libraries(${harfbuzz_dll_name}
  PUBLIC
    ICU::i18n
    ICU::uc
)

rebase(${harfbuzz_dll_name})

install(TARGETS ${harfbuzz_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

