## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    cfg-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/cfg-version.h
)

set(cfg_sources
    ${CMAKE_CURRENT_BINARY_DIR}/cfg-version.h
    cfg.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        cfg.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/cfg.rc
    )
    list(APPEND cfg_sources
        ${CMAKE_CURRENT_BINARY_DIR}/cfg.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

add_executable(cfg ${cfg_sources})

set_property(TARGET cfg PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(cfg
    ${core_dll_name}
    miktex-popt-wrapper
)

install(TARGETS cfg DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
