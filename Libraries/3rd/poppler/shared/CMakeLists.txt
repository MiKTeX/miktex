## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017 Christian Schenk
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

set(${poppler_dll_name}_sources ${poppler_sources})

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../poppler.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/poppler.rc
  )
  list(APPEND ${poppler_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/poppler.rc
  )
endif()

add_library(${poppler_dll_name} SHARED ${${poppler_dll_name}_sources})

set_property(TARGET ${poppler_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${poppler_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version(
  ${poppler_dll_name}
  ${MIKTEX_COMP_INTERFACE_VERSION}
  ${MIKTEX_COMP_J2000_VERSION}
)

target_include_directories(${poppler_dll_name} PUBLIC ${public_include_directories})

target_link_libraries(${poppler_dll_name}
  PUBLIC
    ${core_dll_name}
    ${tiff_dll_name}
    Threads::Threads
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${poppler_dll_name}
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(USE_SYSTEM_CAIRO)
  target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::CAIRO)
else()
  target_link_libraries(${poppler_dll_name} PUBLIC ${cairo_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
  target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${poppler_dll_name} PUBLIC ${freetype2_dll_name})
endif()

if(USE_SYSTEM_PNG)
  target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::PNG)
else()
  target_link_libraries(${poppler_dll_name} PUBLIC ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${poppler_dll_name} PUBLIC ${zlib_dll_name})
endif()

if(ENABLE_LIBCURL)
  if(USE_SYSTEM_CURL)
    target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::CURL)
  else()
    target_link_libraries(${poppler_dll_name} PUBLIC ${curl_dll_name})
  endif()
endif()

if(ENABLE_LIBJPEG)
  if(USE_SYSTEM_JPEG)
    target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::JPEG)
  else()
    target_link_libraries(${poppler_dll_name} PUBLIC ${jpeg_dll_name})
  endif()
endif()

if(ENABLE_ZLIB)
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(${poppler_dll_name} PUBLIC ${zlib_dll_name})
  endif()
endif()

if(WITH_FONTCONFIGURATION_FONTCONFIG)
  if(USE_SYSTEM_FONTCONFIG)
    target_link_libraries(${poppler_dll_name} PUBLIC MiKTeX::Imported::FONTCONFIG)
  else()
    target_link_libraries(${poppler_dll_name} PUBLIC ${fontconfig_dll_name})
  endif()
endif()

rebase(${poppler_dll_name})

install(TARGETS ${poppler_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
