## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${ptexenc_lib_name} STATIC ${ptexenc_sources})

set_property(TARGET ${ptexenc_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${ptexenc_lib_name}
    PRIVATE
        -DNO_PTENC_DLL
)

target_include_directories(${ptexenc_lib_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${ptexenc_lib_name}
    PRIVATE
        ${kpsemu_lib_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${ptexenc_lib_name}
        PRIVATE
            ${utf8wrap_lib_name}
    )
endif()
