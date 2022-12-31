## CMakeLists.txt
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

add_library(${mpfr_lib_name} STATIC ${mpfr_sources})

set_property(TARGET ${mpfr_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(USE_SYSTEM_GMP)
    target_link_libraries(${mpfr_lib_name} PUBLIC MiKTeX::Imported::GMP)
else()
    target_link_libraries(${mpfr_lib_name} PUBLIC ${gmp_lib_name})
endif()

target_include_directories(${mpfr_lib_name}
    PUBLIC
        ${public_include_directories}
)
