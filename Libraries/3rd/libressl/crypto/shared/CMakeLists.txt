## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2017 Christian Schenk
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

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set(${libressl_crypto_dll_name}_sources ${crypto_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    crypto.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/crypto.rc
  )
  list(APPEND ${libressl_crypto_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/crypto.rc
  )
endif()

add_library(${libressl_crypto_dll_name} SHARED ${${libressl_crypto_dll_name}_sources})

set_property(TARGET ${libressl_crypto_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(CERT_LOAD_MEM)
  add_dependencies(${libressl_crypto_dll_name} gen-crypto-sources)
  set_source_files_properties(
    ${generated_crypto_sources}
    PROPERTIES
      GENERATED TRUE
  )
endif()

set_target_properties(${libressl_crypto_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_MAJOR_MINOR_STR}"
    SOVERSION "1"
)

target_include_directories(${libressl_crypto_dll_name}
  PUBLIC
    ${public_include_directories}
)

if (MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${libressl_crypto_dll_name}
    PRIVATE
      Ws2_32.lib)
endif()

rebase(${libressl_crypto_dll_name})

install(TARGETS ${libressl_crypto_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
