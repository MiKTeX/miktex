## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${bzip2_lib_name} STATIC ${bzip2_sources})

set_property(TARGET ${bzip2_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${bzip2_lib_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${bzip2_lib_name}
    PUBLIC
        ${utf8wrap_lib_name}
)
