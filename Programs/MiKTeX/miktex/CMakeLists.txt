## CMakeLists.txt
##
## Copyright (C) 2021 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_PROGRAMS_FOLDER}/miktex")

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    miktex-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-version.h
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
    -DHAVE_CONFIG_H=1
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

set(miktex_sources
    internal.h
    miktex.cpp
    shims/mkfntmap.cpp
    shims/mkfntmap.h
    shims/updmap.cpp
    shims/updmap.h
    topics/Command.cpp
    topics/Command.h
    topics/Topic.cpp
    topics/Topic.h
    topics/filesystem/topic.cpp
    topics/filesystem/topic.h
    topics/filesystem/commands/commands.h
    topics/filesystem/commands/watch.cpp
    topics/fontmaps/topic.cpp
    topics/fontmaps/topic.h
    topics/fontmaps/commands/FontMapManager.cpp
    topics/fontmaps/commands/FontMapManager.h
    topics/fontmaps/commands/commands.h
    topics/fontmaps/commands/set-option.cpp
    topics/fontmaps/commands/show-option.cpp
    topics/fontmaps/commands/update.cpp
    topics/languages/topic.cpp
    topics/languages/topic.h
    topics/languages/commands/commands.h
    topics/languages/commands/update.cpp
    topics/links/topic.cpp
    topics/links/topic.h
    topics/links/commands/LinksManager.cpp
    topics/links/commands/LinksManager.h
    topics/links/commands/commands.h
    topics/links/commands/remove.cpp
    topics/links/commands/update.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND miktex_sources
        topics/filetypes/topic.cpp
        topics/filetypes/topic.h
        topics/filetypes/commands/FileTypeManager.cpp
        topics/filetypes/commands/FileTypeManager.h
        topics/filetypes/commands/commands.h
        topics/filetypes/commands/register.cpp
        topics/filetypes/commands/unregister.cpp
    )
endif()

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
