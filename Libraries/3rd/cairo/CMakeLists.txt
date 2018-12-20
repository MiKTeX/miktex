## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2018 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/cairo")

set(HAVE_UINT64_T TRUE)

set(HAVE_FT_DONE_MM_VAR TRUE)
set(HAVE_FT_GET_VAR_DESIGN_COORDINATES TRUE)
set(HAVE_FT_GET_X11_FONT_FORMAT TRUE)
set(HAVE_FT_GLYPHSLOT_EMBOLDEN TRUE)
set(HAVE_FT_GLYPHSLOT_OBLIQUE TRUE)
set(HAVE_FT_LIBRARY_SETLCDFILTER TRUE)
set(HAVE_FT_LOAD_SFNT_TABLE TRUE)

set(CAIRO_HAS_XLIB_SURFACE FALSE)
set(CAIRO_HAS_XLIB_XRENDER_SURFACE FALSE)
set(CAIRO_HAS_XCB_SURFACE FALSE)
set(CAIRO_HAS_XLIB_XCB_FUNCTIONS FALSE)
set(CAIRO_HAS_XCB_SHM_FUNCTIONS FALSE)
set(CAIRO_HAS_QT_SURFACE FALSE)
set(CAIRO_HAS_QUARTZ_SURFACE FALSE)
set(CAIRO_HAS_QUARTZ_FONT FALSE)
set(CAIRO_HAS_QUARTZ_IMAGE_SURFACE FALSE)
set(CAIRO_HAS_WIN32_SURFACE ${MIKTEX_NATIVE_WINDOWS})
set(CAIRO_HAS_WIN32_FONT ${MIKTEX_NATIVE_WINDOWS})
set(CAIRO_HAS_OS2_SURFACE FALSE)
set(CAIRO_HAS_BEOS_SURFACE FALSE)
set(CAIRO_HAS_DRM_SURFACE FALSE)
set(CAIRO_HAS_GALLIUM_SURFACE FALSE)
set(CAIRO_HAS_PNG_FUNCTIONS TRUE)
set(CAIRO_HAS_GL_SURFACE FALSE)
set(CAIRO_HAS_GLESV2_SURFACE FALSE)
set(CAIRO_HAS_GLESV3_SURFACE FALSE)
set(CAIRO_HAS_COGL_SURFACE FALSE)
set(CAIRO_HAS_DIRECTFB_SURFACE FALSE)
set(CAIRO_HAS_VG_SURFACE FALSE)
set(CAIRO_HAS_EGL_FUNCTIONS FALSE)
set(CAIRO_HAS_GLX_FUNCTIONS FALSE)
set(CAIRO_HAS_WGL_FUNCTIONS FALSE)
set(CAIRO_HAS_SCRIPT_SURFACE TRUE)
set(CAIRO_HAS_FT_FONT TRUE)
set(CAIRO_HAS_FC_FONT TRUE)
set(CAIRO_HAS_PS_SURFACE TRUE)
set(CAIRO_HAS_PDF_SURFACE TRUE)
set(CAIRO_HAS_SVG_SURFACE TRUE)
set(CAIRO_HAS_TEST_SURFACES FALSE)
set(CAIRO_HAS_IMAGE_SURFACE TRUE)
set(CAIRO_HAS_MIME_SURFACE TRUE)
set(CAIRO_HAS_RECORDING_SURFACE TRUE)
set(CAIRO_HAS_OBSERVER_SURFACE TRUE)
set(CAIRO_HAS_TEE_SURFACE FALSE)
set(CAIRO_HAS_XML_SURFACE FALSE)
set(CAIRO_HAS_USER_FONT TRUE)
set(CAIRO_HAS_PTHREAD ${MIKTEX_UNIX_ALIKE})
set(CAIRO_HAS_GOBJECT_FUNCTIONS FALSE)
set(CAIRO_HAS_TRACE FALSE)
set(CAIRO_HAS_INTERPRETER TRUE)
set(CAIRO_HAS_SYMBOL_LOOKUP FALSE)

include(features.cmake)

set(public_headers
  ${enabled_cairo_headers}
  source/cairo-version.h
)

foreach(h ${public_headers})
  get_filename_component(fn ${h} NAME)
  configure_file(
    ${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${fn}
    COPYONLY
  )
endforeach()

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  cairo-features.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/cairo-features.h
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

configure_file(
  miktex-cairo-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-cairo-version.h
)

set(${cairo_dll_name}_sources
  ${CMAKE_CURRENT_BINARY_DIR}/miktex-cairo-version.h
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${enabled_cairo_sources}
  ${cairo_headers}
  ${cairo_private}
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    cairo.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/cairo.rc
  )
  list(APPEND ${cairo_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/cairo.rc
  )
endif()

add_library(${cairo_dll_name} SHARED ${${cairo_dll_name}_sources})

third_party_target_compile_options(${cairo_dll_name})

set_property(TARGET ${cairo_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version(
  ${cairo_dll_name}
  ${MIKTEX_COMP_INTERFACE_VERSION}
  ${MIKTEX_COMP_J2000_VERSION}
)

target_include_directories(${cairo_dll_name}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  SYSTEM INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${cairo_dll_name}
  PRIVATE
    Threads::Threads
)

if(USE_SYSTEM_FONTCONFIG)
  target_link_libraries(${cairo_dll_name} PUBLIC MiKTeX::Imported::FONTCONFIG)
else()
  target_link_libraries(${cairo_dll_name} PUBLIC ${fontconfig_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
  target_link_libraries(${cairo_dll_name} PUBLIC MiKTeX::Imported::FREETYPE2)
else()
  target_link_libraries(${cairo_dll_name} PUBLIC ${freetype2_dll_name})
endif()

if(USE_SYSTEM_PIXMAN)
  target_link_libraries(${cairo_dll_name} PRIVATE MiKTeX::Imported::PIXMAN)
else()
  target_link_libraries(${cairo_dll_name} PRIVATE ${pixman_dll_name})
endif()

if(USE_SYSTEM_PNG)
  target_link_libraries(${cairo_dll_name} PRIVATE MiKTeX::Imported::PNG)
else()
  target_link_libraries(${cairo_dll_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${cairo_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${cairo_dll_name} PRIVATE ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${cairo_dll_name}
    PRIVATE
      gdi32.lib
      msimg32.lib
      user32.lib
  )
endif()

#delay_load(${cairo_dll_name}
#)

install(TARGETS ${cairo_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
