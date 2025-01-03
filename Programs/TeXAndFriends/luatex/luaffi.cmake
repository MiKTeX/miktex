## luaffi.cmake
##
## Copyright (C) 2010-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(luaffi_sources
    source/luaffi/call.c
    source/luaffi/call_arm.h
    source/luaffi/call_arm64.h
    source/luaffi/call_arm_hf.h
    source/luaffi/call_x64.h
    source/luaffi/call_x64win.h
    source/luaffi/call_x86.h
    source/luaffi/ctype.c
    source/luaffi/ffi.c
    source/luaffi/ffi.h
    source/luaffi/parser.c
) 

add_library(luatex-luaffi-objects OBJECT ${luaffi_sources})

set_property(TARGET luatex-luaffi-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(luatex-luaffi-objects
    PRIVATE
        source/luaffi/dynasm
)

target_link_libraries(luatex-luaffi-objects
    PUBLIC
        ${CMAKE_DL_LIBS}
        ${lua53_target_name}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-luaffi-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(luatex-luaffi-objects PUBLIC ${zlib_dll_name})
endif()
