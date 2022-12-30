## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${getopt_dll_name}_sources ${getopt_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-getopt.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-getopt.rc
    )
    list(APPEND ${getopt_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-getopt.rc
    )
endif()

add_library(${getopt_dll_name} SHARED ${${getopt_dll_name}_sources})

set_property(TARGET ${getopt_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${getopt_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${getopt_dll_name}
    PUBLIC
        -DMIKTEX_GETOPT_SHARED
)

target_include_directories(${getopt_dll_name}
    PUBLIC
        ${public_include_directories}
)

install(TARGETS ${getopt_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
