## CMakeLists.txt
##
## Copyright (C) 2020-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${glm_lib_name} STATIC ${glm_sources})

set_property(TARGET ${glm_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${glm_lib_name}
    PUBLIC
        ${public_include_directories}
)
