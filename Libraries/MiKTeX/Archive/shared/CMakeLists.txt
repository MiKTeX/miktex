## CMakeLists.txt
##
## Copyright (C) 2007-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${archive_dll_name}_sources ${archive_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/archive.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/archive.rc
    )
    list(APPEND ${archive_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/archive.rc
    )
endif()

add_library(${archive_dll_name} SHARED ${${archive_dll_name}_sources})

set_property(TARGET ${archive_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${archive_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${archive_dll_name}
    PUBLIC
        -DMIKTEX_ARCHIVE_SHARED
)

target_include_directories(${archive_dll_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${archive_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
    target_link_libraries(${archive_dll_name} PRIVATE ${fmt_dll_name})
endif()

if(USE_SYSTEM_MSPACK)
    target_link_libraries(${archive_dll_name} PRIVATE MiKTeX::Imported::MSPACK)
else()
    target_link_libraries(${archive_dll_name} PRIVATE ${mspack_dll_name})
endif()

target_link_libraries(${archive_dll_name}
    PRIVATE
        ${core_dll_name}
)

delay_load(${archive_dll_name}
    ${bzip2_dll_name}
    ${mspack_dll_name}
)

install(TARGETS ${archive_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
