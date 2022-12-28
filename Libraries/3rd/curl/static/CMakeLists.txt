## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${curl_lib_name} STATIC ${curl_sources})

set_property(TARGET ${curl_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${curl_lib_name}
    PUBLIC
        -DCURL_STATICLIB
)

target_include_directories(${curl_lib_name}
    PUBLIC
        ${public_include_directories}
)

if (MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${curl_lib_name}
        PUBLIC
            crypt32
            winmm
            wsock32
            ws2_32
            Wldap32
    )
endif()
