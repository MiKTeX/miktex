## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${md5_lib_name} STATIC ${md5_sources})

set_property(TARGET ${md5_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${md5_lib_name}
    PUBLIC
        -DMIKTEX_MD5_STATIC
)

target_include_directories(${md5_lib_name}
    PUBLIC
        ${public_include_directories}
)
