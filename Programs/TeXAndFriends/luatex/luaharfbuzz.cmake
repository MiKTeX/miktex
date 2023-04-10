## luaharfbuzz.cmake
##
## Copyright (C) 2019-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(luaharfbuzz_sources
    source/luaharfbuzz/src/luaharfbuzz/blob.c
    source/luaharfbuzz/src/luaharfbuzz/buffer.c
    source/luaharfbuzz/src/luaharfbuzz/class_utils.c
    source/luaharfbuzz/src/luaharfbuzz/direction.c
    source/luaharfbuzz/src/luaharfbuzz/face.c
    source/luaharfbuzz/src/luaharfbuzz/feature.c
    source/luaharfbuzz/src/luaharfbuzz/font.c
    source/luaharfbuzz/src/luaharfbuzz/language.c
    source/luaharfbuzz/src/luaharfbuzz/luaharfbuzz.c
    source/luaharfbuzz/src/luaharfbuzz/luaharfbuzz.h
    source/luaharfbuzz/src/luaharfbuzz/ot.c
    source/luaharfbuzz/src/luaharfbuzz/script.c
    source/luaharfbuzz/src/luaharfbuzz/tag.c
    source/luaharfbuzz/src/luaharfbuzz/unicode.c
    source/luaharfbuzz/src/luaharfbuzz/variation.c
)

add_library(luahbtex-luaharfbuzz-objects OBJECT ${luaharfbuzz_sources})

set_property(TARGET luahbtex-luaharfbuzz-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_GRAPHITE2)
    target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC MiKTeX::Imported::GRAPHITE2)
else()
    target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC ${graphite2_dll_name})
endif()

if(USE_SYSTEM_HARFBUZZ_ICU)
    target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC MiKTeX::Imported::HARFBUZZ_ICU)
    target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC MiKTeX::Imported::HARFBUZZ)
else()
    target_link_libraries(luahbtex-luaharfbuzz-objects PUBLIC ${harfbuzz_dll_name})
endif()

target_link_libraries(luahbtex-luaharfbuzz-objects
    PUBLIC
        ${core_dll_name}
        ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(luahbtex-luaharfbuzz-objects
        PUBLIC
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()
