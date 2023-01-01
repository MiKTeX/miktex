## CMakeLists.txt
##
## Copyright (C) 2007-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/teckit")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/source/Public-headers
)

configure_file(
    miktex-teckit-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-teckit-version.h
)

set(sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-teckit-version.h
    source/source/Engine.cpp
    source/source/Engine.h
    source/source/Public-headers/TECkit_Common.h
    source/source/Public-headers/TECkit_Engine.h
    source/source/TECkit_Format.h
)

set(${teckit_dll_name}_sources ${sources})

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        windows/miktex-teckit.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-teckit.rc
    )
    list(APPEND ${teckit_dll_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-teckit.rc
        windows/miktex-teckit.def
    )
endif()

add_library(${teckit_dll_name} SHARED ${${teckit_dll_name}_sources})

set_property(TARGET ${teckit_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_shared_library_version_properties(
    ${teckit_dll_name}
    ${MIKTEX_COMP_MAJOR_VERSION}.${MIKTEX_COMP_MINOR_VERSION}.${MIKTEX_COMP_PATCH_VERSION}
    ${MIKTEX_COMP_INTERFACE_VERSION}
)

target_include_directories(${teckit_dll_name}
    PUBLIC
        ${public_include_directories}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${teckit_dll_name} PRIVATE MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${teckit_dll_name} PRIVATE ${zlib_dll_name})
endif()

delay_load(${teckit_dll_name}
    ${zlib_dll_name}
)

install(TARGETS ${teckit_dll_name}
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
)
