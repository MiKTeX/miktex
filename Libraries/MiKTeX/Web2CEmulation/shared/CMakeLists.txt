## CMakeLists.txt
##
## Copyright (C) 2010-2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${w2cemu_dll_name}_sources ${w2cemu_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/w2cemu.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/w2cemu.rc
    )
    list(APPEND ${w2cemu_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/w2cemu.rc
    )
endif()

add_library(${w2cemu_dll_name} SHARED ${${w2cemu_dll_name}_sources})

set_property(TARGET ${w2cemu_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${w2cemu_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${w2cemu_dll_name}
    PUBLIC
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_W2CEMU_DIR}/include
        ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_W2CEMU_DIR}/include
)

target_link_libraries(${w2cemu_dll_name}
    PUBLIC
        ${core_dll_name}
        ${kpsemu_dll_name}
        ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${w2cemu_dll_name}
        INTERFACE
            ${unxemu_dll_name}
    )
endif()

if(USE_SYSTEM_FMT)
    target_link_libraries(${w2cemu_dll_name} PRIVATE MiKTeX::Imported::FMT)
else()
    target_link_libraries(${w2cemu_dll_name} PRIVATE ${fmt_dll_name})
endif()

install(TARGETS ${w2cemu_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
