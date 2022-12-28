## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${curl_dll_name}_sources ${curl_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-curl.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-curl.rc
    )
    list(APPEND ${curl_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-curl.rc
    )
endif()

add_library(${curl_dll_name} SHARED ${${curl_dll_name}_sources})

set_property(TARGET ${curl_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${curl_dll_name}
    PUBLIC
        ${public_include_directories}
)

set_shared_library_version_properties(
    ${curl_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${curl_dll_name}
        PRIVATE
            crypt32
            winmm
            wsock32
            ws2_32
            Wldap32
    )
endif()

install(TARGETS ${curl_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
