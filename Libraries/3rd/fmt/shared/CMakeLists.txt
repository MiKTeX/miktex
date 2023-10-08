## CMakeLists.txt
##
## Copyright (C) 2018-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${fmt_dll_name}_sources ${fmt_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-fmt.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-fmt.rc
    )
    list(APPEND ${fmt_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-fmt.rc
    )
endif()

add_library(${fmt_dll_name} SHARED ${${fmt_dll_name}_sources})

set_property(TARGET ${fmt_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${fmt_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${fmt_dll_name}
    INTERFACE
        -DFMT_SHARED
    PRIVATE
        -DFMT_LIB_EXPORT
)

target_include_directories(${fmt_dll_name} PUBLIC ${public_include_directories})

install(TARGETS ${fmt_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
