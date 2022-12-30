## sources.cmake
##
## Copyright (C) 2016-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(fribidi_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/arabic-misc.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/arabic-shaping.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/bidi-type.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/bidi-types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/brackets-type.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/brackets.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-arabic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-bidi-types.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-bidi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-brackets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-cap-rtl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-cap-rtl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-cp1255.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-cp1255.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-cp1256.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-cp1256.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-iso8859-6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-iso8859-6.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-iso8859-8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-iso8859-8.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-utf8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets-utf8.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-char-sets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-deprecated.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-joining-types.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-joining.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-mirroring.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-run.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-shape.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/joining-type.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/joining-types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/mirroring.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/run.h
    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/arabic-shaping.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/bidi-type.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/brackets-type.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/brackets.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/fribidi-unicode-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/joining-type.tab.i
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/mirroring.tab.i
    
    ${CMAKE_CURRENT_BINARY_DIR}/include/fribidi/fribidi-config.h

    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-fribidi-version.h
)
