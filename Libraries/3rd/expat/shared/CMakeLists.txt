## CMakeLists.txt
##
## Copyright (C) 2001-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${expat_dll_name}_sources ${expat_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-expat.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/expat.rc
    )
    list(APPEND ${expat_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/expat.rc
        windows/expat.def
    )
endif()

add_library(${expat_dll_name} SHARED ${${expat_dll_name}_sources})

set_property(TARGET ${expat_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${expat_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${expat_dll_name}
    PUBLIC
        ${public_include_directories}
)

install(TARGETS ${expat_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
