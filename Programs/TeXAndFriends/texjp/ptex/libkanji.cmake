## libkanji.cmake
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(libkanji_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji_dump.c
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/kanji_dump.c
    PROPERTIES LANGUAGE CXX
)

add_library(texjp-kanji STATIC ${libkanji_sources})

target_include_directories(texjp-kanji
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(texjp-kanji
    PUBLIC
        ${ptexenc_dll_name}
        ${w2cemu_dll_name}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(texjp-kanji PUBLIC MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(texjp-kanji PUBLIC ${zlib_dll_name})
endif()

target_link_libraries(texjp-kanji
    PRIVATE
        ${web2c_sources_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(texjp-kanji
        PRIVATE
            ${utf8wrap_dll_name}
    )
endif()
