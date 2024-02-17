## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${kpsemu_dll_name}_sources ${kpsemu_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/kpsemu.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/kpsemu.rc
    )
    list(APPEND ${kpsemu_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/kpsemu.rc
    )
endif()

add_library(${kpsemu_dll_name} SHARED ${${kpsemu_dll_name}_sources})

set_property(TARGET ${kpsemu_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${kpsemu_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${kpsemu_dll_name}
    PUBLIC
        -DMIKTEX_KPSEMU_SHARED
)

target_include_directories(${kpsemu_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${kpsemu_dll_name}
    PRIVATE
        ${app_dll_name}
        ${texmf_dll_name}
)

target_link_libraries(${kpsemu_dll_name}
    INTERFACE
        ${core_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${kpsemu_dll_name}
        PUBLIC
        ${getopt_dll_name}
        INTERFACE
        ${unxemu_dll_name}
    )
endif()

install(TARGETS ${kpsemu_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
