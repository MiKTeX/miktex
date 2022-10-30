## CMakeLists.txt
##
## Copyright (C) 2020-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_INDEXING_FOLDER}/upmendex")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    c-auto.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/c-auto.h
)

configure_file(
    miktex-upmendex-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-upmendex-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        miktex-upmendex.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-upmendex.rc
    )
endif()

add_definitions(
    -Dmain=Main
    -DUSE_MIKTEX_EXIT=1
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

set(upmendex_sources
    ${CMAKE_CURRENT_BINARY_DIR}/c-auto.h
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-upmendex-version.h
    ${MIKTEX_LIBRARY_WRAPPER}
    source/convert.c
    source/exkana.h
    source/exvar.h
    source/fread.c
    source/fwrite.c
    source/kana.h
    source/kp.c
    source/kp.h
    source/main.c
    source/mendex.h
    source/pageread.c
    source/qsort.c
    source/qsort.h
    source/sort.c
    source/styfile.c
    source/var.h
    source/version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND upmendex_sources
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-upmendex.rc
        ${MIKTEX_COMMON_MANIFEST}
    )
endif()

add_executable(${MIKTEX_PREFIX}upmendex ${upmendex_sources} ${headers})

set_property(TARGET ${MIKTEX_PREFIX}upmendex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}upmendex
    ${app_dll_name}
    ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}upmendex
        ${utf8wrap_dll_name}
    )
endif()

if(USE_SYSTEM_ICU)
    target_link_libraries(${MIKTEX_PREFIX}upmendex ICU::i18n)
    target_link_libraries(${MIKTEX_PREFIX}upmendex ICU::uc)
else()
    # not an option
endif()

install(TARGETS ${MIKTEX_PREFIX}upmendex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
