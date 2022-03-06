## triptex.cmake
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(triptex_dll_name "triptex_")

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/triptex-miktex.ch
    COMMAND
        ${MIKTEX_PREFIX}tie
        -c ${CMAKE_CURRENT_BINARY_DIR}/triptex-miktex.ch
        ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web
        ${tex_changefiles}
    DEPENDS
        ${MIKTEX_PREFIX}tie
        source/tex.web
        ${tex_changefiles}
    VERBATIM
)

set(triptex_web_file ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web)
set(triptex_change_file ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.ch)

if(LINK_EVERYTHING_STATICALLY)
    set(triptex_target_name ${triptex_lib_name})
else()
    set(triptex_target_name ${triptex_dll_name})
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
    COMMAND
        inipool
        ${CMAKE_CURRENT_BINARY_DIR}/triptex.pool
        miktex-tex.h
        > ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        inipool
        ${CMAKE_CURRENT_BINARY_DIR}/triptex.pool
)

set(${triptex_target_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
)

create_web_app(TripTeX)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${triptex_target_name}
        ${w2cemu_lib_name}
    )
else()
    target_link_libraries(${triptex_target_name}
        PRIVATE
            ${w2cemu_dll_name}
    )
endif()

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/triptex_pool.cpp
    PROPERTIES COMPILE_FLAGS
        "-DMIKTEX_TRIPTEX"
)
