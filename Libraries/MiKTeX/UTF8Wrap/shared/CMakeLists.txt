## CMakeLists.txt
##
## Copyright (C) 2011-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${utf8wrap_dll_name}_sources ${lib_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        ../utf8wrap.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/utf8wrap.rc
    )
    list(APPEND ${utf8wrap_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/utf8wrap.rc
    )
endif()

add_library(${utf8wrap_dll_name} SHARED ${${utf8wrap_dll_name}_sources})

set_property(TARGET ${utf8wrap_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${utf8wrap_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${utf8wrap_dll_name}
    PUBLIC
        -DMIKTEX_UTF8WRAP_SHARED
)

target_include_directories(${utf8wrap_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${utf8wrap_dll_name}
    PRIVATE
        ${util_dll_name}
)

install(TARGETS ${utf8wrap_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

source_group(Public FILES ${public_headers})
