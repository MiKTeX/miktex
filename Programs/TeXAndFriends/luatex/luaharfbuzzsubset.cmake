## luaharfbuzzsubset.cmake
##
## Copyright (C) 2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(luaharfbuzzsubset_sources
    source/luaharfbuzz/src/luaharfbuzzsubset/luaharfbuzzsubset.c
    source/luaharfbuzz/src/luaharfbuzzsubset/subset-input.c
)

add_library(luahbtex-luaharfbuzzsubset-objects OBJECT ${luaharfbuzzsubset_sources})

set_property(TARGET luahbtex-luaharfbuzzsubset-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_GRAPHITE2)
    target_link_libraries(luahbtex-luaharfbuzzsubset-objects PUBLIC MiKTeX::Imported::GRAPHITE2)
else()
    target_link_libraries(luahbtex-luaharfbuzzsubset-objects PUBLIC ${graphite2_dll_name})
endif()

if(USE_SYSTEM_HARFBUZZ_ICU)
    target_link_libraries(luahbtex-luaharfbuzzsubset-objects PUBLIC MiKTeX::Imported::HARFBUZZ_ICU)
    target_link_libraries(luahbtex-luaharfbuzzsubset-objects PUBLIC MiKTeX::Imported::HARFBUZZ)
else()
    target_link_libraries(luahbtex-luaharfbuzzsubset-objects PUBLIC ${harfbuzz_dll_name})
endif()

target_link_libraries(luahbtex-luaharfbuzzsubset-objects
    PUBLIC
        ${core_dll_name}
        ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(luahbtex-luaharfbuzzsubset-objects
        PUBLIC
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()
