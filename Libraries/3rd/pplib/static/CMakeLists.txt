## CMakeLists.txt
##
## Copyright (C) 2020-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_library(${pplib_lib_name} STATIC ${pplib_sources})

set_property(TARGET ${pplib_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${pplib_lib_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${pplib_lib_name} PUBLIC MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${pplib_lib_name} PUBLIC ${zlib_dll_name})
endif()

target_link_libraries(${pplib_lib_name}
    PUBLIC
        ${core_dll_name}
        ${lua53_target_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${pplib_lib_name}
        PUBLIC
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()
