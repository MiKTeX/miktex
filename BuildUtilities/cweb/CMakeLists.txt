## CMakeLists.txt
##
## Copyright (C) 2006-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_BUILD_UTILITIES_FOLDER}/CWeb")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/texlive
)

add_definitions(
    -DUSE_MIKTEX_EXIT
)

set(initctangle_sources
    ${MIKTEX_LIBRARY_WRAPPER}
    init/initcommon.c
    init/initctangle.c
    init/initcweb.h
    miktex-ctangle-version.h
    texlive/help.h
)

set_source_files_properties(
    init/initcommon.c
    init/initctangle.c
    PROPERTIES LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND initctangle_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-ctangle.rc
    )
endif()

add_executable(initctangle ${initctangle_sources})

set_property(TARGET initctangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(initctangle
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

set(ctangle_sources
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-ctangle-version.h
    texlive/help.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    PROPERTIES LANGUAGE CXX
)

set_source_files_properties(
    ${MIKTEX_LIBRARY_WRAPPER}
    PROPERTIES COMPILE_FLAGS "-DCPLUSPLUSMAIN"
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND ctangle_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-ctangle.rc
    )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    COMMAND
        initctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
        ${CMAKE_CURRENT_SOURCE_DIR}/texlive/ctang-w2c.ch
        ${CMAKE_CURRENT_BINARY_DIR}/ctangle.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ctangle.w
        ${CMAKE_CURRENT_SOURCE_DIR}/texlive/ctang-w2c.ch
        initctangle
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/common.c
        ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
    COMMAND
        initctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
        ${CMAKE_CURRENT_SOURCE_DIR}/texlive/comm-w2c.ch
        ${CMAKE_CURRENT_BINARY_DIR}/common.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.w
        ${CMAKE_CURRENT_SOURCE_DIR}/texlive/comm-w2c.ch
        initctangle
    VERBATIM
)

add_executable(${MIKTEX_PREFIX}ctangle ${ctangle_sources})

set_property(TARGET ${MIKTEX_PREFIX}ctangle PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}ctangle
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

set(cweave_sources
    ${CMAKE_CURRENT_BINARY_DIR}/common.c
    ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    ${CMAKE_CURRENT_BINARY_DIR}/cweb.h
    ${MIKTEX_LIBRARY_WRAPPER}
    miktex-cweave-version.h
    texlive/help.h
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    PROPERTIES LANGUAGE CXX
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND cweave_sources
        ${MIKTEX_COMMON_MANIFEST}
        miktex-cweave.rc
    )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    COMMAND
        ${MIKTEX_PREFIX}ctangle
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
        ${CMAKE_CURRENT_SOURCE_DIR}/texlive/cweav-w2c.ch
        ${CMAKE_CURRENT_BINARY_DIR}/cweave.c
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/source/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/cweave.w
        ${CMAKE_CURRENT_SOURCE_DIR}/source/prod.w
        ${CMAKE_CURRENT_SOURCE_DIR}/texlive/cweav-w2c.ch
        ${MIKTEX_PREFIX}ctangle
    VERBATIM
)

add_executable(${MIKTEX_PREFIX}cweave ${cweave_sources})

set_property(TARGET ${MIKTEX_PREFIX}cweave PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}cweave
    ${app_dll_name}
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

install(
    TARGETS
        ${MIKTEX_PREFIX}ctangle
        ${MIKTEX_PREFIX}cweave
    DESTINATION
        "${MIKTEX_BINARY_DESTINATION_DIR}"
)
