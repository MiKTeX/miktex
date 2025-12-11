## CMakeLists.txt
##
## Copyright (C) 2013-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${cairo_dll_name}_sources
    ${cairo_headers}
    ${cairo_sources}
    ${configured_headers}
    ${feature_headers}
    ${feature_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-cairo.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-cairo.rc
    )
    list(APPEND ${cairo_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-cairo.rc
    )
endif()

add_library(${cairo_dll_name} SHARED ${${cairo_dll_name}_sources})

if (MIKTEX_NATIVE_WINDOWS AND MSVC)
  target_compile_definitions(${cairo_dll_name} 
    PRIVATE
        NTDDI_VERSION=NTDDI_WIN10_RS3
  )
endif()

prevent_all_warnings_on_bad_code_target(${cairo_dll_name})

set_property(TARGET ${cairo_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version(
    ${cairo_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${cairo_dll_name}
    PRIVATE
        ${proj_binary_dir}/include
        ${proj_source_dir}/include
    SYSTEM INTERFACE
        ${proj_binary_dir}/include
        ${proj_source_dir}/include
)

target_link_libraries(${cairo_dll_name}
    PRIVATE
        Threads::Threads
)

if(USE_SYSTEM_FONTCONFIG)
    target_link_libraries(${cairo_dll_name} PUBLIC MiKTeX::Imported::FONTCONFIG)
else()
    target_link_libraries(${cairo_dll_name} PUBLIC ${fontconfig_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
    target_link_libraries(${cairo_dll_name} PUBLIC MiKTeX::Imported::FREETYPE2)
else()
    target_link_libraries(${cairo_dll_name} PUBLIC ${freetype2_dll_name})
endif()

if(USE_SYSTEM_PIXMAN)
    target_link_libraries(${cairo_dll_name} PRIVATE MiKTeX::Imported::PIXMAN)
else()
    target_link_libraries(${cairo_dll_name} PRIVATE ${pixman_dll_name})
endif()

if(USE_SYSTEM_PNG)
    target_link_libraries(${cairo_dll_name} PRIVATE MiKTeX::Imported::PNG)
else()
    target_link_libraries(${cairo_dll_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${cairo_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${cairo_dll_name} PRIVATE ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${cairo_dll_name}
        PRIVATE
            gdi32.lib
            msimg32.lib
            user32.lib
    )
endif()

install(TARGETS ${cairo_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
