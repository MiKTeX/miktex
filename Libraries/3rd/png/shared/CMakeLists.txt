## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${png_dll_name}_sources ${png_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-libpng.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libpng.rc
    )
    list(APPEND ${png_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libpng.rc
        )
endif()

add_library(${png_dll_name} SHARED ${${png_dll_name}_sources})

set_property(TARGET ${png_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${png_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

if(WIN32)
    set_target_properties(${png_dll_name}
        PROPERTIES
            DEFINE_SYMBOL PNG_BUILD_DLL)
endif()

target_compile_definitions(${png_dll_name}
    INTERFACE
        ${interface_definitions}
        -DPNG_USE_DLL
)

target_include_directories(${png_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${png_dll_name}
    PRIVATE
        ${zlib_dll_name}
)

install(TARGETS ${png_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
