## CMakeLists.txt
##
## Copyright (C) 2008-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${uriparser_dll_name}_sources ${uriparser_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-uriparser.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-uriparser.rc
    )
    list(APPEND ${uriparser_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-uriparser.rc
    )
endif()

add_definitions(
    -DURI_LIBRARY_BUILD
)

add_library(${uriparser_dll_name} SHARED ${${uriparser_dll_name}_sources})

set_property(TARGET ${uriparser_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${uriparser_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${uriparser_dll_name}
    PUBLIC
        ${public_include_directories}
)

install(TARGETS ${uriparser_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
