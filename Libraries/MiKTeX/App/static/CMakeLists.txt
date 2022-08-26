## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${app_lib_name} STATIC ${app_sources})

set_property(TARGET ${app_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${app_lib_name} gen-${MIKTEX_COMP_ID}-sources)

set_source_files_properties(
    ${generated_${MIKTEX_COMP_ID}_sources}
    PROPERTIES
        GENERATED TRUE
)

target_compile_definitions(${app_lib_name}
    PUBLIC
        -DMIKTEX_APP_STATIC
)

target_include_directories(${app_lib_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(${app_lib_name} PUBLIC MiKTeX::Imported::FMT)
else()
    target_link_libraries(${app_lib_name} PUBLIC ${fmt_lib_name})
endif()

if(USE_SYSTEM_LOG4CXX)
    target_link_libraries(${app_lib_name} PUBLIC MiKTeX::Imported::LOG4CXX)
else()
    target_link_libraries(${app_lib_name} PUBLIC ${log4cxx_lib_name})
endif()

target_link_libraries(${app_lib_name}
    PUBLIC
        ${core_lib_name}
        ${loc_lib_name}
        ${mpm_lib_name}
        ${setup_lib_name}
)

if(INSTALL_STATIC_LIBRARIES)
    install(TARGETS ${app_lib_name}
        RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
        LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
        ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    )
endif()

source_group(Public FILES ${public_headers})
