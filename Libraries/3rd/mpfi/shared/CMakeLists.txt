## CMakeLists.txt
##
## Copyright (C) 2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${mpfi_dll_name}_sources ${mpfi_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-mpfi.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-mpfi.rc
    )
    list(APPEND ${mpfi_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-mpfi.rc
    )
endif()

add_library(${mpfi_dll_name} SHARED ${${mpfi_dll_name}_sources})

set_property(TARGET ${mpfi_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${mpfi_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version_properties(
    ${mpfi_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${mpfi_dll_name} PUBLIC ${public_include_directories})

if(USE_SYSTEM_GMP)
    target_link_libraries(${mpfi_dll_name} PUBLIC MiKTeX::Imported::GMP)
else()
    target_link_libraries(${mpfi_dll_name} PUBLIC ${gmp_lib_name})
endif()

if(USE_SYSTEM_MPFR)
    target_link_libraries(${mpfi_dll_name} PUBLIC MiKTeX::Imported::MPFR)
else()
    target_link_libraries(${mpfi_dll_name} PUBLIC ${mpfr_lib_name})
endif()

install(TARGETS ${mpfi_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
