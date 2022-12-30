## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${lzma_dll_name}_sources ${liblzma_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-liblzma.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblzma.rc
    )
    list(APPEND ${lzma_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblzma.rc
    )
endif()

add_library(${lzma_dll_name} SHARED ${${lzma_dll_name}_sources})

set_property(TARGET ${lzma_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${lzma_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${lzma_dll_name}
    PUBLIC
        -DDLL_EXPORT
)

target_include_directories(${lzma_dll_name}
    PUBLIC
        ${public_include_directories}
)

install(TARGETS ${lzma_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
