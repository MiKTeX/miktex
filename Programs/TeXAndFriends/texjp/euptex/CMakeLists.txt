## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_CURRENT_FOLDER}/eu-pTeX")

if(LINK_EVERYTHING_STATICALLY)
    use_static_crt()
endif()

default_char_type_is_unsigned()

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_INCLUDE_DIR}
)

add_definitions(
    -DHAVE_C4P_PRE_H
    -DSYNCTEX_ENGINE_H="synctex-euptex.h"
    -DTeX
    -D__SyncTeX__
    -DeupTeX
)

list(APPEND C4P_FLAGS
    --auto-exit-label=10
    --chars-are-unsigned
    --emit-optimize-pragmas
)

set(euptex_target_name euptex)

include(webify.cmake)

list(APPEND ${euptex_target_name}_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/euptexextra.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/euptexextra.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/euptexextra.c
    PROPERTIES LANGUAGE CXX
)

list(APPEND ${euptex_target_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/euptex_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-euptex.cpp
)

list(APPEND ${euptex_target_name}_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/c4p_pre.h
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex-first.h
)

list(APPEND ${euptex_target_name}_sources
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-common.h
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex-euptex.h
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.c
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.h
)

set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_SYNCTEX_SOURCE_DIR}/synctex.c
    PROPERTIES LANGUAGE CXX
)

file(READ ${MIKTEX_DYN_TEX_SCRIPT} tex_dyn_sed_contents)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed "${tex_dyn_sed_contents}")
endif()

set(euptex_web_file ${CMAKE_CURRENT_BINARY_DIR}/euptex-final.web)
set(euptex_header_file ${CMAKE_CURRENT_BINARY_DIR}/euptexd.h)

create_web_app(eupTeX)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/euptex_pool.cpp
    COMMAND
        inipool
            ${CMAKE_CURRENT_BINARY_DIR}/euptex.pool
            miktex-euptex.h
            ${euptex_progclass}
            ${euptex_prog}
        > ${CMAKE_CURRENT_BINARY_DIR}/euptex_pool.cpp
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_BINARY_DIR}/euptex.pool
    DEPENDS
        inipool
)

if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${MIKTEX_PREFIX}euptex
        ${kpsemu_lib_name}
        ${w2cemu_lib_name}
        ${web2c_sources_lib_name}
        texjp-ukanji
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${MIKTEX_PREFIX}euptex
            ${utf8wrap_lib_name}
        )
    endif()
else()
    target_link_libraries(${euptex_target_name}
        PRIVATE
            ${kpsemu_dll_name}
            ${w2cemu_dll_name}
            ${web2c_sources_dll_name}
            texjp-ukanji
    )
    if(MIKTEX_NATIVE_WINDOWS)
        target_link_libraries(${euptex_target_name}
            PRIVATE
                ${utf8wrap_dll_name}
        )
    endif()
endif()
