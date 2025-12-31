## CMakeLists.txt
##
## Copyright (C) 2006-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${archive_lib_name} STATIC ${archive_sources})

set_property(TARGET ${archive_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${archive_lib_name}
    PUBLIC
        -DMIKTEX_ARCHIVE_STATIC
)

target_include_directories(${archive_lib_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_BZIP2)
    target_link_libraries(${archive_lib_name} PUBLIC MiKTeX::Imported::BZIP2)
else()
    target_link_libraries(${archive_lib_name} PUBLIC ${bzip2_lib_name})
endif()

if(USE_SYSTEM_FMT)
    target_link_libraries(${archive_lib_name} PUBLIC MiKTeX::Imported::FMT)
else()
    target_link_libraries(${archive_lib_name} PUBLIC ${fmt_lib_name})
endif()

if(USE_SYSTEM_MSPACK)
    target_link_libraries(${archive_lib_name} PUBLIC MiKTeX::Imported::MSPACK)
else()
    target_link_libraries(${archive_lib_name} PUBLIC ${mspack_lib_name})
endif()

target_link_libraries(${archive_lib_name}
    PUBLIC
        ${core_lib_name}   
)

source_group(Public FILES ${public_headers})
