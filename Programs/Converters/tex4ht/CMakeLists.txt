## CMakeLists.txt
##
## Copyright (C) 2006-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_CONVERTERS_FOLDER}/tex4ht")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
    -DANSI
    -DCPLUSPLUSMAIN
    -DDISABLE_INSTALLER
    -DKPATHSEA
    -Dmain=Main
    -DUSE_MIKTEX_EXIT
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DWIN32
        -D_UNICODE -DUNICODE
    )
endif()

set_source_files_properties(
    source/t4ht.c
    source/tex4ht.c
    PROPERTIES LANGUAGE CXX
)

set(t4ht_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-t4ht-version.h
    miktex/tex4ht.h
    source/t4ht.c
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND t4ht_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-t4ht.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}t4ht ${t4ht_sources})

set_property(TARGET ${MIKTEX_PREFIX}t4ht PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}t4ht
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}t4ht
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}t4ht DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

set(tex4ht_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-tex4ht-version.h
    miktex/tex4ht.h
    source/tex4ht.c
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND tex4ht_sources
        ${MIKTEX_COMMON_MANIFEST}
        windows/miktex-tex4ht.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}tex4ht ${tex4ht_sources})

set_property(TARGET ${MIKTEX_PREFIX}tex4ht PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}tex4ht
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}tex4ht
        ${unxemu_dll_name}
        ${utf8wrap_dll_name}
    )
endif()

install(TARGETS ${MIKTEX_PREFIX}tex4ht DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})
