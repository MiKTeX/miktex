## CMakeLists.txt
##
## Copyright (C) 2015-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${apr_util_dll_name}_sources ${apu_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-apr-util.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-apr-util.rc
    )
    list(APPEND ${apr_util_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-apr-util.rc
    )
endif()

add_library(${apr_util_dll_name} SHARED ${${apr_util_dll_name}_sources})

set_property(TARGET ${apr_util_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${apr_util_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${apr_util_dll_name}
    PRIVATE
        -DAPU_DECLARE_EXPORT
)

target_include_directories(${apr_util_dll_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_APR)
    target_link_libraries(${apr_util_dll_name} PRIVATE MiKTeX::Imported::APR)
else()
    target_link_libraries(${apr_util_dll_name} PRIVATE ${apr_dll_name})
endif()

if(USE_SYSTEM_EXPAT)
    target_link_libraries(${apr_util_dll_name} PRIVATE MiKTeX::Imported::EXPAT)
else()
    target_link_libraries(${apr_util_dll_name} PRIVATE ${expat_dll_name})
endif()

install(TARGETS ${apr_util_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
