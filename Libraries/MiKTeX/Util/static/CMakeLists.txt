## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${util_lib_name} STATIC ${component_sources})

set_property(TARGET ${util_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${util_lib_name}
    PUBLIC
        -DMIKTEX_UTIL_STATIC
)

target_include_directories(${util_lib_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${util_lib_name} PRIVATE MiKTeX::Imported::FMT)
else()
    target_link_libraries(${util_lib_name} PRIVATE ${fmt_lib_name})
endif()

source_group(Public FILES ${public_headers})
