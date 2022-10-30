## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(ttf2afm_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${projdir}/source/ttf2afm.c
    ttf2afm-version.h
)

set_source_files_properties(
    ${projdir}/source/ttf2afm.c
    PROPERTIES LANGUAGE CXX
)

set_source_files_properties(
  ${MIKTEX_LIBRARY_WRAPPER}
  PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ttf2afm_sources
        ${MIKTEX_COMMON_MANIFEST}
        ttf2afm.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}ttf2afm ${ttf2afm_sources})

set_property(TARGET ${MIKTEX_PREFIX}ttf2afm PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/ttf2afm)

target_link_libraries(${MIKTEX_PREFIX}ttf2afm
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${texmf_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}ttf2afm
        ${getopt_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}ttf2afm DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
