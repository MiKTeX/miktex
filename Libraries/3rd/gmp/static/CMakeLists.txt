## CMakeLists.txt
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set_source_files_properties(
    ${generated_gmp_sources}
    PROPERTIES GENERATED TRUE
)

add_library(${gmp_lib_name} STATIC ${gmp_sources})

add_dependencies(${gmp_lib_name} gen-gmp-sources)

set_property(TARGET ${gmp_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${gmp_lib_name}
    PUBLIC
        -DMIKTEX_GMP_STATIC
)
  
target_include_directories(${gmp_lib_name}
    PUBLIC
        ${public_include_directories}
)
