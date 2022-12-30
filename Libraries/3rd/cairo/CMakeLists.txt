## CMakeLists.txt
##
## Copyright (C) 2013-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/cairo")

set(proj_binary_dir ${CMAKE_CURRENT_BINARY_DIR})
set(proj_source_dir ${CMAKE_CURRENT_SOURCE_DIR})

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
    list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/include/${fn})
endforeach()

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/config.h)

configure_file(
    cairo-features.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/cairo-features.h
)

list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/include/cairo-features.h)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

configure_file(
    miktex-cairo-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-cairo-version.h
)

list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/miktex-cairo-version.h)

set(cairo_sources
    ${cairo_headers}
    ${cairo_private}
    ${configured_headers}
    ${enabled_cairo_sources}
)

add_subdirectory(shared)
