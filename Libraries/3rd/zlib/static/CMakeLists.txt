## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${zlib_lib_name} STATIC ${zlib_sources})

set_property(TARGET ${zlib_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${zlib_lib_name}
    INTERFACE
        ${interface_definitions}
)

target_include_directories(${zlib_lib_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${zlib_lib_name}
    PUBLIC
        ${utf8wrap_lib_name}
)
