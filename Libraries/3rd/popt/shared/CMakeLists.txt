## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${popt_dll_name}_sources ${popt_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-popt.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-popt.rc
    )
    list(APPEND ${popt_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-popt.rc
    )
endif()

add_library(${popt_dll_name} SHARED ${${popt_dll_name}_sources})

set_property(TARGET ${popt_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${popt_dll_name}
    PUBLIC
        -DMIKTEX_POPT_SHARED
)

target_include_directories(${popt_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${popt_dll_name}
    PRIVATE
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
)

set_shared_library_version_properties(
    ${popt_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

install(TARGETS ${popt_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
