## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${app_dll_name}_sources ${app_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        ../app.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/app.rc
    )
    list(APPEND ${app_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/app.rc
    )
endif()

add_library(${app_dll_name} SHARED ${${app_dll_name}_sources})

set_property(TARGET ${app_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${app_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

add_dependencies(${app_dll_name} gen-${MIKTEX_COMP_ID}-sources)

set_source_files_properties(
    ${generated_${MIKTEX_COMP_ID}_sources}
    PROPERTIES
        GENERATED TRUE
)

target_compile_definitions(${app_dll_name}
    PUBLIC
        -DMIKTEX_APP_SHARED
)

target_include_directories(${app_dll_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${app_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
    target_link_libraries(${app_dll_name} PRIVATE ${fmt_dll_name})
endif()

if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(${app_dll_name} PRIVATE MiKTeX::Imported::LOG4CXX)
else()
    target_link_libraries(${app_dll_name} PRIVATE ${log4cxx_dll_name})
endif()

target_link_libraries(${app_dll_name}
    PUBLIC
        ${core_dll_name}
        ${loc_dll_name}
        ${mpm_dll_name}
        ${setup_dll_name}
)

if(WITH_UI_QT)
    target_link_libraries(${app_dll_name}
        PRIVATE
            ${ui_qt_dll_name}
    )
endif()

delay_load(${app_dll_name}
    ${mpm_dll_name}
    ${ui_qt_dll_name}
)

install(TARGETS ${app_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
