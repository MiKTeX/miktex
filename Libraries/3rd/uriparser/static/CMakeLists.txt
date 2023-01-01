## CMakeLists.txt
##
## Copyright (C) 2008-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${uriparser_lib_name} STATIC ${uriparser_sources})

set_property(TARGET ${uriparser_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${uriparser_lib_name}
    PUBLIC
        -DURI_STATIC_BUILD
)

target_include_directories(${uriparser_lib_name}
    PUBLIC
        ${public_include_directories}
)
