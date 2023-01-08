## CMakeLists.txt
##
## Copyright (C) 2006-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_library(${core_ps_dll_name} SHARED ${core_ps_sources})

set_property(TARGET ${core_ps_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${core_ps_dll_name} gen-core-com)

set_source_files_properties(
    ${generated_core_ps_sources}
    PROPERTIES
        GENERATED TRUE
)

target_compile_definitions(${core_ps_dll_name}
    PRIVATE
        -DREGISTER_PROXY_DLL
)

target_include_directories(${core_ps_dll_name}
    PRIVATE
        ${public_include_directories}
)

target_link_libraries(${core_ps_dll_name}
    PRIVATE
        oleaut32
        rpcns4
        rpcrt4
        uuid
)

install(TARGETS ${core_ps_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
