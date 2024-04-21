## CMakeLists.txt
##
## Copyright (C) 2013-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/cairo")

set(proj_binary_dir ${CMAKE_CURRENT_BINARY_DIR})
set(proj_source_dir ${CMAKE_CURRENT_SOURCE_DIR})

check_include_files(d2d1_3.h HAVE_D2D1_3_H)

set(HAS_PIXMAN_GLYPHS TRUE)

set(HAVE_UINT64_T TRUE)

set(CAIRO_HAS_DWRITE_FONT ${MIKTEX_NATIVE_WINDOWS})
set(CAIRO_HAS_FC_FONT TRUE)
set(CAIRO_HAS_FT_FONT TRUE)
set(CAIRO_HAS_IMAGE_SURFACE TRUE)
set(CAIRO_HAS_INTERPRETER TRUE)
set(CAIRO_HAS_MIME_SURFACE TRUE)
set(CAIRO_HAS_OBSERVER_SURFACE TRUE)
set(CAIRO_HAS_PDF_SURFACE TRUE)
set(CAIRO_HAS_PNG_FUNCTIONS TRUE)
set(CAIRO_HAS_PS_SURFACE TRUE)
set(CAIRO_HAS_PTHREAD ${MIKTEX_UNIX_ALIKE})
set(CAIRO_HAS_RECORDING_SURFACE TRUE)
set(CAIRO_HAS_SCRIPT_SURFACE TRUE)
set(CAIRO_HAS_SVG_SURFACE TRUE)
set(CAIRO_HAS_USER_FONT TRUE)
set(CAIRO_HAS_WIN32_FONT ${MIKTEX_NATIVE_WINDOWS})
set(CAIRO_HAS_WIN32_SURFACE ${MIKTEX_NATIVE_WINDOWS})

add_definitions(
    -DCAIRO_COMPILATION
)

include(features.cmake)

set(public_headers
    ${cairo_headers}
    ${feature_headers}
    source/src/cairo-version.h
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

add_subdirectory(shared)
