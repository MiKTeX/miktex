## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${popt_lib_name} STATIC ${popt_sources})

set_property(TARGET ${popt_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${popt_lib_name}
    PUBLIC
        -DMIKTEX_POPT_STATIC
)

target_include_directories(${popt_lib_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${popt_lib_name}
    PUBLIC
        ${unxemu_lib_name}
        ${utf8wrap_lib_name}
)
