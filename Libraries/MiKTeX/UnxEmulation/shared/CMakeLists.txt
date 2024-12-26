## CMakeLists.txt
##
## Copyright (C) 2011-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${unxemu_dll_name}_sources ${lib_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/unxemu.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/unxemu.rc
    )
    list(APPEND ${unxemu_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/unxemu.rc
    )
endif()

add_library(${unxemu_dll_name} SHARED ${${unxemu_dll_name}_sources})

set_property(TARGET ${unxemu_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${unxemu_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${unxemu_dll_name}
    INTERFACE
        ${interface_definitions}
    PUBLIC
        -DMIKTEX_UNXEMU_SHARED
)

target_include_directories(${unxemu_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${unxemu_dll_name}
    PUBLIC
        ${core_dll_name}
)

install(TARGETS ${unxemu_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
