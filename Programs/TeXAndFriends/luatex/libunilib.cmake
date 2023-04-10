## libunilib.cmake
##
## Copyright (C) 2010-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.
## .cmake

set(unilib_sources
    source/unilib/ArabicForms.c
    source/unilib/alphabet.c
    source/unilib/char.c
    source/unilib/chardata.h
    source/unilib/charset.h
    source/unilib/cjk.c
    source/unilib/combiners.h
    source/unilib/gwwiconv.c
    source/unilib/gwwiconv.h
    source/unilib/ucharmap.c
    source/unilib/unialt.c
    source/unilib/unibasics.h
    source/unilib/usprintf.c
    source/unilib/ustring.c
    source/unilib/ustring.h
    source/unilib/utype.c
    source/unilib/utype.h
)

add_library(luatex-unilib-objects OBJECT ${unilib_sources})

set_property(TARGET luatex-unilib-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-unilib-objects
    PUBLIC
        source/unilib
)

target_link_libraries(luatex-unilib-objects
    PUBLIC
        ${core_dll_name}
        ${kpsemu_dll_name}
        ${w2cemu_dll_name}
)
