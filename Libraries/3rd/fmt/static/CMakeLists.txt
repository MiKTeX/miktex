## CMakeLists.txt
##
## Copyright (C) 2018-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${fmt_lib_name} STATIC ${fmt_sources})

set_property(TARGET ${fmt_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${fmt_lib_name}
    PUBLIC
        ${public_include_directories}
)
