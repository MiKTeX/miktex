## sources.cmake
##
## Copyright (C) 2001-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

configure_file(
    miktex-expat-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-expat-version.h
)

set(expat_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/ascii.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/asciitab.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/expat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/expat_external.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/iasciitab.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/latin1tab.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/nametab.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/siphash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/utf8tab.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmlparse.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmlrole.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmlrole.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmltok.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmltok.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lib/xmltok_impl.h

    ${CMAKE_CURRENT_BINARY_DIR}/miktex-expat-version.h
)

configure_file(
    source/expat_config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/expat_config.h
)

list(APPEND expat_sources
    ${CMAKE_CURRENT_BINARY_DIR}/expat_config.h
)
