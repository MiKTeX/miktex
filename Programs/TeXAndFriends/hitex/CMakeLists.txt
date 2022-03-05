## CMakeLists.txt
##
## Copyright (C) 2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}/HiTeX")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

add_definitions(
    -DINIT
    -DSTAT
    -Dmain=Main
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -DWIN32
        -D_UNICODE
    )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/hibasetypes.h
        ${CMAKE_CURRENT_BINARY_DIR}/hierror.h
        ${CMAKE_CURRENT_BINARY_DIR}/hiformat.c
        ${CMAKE_CURRENT_BINARY_DIR}/hiformat.h
        ${CMAKE_CURRENT_BINARY_DIR}/higet.c
        ${CMAKE_CURRENT_BINARY_DIR}/higet.h
        ${CMAKE_CURRENT_BINARY_DIR}/hilexer.l
        ${CMAKE_CURRENT_BINARY_DIR}/himktables.c
        ${CMAKE_CURRENT_BINARY_DIR}/hiparser.y
        ${CMAKE_CURRENT_BINARY_DIR}/hiput.c
        ${CMAKE_CURRENT_BINARY_DIR}/hiput.h
        ${CMAKE_CURRENT_BINARY_DIR}/hishrink.c
        ${CMAKE_CURRENT_BINARY_DIR}/hiskip.c
        ${CMAKE_CURRENT_BINARY_DIR}/histretch.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle
            ${CMAKE_CURRENT_SOURCE_DIR}/source/hiformat.w
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/hiformat.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/hitex.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle
            ${CMAKE_CURRENT_SOURCE_DIR}/source/hitex.w
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/hitex.w
    DEPENDS
        ${MIKTEX_PREFIX}ctangle
)

set(himktables_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/himktables.c
    ${CMAKE_CURRENT_BINARY_DIR}/hibasetypes.h
    ${CMAKE_CURRENT_BINARY_DIR}/hiformat.h
)

add_executable(himktables ${himktables_sources})

target_link_libraries(himktables
    ${app_dll_name}
    ${core_dll_name}
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/hitables.c
    COMMAND
        himktables > ${CMAKE_CURRENT_BINARY_DIR}/hitables.c
    DEPENDS
        himktables
)

set(hitex_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/hitables.c
    ${CMAKE_CURRENT_BINARY_DIR}/hiput.c
    ${CMAKE_CURRENT_BINARY_DIR}/hiput.h
    ${CMAKE_CURRENT_BINARY_DIR}/hitex.c
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND hitex_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-hitex.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}hitex ${hitex_sources})

target_link_libraries(${MIKTEX_PREFIX}hitex
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${MIKTEX_PREFIX}hitex MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${MIKTEX_PREFIX}hitex ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}hitex ${utf8wrap_dll_name})
endif()

install(
    TARGETS
        ${MIKTEX_PREFIX}hitex
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)

set(histretch_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/hitables.c
    ${CMAKE_CURRENT_BINARY_DIR}/histretch.c
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND histretch_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-histretch.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}histretch ${histretch_sources})

target_link_libraries(${MIKTEX_PREFIX}histretch
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${MIKTEX_PREFIX}histretch MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${MIKTEX_PREFIX}histretch ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}histretch ${utf8wrap_dll_name})
endif()

install(
    TARGETS
        ${MIKTEX_PREFIX}histretch
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)






set(hishrink_sources
    ${CMAKE_CURRENT_BINARY_DIR}/hishrink.c
    ${CMAKE_CURRENT_BINARY_DIR}/hitables.c
    ${MIKTEX_LIBRARY_WRAPPER}
    source/hilexer.c
    source/hiparser.c
    source/hiparser.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND hishrink_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-hishrink.rc
    )
endif()

add_executable(${MIKTEX_PREFIX}hishrink ${hishrink_sources})

target_link_libraries(${MIKTEX_PREFIX}hishrink
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

if(USE_SYSTEM_ZLIB)
    target_link_libraries(${MIKTEX_PREFIX}hishrink MiKTeX::Imported::ZLIB)
else()
    target_link_libraries(${MIKTEX_PREFIX}hishrink ${zlib_dll_name})
endif()

if(MIKTEX_NATIVE_WINDOWS)
    target_link_libraries(${MIKTEX_PREFIX}hishrink ${utf8wrap_dll_name})
endif()

install(
    TARGETS
        ${MIKTEX_PREFIX}hishrink
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)

