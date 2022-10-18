## CMakeLists.txt
##
## Copyright (C) 2009-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${texmf_lib_name} STATIC ${texmf_sources})

set_property(TARGET ${texmf_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${app_lib_name} gen-${MIKTEX_COMP_ID}-sources)

set_source_files_properties(
    ${generated_${MIKTEX_COMP_ID}_sources}
    PROPERTIES
        GENERATED TRUE
)

add_dependencies(${texmf_lib_name} gen-defaults)

set_source_files_properties(
    ${generated_texmf_sources}
    PROPERTIES
        GENERATED TRUE
)

target_compile_definitions(${texmf_lib_name}
    PUBLIC
        -DMIKTEX_TEXMF_STATIC
)

target_include_directories(${texmf_lib_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_BZIP2)
    target_link_libraries(${texmf_lib_name} PUBLIC MiKTeX::Imported::BZIP2)
else()
    target_link_libraries(${texmf_lib_name} PUBLIC ${bzip2_lib_name})
endif()

if(USE_SYSTEM_FMT)
    target_link_libraries(${texmf_lib_name} PUBLIC MiKTeX::Imported::FMT)
else()
    target_link_libraries(${texmf_lib_name} PUBLIC ${fmt_lib_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${texmf_lib_name} PUBLIC MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${texmf_lib_name} PUBLIC ${zlib_lib_name})
endif()

target_link_libraries(${texmf_lib_name}
    PUBLIC
        ${app_lib_name}
        ${core_lib_name}
        ${loc_lib_name}
        miktex-popt-wrapper-static
)

if(INSTALL_STATIC_LIBRARIES)
    install(TARGETS ${texmf_lib_name}
        RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
        LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
        ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    )
endif()

source_group(Public/C4P FILES ${public_headers_c4p})
source_group(Public/TEXMF FILES ${public_headers_texmf})
source_group(Public/noext FILES ${public_headers_no_ext})
