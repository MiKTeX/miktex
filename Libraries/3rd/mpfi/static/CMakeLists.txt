## CMakeLists.txt
##
## Copyright (C) 2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_library(${mpfi_lib_name} STATIC ${mpfi_sources})

set_property(TARGET ${mpfi_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_GMP)
    target_link_libraries(${mpfi_lib_name} PUBLIC MiKTeX::Imported::GMP)
else()
    target_link_libraries(${mpfi_lib_name} PUBLIC ${gmp_lib_name})
endif()

if(USE_SYSTEM_MPFR)
    target_link_libraries(${mpfi_lib_name} PUBLIC MiKTeX::Imported::USE_SYSTEM_MPFR)
else()
    target_link_libraries(${mpfi_lib_name} PUBLIC ${mpfr_lib_name})
endif()

target_include_directories(${mpfi_lib_name}
    PUBLIC
        ${public_include_directories}
)
