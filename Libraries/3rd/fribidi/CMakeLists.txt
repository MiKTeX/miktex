## CMakeLists.txt
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/fribidi")

include_directories(BEFORE  
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}

    ${CMAKE_CURRENT_BINARY_DIR}/include/fribidi
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/charset
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib
)

set(FRIBIDI_CHARSETS 1)
set(FRIBIDI_USE_GLIB 0)
set(SIZEOF_INT 4)
set(HAVE_STRINGIZE 1)

configure_file(
    source/lib/fribidi-config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/fribidi/fribidi-config.h
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
    lib/fribidi-arabic.h
    lib/fribidi-begindecls.h
    lib/fribidi-bidi-types-list.h
    lib/fribidi-bidi-types.h
    lib/fribidi-bidi.h
    lib/fribidi-brackets.h
    lib/fribidi-char-sets-list.h
    lib/fribidi-char-sets.h
    lib/fribidi-common.h
    lib/fribidi-deprecated.h
    lib/fribidi-enddecls.h
    lib/fribidi-flags.h
    lib/fribidi-joining-types-list.h
    lib/fribidi-joining-types.h
    lib/fribidi-joining.h
    lib/fribidi-mirroring.h
    lib/fribidi-shape.h
    lib/fribidi-types.h
    lib/fribidi-unicode-version.h
    lib/fribidi-unicode.h
    lib/fribidi.h
)

foreach(hdr ${public_headers})
    get_filename_component(h ${hdr} NAME)
    configure_file(
        source/${hdr}
        ${CMAKE_CURRENT_BINARY_DIR}/include/fribidi/${h}
        COPYONLY
    )
endforeach()

configure_file(
    miktex-fribidi-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-fribidi-version.h
)

include(sources.cmake)

add_subdirectory(static)
