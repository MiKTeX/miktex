## miktex.cmake
##
## Copyright (C) 2010-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(miktex_sources
    miktex/luatex.h
    miktex/miktex.cpp
)

add_library(luatex-miktex-objects OBJECT ${miktex_sources})

set_property(TARGET luatex-miktex-objects PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(luatex-miktex-objects
    PUBLIC
        ${app_dll_name}
        ${core_dll_name}
        ${kpsemu_dll_name}
)

if(USE_SYSTEM_FMT)
    target_link_libraries(luatex-miktex-objects PUBLIC MiKTeX::Imported::FMT)
else()
    target_link_libraries(luatex-miktex-objects PUBLIC ${fmt_dll_name})
endif()
