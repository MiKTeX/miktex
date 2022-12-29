## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${getopt_lib_name} STATIC ${getopt_sources})

set_property(TARGET ${getopt_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${getopt_lib_name}
    PUBLIC
        -DMIKTEX_GETOPT_STATIC
)

target_include_directories(${getopt_lib_name}
    PUBLIC
        ${public_include_directories}
)
