## CMakeLists.txt
##
## Copyright (C) 2015-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${libressl_crypto_lib_name} STATIC ${crypto_sources})

prevent_all_warnings_on_bad_code_target(${libressl_crypto_lib_name})

set_property(TARGET ${libressl_crypto_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${libressl_crypto_lib_name}
    PUBLIC
        ${public_include_directories}
)

if (MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${libressl_crypto_lib_name}
        PUBLIC
            Ws2_32.lib
    )
endif()
