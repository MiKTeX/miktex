## CMakeLists.txt
##
## Copyright (C) 2015-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set(${libressl_crypto_dll_name}_sources ${crypto_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-libressl-crypto.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libressl-crypto.rc
    )
    list(APPEND ${libressl_crypto_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libressl-crypto.rc
    )
endif()

add_library(${libressl_crypto_dll_name} SHARED ${${libressl_crypto_dll_name}_sources})

prevent_all_warnings_on_bad_code_target(${libressl_crypto_dll_name})

set_property(TARGET ${libressl_crypto_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${libressl_crypto_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${libressl_crypto_dll_name}
    PUBLIC
        ${public_include_directories}
)

if (MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${libressl_crypto_dll_name}
        PRIVATE
            Bcrypt.lib
            Ws2_32.lib
    )
endif()

install(TARGETS ${libressl_crypto_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
