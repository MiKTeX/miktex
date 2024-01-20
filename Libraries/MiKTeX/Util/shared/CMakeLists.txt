## CMakeLists.txt
##
## Copyright (C) 2006-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(${util_dll_name}_sources
    ${component_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/util.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/util.rc
    )
    list(APPEND ${util_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/util.rc
    )
    list(APPEND resource_files
        ${CMAKE_CURRENT_BINARY_DIR}/util.rc
    )
endif()

add_library(${util_dll_name} SHARED ${${util_dll_name}_sources})

set_property(TARGET ${util_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${util_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${util_dll_name}
    PUBLIC
        -DMIKTEX_UTIL_SHARED
)

target_include_directories(${util_dll_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${util_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
    target_link_libraries(${util_dll_name} PRIVATE ${fmt_dll_name})
endif()

install(TARGETS ${util_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

source_group(Resources FILES ${resource_files})
source_group(Public FILES ${public_headers})
