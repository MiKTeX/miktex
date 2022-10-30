## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${ptexenc_dll_name}_sources ${ptexenc_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        miktex-ptexenc.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptexenc.rc
    )
    list(APPEND ${ptexenc_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-ptexenc.rc
    )
endif()

add_library(${ptexenc_dll_name} SHARED ${${ptexenc_dll_name}_sources})

set_property(TARGET ${ptexenc_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${ptexenc_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_compile_definitions(${ptexenc_dll_name}
    PRIVATE
        -DMAKE_PTENC_DLL
)

target_include_directories(${ptexenc_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${ptexenc_dll_name}
    PRIVATE
        ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${ptexenc_dll_name}
        PRIVATE
            ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${ptexenc_dll_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)
