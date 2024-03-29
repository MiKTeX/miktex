## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${texmf_dll_name}_sources ${texmf_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        ../texmf.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/texmf.rc
    )
    list(APPEND ${texmf_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/texmf.rc
    )
endif()

add_library(${texmf_dll_name} SHARED ${${texmf_dll_name}_sources})

set_property(TARGET ${texmf_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${texmf_dll_name} gen-${MIKTEX_COMP_ID}-sources)

set_source_files_properties(
    ${generated_${MIKTEX_COMP_ID}_sources}
    PROPERTIES
        GENERATED TRUE
)

add_dependencies(${texmf_dll_name} gen-defaults)

set_source_files_properties(
    ${generated_texmf_sources}
    PROPERTIES
        GENERATED TRUE
)

set_shared_library_version_properties(
    ${texmf_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${texmf_dll_name}
    PUBLIC
        -DMIKTEX_TEXMF_SHARED
)

target_include_directories(${texmf_dll_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_BZIP2)
    target_link_libraries(${texmf_dll_name} PRIVATE MiKTeX::Imported::BZIP2)
else()
    target_link_libraries(${texmf_dll_name} PRIVATE ${bzip2_dll_name})
endif()

if(USE_SYSTEM_FMT)
    target_link_libraries(${texmf_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
    target_link_libraries(${texmf_dll_name} PRIVATE ${fmt_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${texmf_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${texmf_dll_name} PRIVATE ${zlib_dll_name})
endif()

target_link_libraries(${texmf_dll_name}
    PUBLIC
        ${app_dll_name}
        ${core_dll_name}
        ${loc_dll_name}
        miktex-popt-wrapper
)

install(TARGETS ${texmf_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)

source_group(Public/C4P FILES ${public_headers_c4p})
source_group(Public/TEXMF FILES ${public_headers_texmf})
source_group(Public/noext FILES ${public_headers_no_ext})
