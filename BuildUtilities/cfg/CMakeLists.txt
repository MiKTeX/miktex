## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}")

set(cfg_sources
    cfg-version.h
    cfg.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND cfg_sources
        ${MIKTEX_COMMON_MANIFEST}
        cfg.rc
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
