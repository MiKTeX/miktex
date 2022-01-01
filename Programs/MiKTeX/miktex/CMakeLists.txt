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

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

list(APPEND miktex_sources
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
    utilities/Command.cpp
    utilities/Command.h
    utilities/Utility.cpp
    utilities/Utility.h
)

list(APPEND miktex_sources
    utilities/filesystem/commands/commands.h
    utilities/filesystem/commands/watch.cpp
    utilities/filesystem/utility.cpp
    utilities/filesystem/utility.h
)

list(APPEND miktex_sources
    utilities/fndb/commands/commands.h
    utilities/fndb/commands/remove.cpp
    utilities/fndb/commands/update.cpp
    utilities/fndb/utility.cpp
    utilities/fndb/utility.h
)

list(APPEND miktex_sources
    utilities/fontmaps/commands/FontMapManager.cpp
    utilities/fontmaps/commands/FontMapManager.h
    utilities/fontmaps/commands/commands.h
    utilities/fontmaps/commands/set-option.cpp
    utilities/fontmaps/commands/show-option.cpp
    utilities/fontmaps/commands/update.cpp
    utilities/fontmaps/utility.cpp
    utilities/fontmaps/utility.h
)

list(APPEND miktex_sources
    utilities/formats/commands/FormatsManager.cpp
    utilities/formats/commands/FormatsManager.h
    utilities/formats/commands/commands.h
    utilities/formats/commands/list.cpp
    utilities/formats/commands/update.cpp
    utilities/formats/utility.cpp
    utilities/formats/utility.h
)

list(APPEND miktex_sources
    utilities/languages/commands/commands.h
    utilities/languages/commands/update.cpp
    utilities/languages/utility.cpp
    utilities/languages/utility.h
)

list(APPEND miktex_sources
    utilities/links/commands/LinksManager.cpp
    utilities/links/commands/LinksManager.h
    utilities/links/commands/commands.h
    utilities/links/commands/remove.cpp
    utilities/links/commands/update.cpp
    utilities/links/utility.cpp
    utilities/links/utility.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND miktex_sources
        utilities/filetypes/commands/FileTypeManager.cpp
        utilities/filetypes/commands/FileTypeManager.h
        utilities/filetypes/commands/commands.h
        utilities/filetypes/commands/register.cpp
        utilities/filetypes/commands/unregister.cpp
        utilities/filetypes/utility.cpp
        utilities/filetypes/utility.h
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
