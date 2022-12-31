## CMakeLists.txt
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${pixman_dll_name}_sources ${pixman_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-pixman.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-pixman.rc
    )
    list(APPEND ${pixman_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-pixman.rc
    )
endif()

add_library(${pixman_dll_name} SHARED ${${pixman_dll_name}_sources})

set_property(TARGET ${pixman_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${pixman_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version_properties(
    ${pixman_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${pixman_dll_name} PUBLIC ${public_include_directories})

install(TARGETS ${pixman_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
