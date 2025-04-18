## CMakeLists.txt
##
## Copyright (C) 2015-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${apr_dll_name}_sources ${apr_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-apr.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-apr.rc
    )
    list(APPEND ${apr_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-apr.rc
    )
endif()

add_library(${apr_dll_name} SHARED ${${apr_dll_name}_sources})

set_property(TARGET ${apr_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_source_files_properties(
    ${generated_headers}
    PROPERTIES GENERATED TRUE
)

add_dependencies(${apr_dll_name} gen-apr-headers)

set_shared_library_version_properties(
    ${apr_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${apr_dll_name}
    PRIVATE
        -DAPR_DECLARE_EXPORT
)

target_include_directories(${apr_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${apr_dll_name}
    PRIVATE
        ${system_libraries}
)

install(TARGETS ${apr_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
