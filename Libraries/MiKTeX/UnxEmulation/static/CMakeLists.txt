## CMakeLists.txt
##
## Copyright (C) 2011-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${unxemu_lib_name} STATIC ${lib_sources})

set_property(TARGET ${unxemu_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${unxemu_lib_name}
    INTERFACE
        ${interface_definitions}
    PUBLIC
        -DMIKTEX_UNXEMU_STATIC
)

target_include_directories(${unxemu_lib_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${unxemu_lib_name}
    PUBLIC
        ${core_lib_name}
)

source_group(Public FILES ${public_headers})
