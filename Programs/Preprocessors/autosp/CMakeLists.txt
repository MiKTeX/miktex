## CMakeLists.txt
##
## Copyright (C) 2017-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_PREPROCESSORS_FOLDER}/autosp")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_definitions(
    -DHAVE_CONFIG_H
)

set(autosp_sources
    source/autosp.c
    source/autosp.h
    source/process_command.c
    source/process_command.h
    source/process_score.c
    source/process_score.h
    source/spacing_note.c
    source/status.c
    source/utils.c
    source/utils.h
    source/xtuplet.c
)

list(APPEND autosp_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    miktex-autosp-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND autosp_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-autosp.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}autosp ${autosp_sources})

set_property(TARGET ${MIKTEX_PREFIX}autosp PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}autosp ${getopt_dll_name})
endif()

install(TARGETS ${MIKTEX_PREFIX}autosp DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(rebar_sources
    source/rebar.c
    source/utils.c
    source/utils.h
)

list(APPEND rebar_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    miktex-rebar-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND rebar_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-rebar.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}rebar ${rebar_sources})

set_property(TARGET ${MIKTEX_PREFIX}rebar PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}rebar ${getopt_dll_name})
endif()

install(TARGETS ${MIKTEX_PREFIX}rebar DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(tex2aspc_sources
    source/tex2aspc.c
    source/utils.c
    source/utils.h
)

list(APPEND tex2aspc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    miktex-tex2aspc-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND tex2aspc_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-tex2aspc.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}tex2aspc ${tex2aspc_sources})

set_property(TARGET ${MIKTEX_PREFIX}tex2aspc PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}tex2aspc ${getopt_dll_name})
endif()

install(TARGETS ${MIKTEX_PREFIX}tex2aspc DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
