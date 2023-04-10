## luamisc.cmake
##
## Copyright (C) 2010-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(luamisc_sources
    source/luafilesystem/src/lfs.c
    source/luafilesystem/src/lfs.h
    source/luamd5/luamd5.h
    source/luamd5/md5.c
    source/luamd5/md5lib.c
    source/luapeg/lpeg.c
    source/luapeg/lpeg.h
    source/luazip/src/luazip.c
    source/luazip/src/luazip.h
    source/luazlib/lgzip.c
    source/luazlib/lzlib.c
    source/slnunicode/slnunico.c
)

add_library(luatex-luamisc-objects OBJECT ${luamisc_sources})

set_property(TARGET luatex-luamisc-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(luatex-luamisc-objects
    PRIVATE
        -DLUAZIP_API=
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(luatex-luamisc-objects PUBLIC MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(luatex-luamisc-objects PUBLIC ${zlib_dll_name})
endif()

if(USE_SYSTEM_ZZIP)
    target_link_libraries(luatex-luamisc-objects PUBLIC MiKTeX::Imported::ZZIP)
else()
    target_link_libraries(luatex-luamisc-objects PUBLIC ${zzip_dll_name})
endif()

target_link_libraries(luatex-luamisc-objects
    PUBLIC
        ${core_dll_name}
        ${lua53_target_name}
        ${pplib_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(luatex-luamisc-objects
        PUBLIC
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()
