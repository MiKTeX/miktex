## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${expat_lib_name} STATIC ${expat_sources})

set_property(TARGET ${expat_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${expat_lib_name}
    PUBLIC
        -DXML_STATIC
)

target_include_directories(${expat_lib_name}
    PUBLIC
        ${public_include_directories}
)
