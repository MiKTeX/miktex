## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/gd")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
  -DHAVE_CONFIG_H
)

ignore_warnings()

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DBGDWIN32
  )
endif()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(sources
  source/src/bmp.h
  source/src/gd.c
  source/src/gd.h
  source/src/gd_bmp.c
  source/src/gd_color.c
  source/src/gd_color.h
  source/src/gd_color_map.c
  source/src/gd_color_map.h
  source/src/gd_crop.c
  source/src/gd_filename.c
  source/src/gd_filter.c
  source/src/gd_gd.c
  source/src/gd_gd2.c
  source/src/gd_gif_in.c
  source/src/gd_gif_out.c
  source/src/gd_interpolation.c
  source/src/gd_io.c
  source/src/gd_io.h
  source/src/gd_io_dp.c
  source/src/gd_io_file.c
  source/src/gd_io_ss.c
  source/src/gd_io_stream.cxx
  source/src/gd_io_stream.h
  source/src/gd_jpeg.c
  source/src/gd_matrix.c
  source/src/gd_nnquant.c
  source/src/gd_nnquant.h
  source/src/gd_png.c
  source/src/gd_rotate.c
  source/src/gd_security.c
  source/src/gd_ss.c
  source/src/gd_tga.c
  source/src/gd_tga.h
  source/src/gd_tiff.c
  source/src/gd_topal.c
  source/src/gd_transform.c
  source/src/gd_version.c
  source/src/gd_wbmp.c
  source/src/gd_webp.c
  source/src/gd_xbm.c
  source/src/gdcache.c
  source/src/gdcache.h
  source/src/gdfontg.c
  source/src/gdfontg.h
  source/src/gdfontl.c
  source/src/gdfontl.h
  source/src/gdfontmb.c
  source/src/gdfontmb.h
  source/src/gdfonts.c
  source/src/gdfonts.h
  source/src/gdfontt.c
  source/src/gdfontt.h
  source/src/gdft.c
  source/src/gdfx.c
  source/src/gdfx.h
  source/src/gdhelpers.c
  source/src/gdhelpers.h
  source/src/gdkanji.c
  source/src/gdpp.cxx
  source/src/gdpp.h
  source/src/gdtables.c
  source/src/gdxpm.c
  source/src/jisx0208.h
  source/src/wbmp.c
  source/src/wbmp.h

  gd-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    gd.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/gd.rc
  )
  set(sources ${sources}
    ${CMAKE_CURRENT_BINARY_DIR}/gd.rc
  )
endif()

add_library(${gd_dll_name} SHARED ${sources})

set_property(TARGET ${gd_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_target_properties(${gd_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_SERIES_STR}"
    SOVERSION "1"
)

target_include_directories(${gd_dll_name}
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

if(USE_SYSTEM_FONTCONFIG)
  target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::FONTCONFIG)
else()
  target_link_libraries(${gd_dll_name} PRIVATE ${fontconfig_dll_name})
endif()

if(USE_SYSTEM_FONTCONFIG)
  target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${gd_dll_name} PRIVATE ${freetype2_dll_name})
endif()

if(USE_SYSTEM_JPEG)
  target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::JPEG)
else()
  target_link_libraries(${gd_dll_name} PRIVATE ${jpeg_dll_name})
endif()

if(USE_SYSTEM_PNG)
  target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::PNG)
else()
  target_link_libraries(${gd_dll_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${gd_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${gd_dll_name} PRIVATE ${zlib_dll_name})
endif()

target_link_libraries(${gd_dll_name}
  PRIVATE
    ${utf8wrap_dll_name}
)

delay_load(${gd_dll_name}
  ${fontconfig_dll_name}
  ${freetype2_dll_name}
  ${jpeg_dll_name}
  ${png_dll_name}
  ${utf8wrap_dll_name}
  ${zlib_dll_name}
)

rebase(${gd_dll_name})

install(TARGETS ${gd_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

set(public_headers
  gd.h gdfx.h gd_io.h gdcache.h gdfontg.h gdfontl.h
  gdfontmb.h gdfonts.h gdfontt.h gd_color_map.h gd_errors.h
)

foreach(h ${public_headers})
  configure_file(
    source/src/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()
