## initctangle.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(initctangle_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    init/initcommon.c
    init/initctangle.c
    miktex-ctangle-version.h
    source/comm-w2c.h
    texlive/help.h
)

set_source_files_properties(
    init/initcommon.c
    init/initctangle.c
    PROPERTIES LANGUAGE CXX
)

add_executable(initctangle ${initctangle_sources})

set_property(TARGET initctangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(initctangle
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(initctangle ${utf8wrap_dll_name})
endif()
