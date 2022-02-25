## CMakeLists.txt
##
## Copyright (C) 2021-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/miktex")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
    -DHAVE_CONFIG_H=1
)

add_definitions(
    -DWITH_LCDF_TYPETOOLS=1
)

if(NOT USE_SYSTEM_POPPLER)
    add_definitions(
        -DWITH_POPPLER_UTILS=1
    )
endif()

if(NOT USE_SYSTEM_FONTCONFIG)
    add_definitions(
        -DWITH_FONTCONFIG_UTILS=1
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

list(APPEND miktex_sources
    internal.cpp
    internal.h
    miktex.cpp
    shims/mkfntmap.cpp
    shims/mkfntmap.h
    shims/mktexlsr.cpp
    shims/mktexlsr.h
    shims/texlinks.cpp
    shims/texlinks.h
    shims/updmap.cpp
    shims/updmap.h
    topics/Command.cpp
    topics/Command.h
    topics/Topic.cpp
    topics/Topic.h
)

list(APPEND miktex_sources
    topics/filesystem/commands/commands.h
    topics/filesystem/commands/watch.cpp
    topics/filesystem/topic.cpp
    topics/filesystem/topic.h
)

list(APPEND miktex_sources
    topics/fndb/commands/commands.h
    topics/fndb/commands/refresh.cpp
    topics/fndb/commands/remove.cpp
    topics/fndb/topic.cpp
    topics/fndb/topic.h
)

list(APPEND miktex_sources
    topics/fontmaps/commands/FontMapManager.cpp
    topics/fontmaps/commands/FontMapManager.h
    topics/fontmaps/commands/commands.h
    topics/fontmaps/commands/configure.cpp
    topics/fontmaps/commands/set-option.cpp
    topics/fontmaps/commands/show-option.cpp
    topics/fontmaps/topic.cpp
    topics/fontmaps/topic.h
)

list(APPEND miktex_sources
    topics/formats/commands/FormatsManager.cpp
    topics/formats/commands/FormatsManager.h
    topics/formats/commands/commands.h
    topics/formats/commands/build.cpp
    topics/formats/commands/list.cpp
    topics/formats/topic.cpp
    topics/formats/topic.h
)

list(APPEND miktex_sources
    topics/languages/commands/commands.h
    topics/languages/commands/configure.cpp
    topics/languages/commands/list.cpp
    topics/languages/topic.cpp
    topics/languages/topic.h
)

list(APPEND miktex_sources
    topics/links/commands/LinksManager.cpp
    topics/links/commands/LinksManager.h
    topics/links/commands/commands.h
    topics/links/commands/install.cpp
    topics/links/commands/list.cpp
    topics/links/commands/uninstall.cpp
    topics/links/topic.cpp
    topics/links/topic.h
)

list(APPEND miktex_sources
    topics/packages/commands/commands.h
    topics/packages/commands/info.cpp
    topics/packages/commands/install.cpp
    topics/packages/commands/list.cpp
    topics/packages/commands/private.h
    topics/packages/commands/remove.cpp
    topics/packages/topic.cpp
    topics/packages/topic.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND miktex_sources
        topics/filetypes/commands/FileTypeManager.cpp
        topics/filetypes/commands/FileTypeManager.h
        topics/filetypes/commands/commands.h
        topics/filetypes/commands/list.cpp
        topics/filetypes/commands/register.cpp
        topics/filetypes/commands/unregister.cpp
        topics/filetypes/topic.cpp
        topics/filetypes/topic.h
    )
endif()

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

list(APPEND miktex_sources
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    miktex-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-version.h
)

list(APPEND miktex_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
    configure_file(
        miktex.rc.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/miktex.rc
    )
    list(APPEND miktex_sources
        ${MIKTEX_COMMON_MANIFEST}
        ${CMAKE_CURRENT_BINARY_DIR}/miktex.rc
    )
endif()

add_executable(miktex ${miktex_sources})

set_property(TARGET miktex PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

install(TARGETS miktex DESTINATION ${MIKTEX_BINARY_DESTINATION_DIR})

if (USE_SYSTEM_LOG4CXX)
    target_link_libraries(miktex MiKTeX::Imported::LOG4CXX)
else()
    target_link_libraries(miktex ${log4cxx_dll_name})
endif()

if (USE_SYSTEM_FMT)
    target_link_libraries(miktex MiKTeX::Imported::FMT)
else()
    target_link_libraries(miktex ${fmt_dll_name})
endif()

target_link_libraries(miktex
    ${core_dll_name}
    ${mpm_dll_name}
    ${setup_dll_name}
    miktex-popt-wrapper
)
