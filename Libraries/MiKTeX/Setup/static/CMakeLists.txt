## CMakeLists.txt
##
## Copyright (C) 2013-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${setup_lib_name} STATIC ${setup_sources})

set_property(TARGET ${setup_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${setup_lib_name} gen-${MIKTEX_COMP_ID}-sources)

set_source_files_properties(
    ${generated_${MIKTEX_COMP_ID}_sources}
    PROPERTIES
        GENERATED TRUE
)

target_compile_definitions(${setup_lib_name}
    PUBLIC
        -DMIKTEX_SETUP_STATIC
)

target_include_directories(${setup_lib_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${setup_lib_name}
    PUBLIC 
        ${core_lib_name}
        ${loc_lib_name}
        ${mpm_lib_name}
        ${nlohmann_json_lib_name}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${setup_lib_name} PRIVATE MiKTeX::Imported::FMT)
else()
    target_link_libraries(${setup_lib_name} PRIVATE ${fmt_lib_name})
endif()

target_link_libraries(${setup_lib_name}
    PRIVATE 
        ${extractor_lib_name}
)

source_group(Public FILES ${public_headers})
