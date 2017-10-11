## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2009-2017 Christian Schenk
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

add_library(${poppler_lib_name} STATIC ${poppler_sources})

set_property(TARGET ${poppler_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(NOT POPPLER_VERSION VERSION_LESS 0.59.0)
  set_target_properties(${poppler_lib_name}
    PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS MIKTEX_POPPLER_59=1
  )
endif()

target_compile_definitions(${poppler_lib_name}
  PUBLIC
    -DMIKTEX_POPPLER_STATIC
)

target_include_directories(${poppler_lib_name} PUBLIC ${public_include_directories})

target_link_libraries(${poppler_lib_name}
  PUBLIC
    ${core_dll_name}
    ${tiff_dll_name}
    Threads::Threads
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${poppler_lib_name}
    PUBLIC
      ${unxemu_dll_name}
      ${utf8wrap_dll_name}
  )
endif()

if(USE_SYSTEM_CAIRO)
  target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::CAIRO)
else()
  target_link_libraries(${poppler_lib_name} PUBLIC ${cairo_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
  target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${poppler_lib_name} PUBLIC ${freetype2_dll_name})
endif()

if(USE_SYSTEM_PNG)
  target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::PNG)
else()
  target_link_libraries(${poppler_lib_name} PUBLIC ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${poppler_lib_name} PUBLIC ${zlib_dll_name})
endif()

if(ENABLE_LIBCURL)
  if(USE_SYSTEM_CURL)
    target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::CURL)
  else()
    target_link_libraries(${poppler_lib_name} PUBLIC ${curl_dll_name})
  endif()
endif()

if(ENABLE_LIBJPEG)
  if(USE_SYSTEM_JPEG)
    target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::JPEG)
  else()
    target_link_libraries(${poppler_lib_name} PUBLIC ${jpeg_dll_name})
  endif()
endif()

if(ENABLE_ZLIB)
  if(USE_SYSTEM_ZLIB)
    target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::ZLIB)
  else()
    target_link_libraries(${poppler_lib_name} PUBLIC ${zlib_dll_name})
  endif()
endif()

if(WITH_FONTCONFIGURATION_FONTCONFIG)
  if(USE_SYSTEM_FONTCONFIG)
    target_link_libraries(${poppler_lib_name} PUBLIC MiKTeX::Imported::FONTCONFIG)
  else()
    target_link_libraries(${poppler_lib_name} PUBLIC ${fontconfig_dll_name})
  endif()
endif()
