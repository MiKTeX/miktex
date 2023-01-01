## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${zlib_dll_name}_sources ${zlib_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-zlib.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-zlib.rc
    )
    list(APPEND ${zlib_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-zlib.rc
    )
endif()

add_library(${zlib_dll_name} SHARED ${${zlib_dll_name}_sources})

set_property(TARGET ${zlib_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${zlib_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${zlib_dll_name}
    INTERFACE
        ${interface_definitions}
    PUBLIC
        -DZLIB_DLL
)

target_include_directories(${zlib_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${zlib_dll_name}
    PRIVATE
        ${utf8wrap_dll_name}
)

install(TARGETS ${zlib_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
