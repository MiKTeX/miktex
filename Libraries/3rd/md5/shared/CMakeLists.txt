## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${md5_dll_name}_sources ${md5_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-md5.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-md5.rc
    )
    list(APPEND ${md5_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-md5.rc
    )
endif()

add_library(${md5_dll_name} SHARED ${${md5_dll_name}_sources})

set_property(TARGET ${md5_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version(
    ${md5_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${md5_dll_name}
    PUBLIC
        -DMIKTEX_MD5_SHARED
)

target_include_directories(${md5_dll_name}
    PUBLIC
        ${public_include_directories}
)

install(TARGETS ${md5_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
