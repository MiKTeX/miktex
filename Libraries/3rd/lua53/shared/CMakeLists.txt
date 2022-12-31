## CMakeLists.txt
##
## Copyright (C) 2010-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${lua53_dll_name}_sources ${liblua_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-liblua.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblua.rc
    )
    list(APPEND ${lua53_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-liblua.rc
    )
endif()

add_library(${lua53_dll_name} SHARED ${${lua53_dll_name}_sources})

set_property(TARGET ${lua53_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${lua53_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

target_compile_definitions(${lua53_dll_name}
    PUBLIC
        ${public_compile_definitions}
)

target_include_directories(${lua53_dll_name}
    PUBLIC
        ${public_include_directories}
)

target_link_libraries(${lua53_dll_name}
    PRIVATE
        ${core_dll_name}
)

if(MIKTEX_UNIX_ALIKE)
    target_link_libraries(${lua53_dll_name}
        PRIVATE
            ${CMAKE_DL_LIBS}
            m
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${lua53_dll_name}
        PRIVATE
            ${utf8wrap_dll_name}
    )
endif()

set_shared_library_version_properties(
    ${lua53_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

install(TARGETS ${lua53_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
