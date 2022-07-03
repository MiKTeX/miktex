## CMakeLists.txt
##
## Copyright (C) 2011-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

use_static_crt()

add_library(${utf8wrap_lib_name} STATIC ${lib_sources})

set_property(TARGET ${utf8wrap_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_compile_definitions(${utf8wrap_lib_name}
    PUBLIC
        -DMIKTEX_UTF8WRAP_STATIC
)

target_include_directories(${utf8wrap_lib_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${utf8wrap_lib_name}
    PRIVATE
        ${util_lib_name}
)

set(utf8wrap_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/utf8wrap.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND utf8wrap_sources
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(utf8wrap ${utf8wrap_sources})

set_property(TARGET utf8wrap PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(utf8wrap
    ${utf8wrap_lib_name}
)

install(TARGETS utf8wrap DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

source_group(Public FILES ${public_headers})
