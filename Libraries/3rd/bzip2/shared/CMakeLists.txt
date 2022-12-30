## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${bzip2_dll_name}_sources ${bzip2_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-libbz2.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libbz2.rc
    )
    list(APPEND ${bzip2_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-libbz2.rc
        windows/miktex-libbz2.def
    )
endif()

add_library(${bzip2_dll_name} SHARED ${${bzip2_dll_name}_sources})

set_property(TARGET ${bzip2_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${bzip2_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${bzip2_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${bzip2_dll_name}
    PRIVATE
        ${utf8wrap_dll_name}
)

install(TARGETS ${bzip2_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
