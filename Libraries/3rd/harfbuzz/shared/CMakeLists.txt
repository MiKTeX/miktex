## CMakeLists.txt
##
## Copyright (C) 2013-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${harfbuzz_dll_name}_sources ${harfbuzz_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-harfbuzz.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-harfbuzz.rc
    )
    list(APPEND ${harfbuzz_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-harfbuzz.rc
    )
endif()

add_library(${harfbuzz_dll_name} SHARED ${${harfbuzz_dll_name}_sources})

prevent_all_warnings_on_bad_code_target(${harfbuzz_dll_name})

set_property(TARGET ${harfbuzz_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${harfbuzz_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version_properties(
    ${harfbuzz_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${harfbuzz_dll_name}
    PRIVATE
        ${public_include_directories}
    SYSTEM INTERFACE
        ${public_include_directories}
)

if(USE_SYSTEM_GRAPHITE2)
    target_link_libraries(${harfbuzz_dll_name} PUBLIC MiKTeX::Imported::GRAPHITE2)
else()
    target_link_libraries(${harfbuzz_dll_name} PUBLIC ${graphite2_dll_name})
endif()

install(TARGETS ${harfbuzz_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
