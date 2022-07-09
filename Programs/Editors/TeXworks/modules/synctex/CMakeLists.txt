## CMakeLists.txt
##
## Copyright (C) 2019-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_EDITORS_FOLDER}/TeXworks/synctex")

set(synctex_sources
    ${projdir}/source/modules/synctex/synctex_parser.c
    ${projdir}/source/modules/synctex/synctex_parser_utils.c
    ${projdir}/source/modules/synctex/synctex_parser.h
    ${projdir}/source/modules/synctex/synctex_parser_advanced.h
    ${projdir}/source/modules/synctex/synctex_parser_utils.h
    ${projdir}/source/modules/synctex/synctex_version.h
)

add_library(synctex-static STATIC
    ${synctex_sources}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(synctex-static PUBLIC MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(synctex-static PUBLIC ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(synctex-static PUBLIC ${utf8wrap_dll_name})
endif()

set_property(TARGET synctex-static PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(synctex-static
    PUBLIC
        ${projdir}/source/modules/synctex
)
