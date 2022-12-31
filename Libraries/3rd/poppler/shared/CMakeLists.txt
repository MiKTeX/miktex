## CMakeLists.txt
##
## Copyright (C) 2017-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(${poppler_dll_name}_sources ${poppler_sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-poppler.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-poppler.rc
    )
    list(APPEND ${poppler_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-poppler.rc
    )
endif()

add_library(${poppler_dll_name} SHARED ${${poppler_dll_name}_sources})

set_property(TARGET ${poppler_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(NOT POPPLER_VERSION VERSION_LESS 0.59.0)
    set_target_properties(${poppler_dll_name}
        PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS MIKTEX_POPPLER_59=1
    )
endif()

set_property(TARGET ${poppler_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

set_shared_library_version_properties(
    ${poppler_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${poppler_dll_name} PUBLIC ${public_include_directories})

target_link_libraries(${poppler_dll_name}
    PRIVATE
        ${core_dll_name}
        ${tiff_dll_name}
        Threads::Threads
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${poppler_dll_name}
        PRIVATE
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

if(USE_SYSTEM_CAIRO)
    target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::CAIRO)
else()
    target_link_libraries(${poppler_dll_name} PRIVATE ${cairo_dll_name})
endif()

if(USE_SYSTEM_FREETYPE2)
    target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::FREETYPE2)
else()
    target_link_libraries(${poppler_dll_name} PRIVATE ${freetype2_dll_name})
endif()

if(USE_SYSTEM_PNG)
    target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::PNG)
else()
    target_link_libraries(${poppler_dll_name} PRIVATE ${png_dll_name})
endif()

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${poppler_dll_name} PRIVATE ${zlib_dll_name})
endif()

if(ENABLE_LIBCURL)
    if(USE_SYSTEM_CURL)
        target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::CURL)
    else()
        target_link_libraries(${poppler_dll_name} PRIVATE ${curl_dll_name})
    endif()
endif()

if(ENABLE_LIBJPEG)
    if(USE_SYSTEM_JPEG)
        target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::JPEG)
    else()
        target_link_libraries(${poppler_dll_name} PRIVATE ${jpeg_dll_name})
    endif()
endif()

if(ENABLE_ZLIB)
    if(USE_SYSTEM_ZLIB)
        target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
    else()
        target_link_libraries(${poppler_dll_name} PRIVATE ${zlib_dll_name})
    endif()
endif()

if(WITH_FONTCONFIGURATION_FONTCONFIG)
    if(USE_SYSTEM_FONTCONFIG)
        target_link_libraries(${poppler_dll_name} PRIVATE MiKTeX::Imported::FONTCONFIG)
    else()
        target_link_libraries(${poppler_dll_name} PRIVATE ${fontconfig_dll_name})
    endif()
endif()

install(TARGETS ${poppler_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
