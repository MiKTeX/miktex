## CMakeLists.txt
##
## Copyright (C) 2006-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/core")

set(core_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(core_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(REPORT_EVENTS FALSE)
set(MIKTEX_FNDB_VERSION 5)

configure_file(
    include/miktex/Core/Paths.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/Paths.h
)

configure_file(
    include/miktex/Core/Urls.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/Urls.h
)

configure_file(
    include/miktex/Core/IntegerTypes.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/IntegerTypes.h
)

configure_file(
    include/miktex/Core/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/config.h
)

configure_file(
    config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    core-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/core-version.h
)

configure_file(
    include/miktex/Core/vi/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/vi/Version.h
)

set(miktex_der_h ${CMAKE_CURRENT_BINARY_DIR}/miktex.der.h)
set(miktex_config_ini_h ${CMAKE_CURRENT_BINARY_DIR}/miktex-config.ini.h)

set(headers_no_ext
    miktex/Core/AutoResource
    miktex/Core/BZip2Stream
    miktex/Core/BufferSizes
    miktex/Core/Cfg
    miktex/Core/CommandLineBuilder
    miktex/Core/CsvList
    miktex/Core/Debug
    miktex/Core/Directory
    miktex/Core/DirectoryLister
    miktex/Core/Environment
    miktex/Core/Exceptions
    miktex/Core/File
    miktex/Core/FileStream
    miktex/Core/FileSystemWatcher
    miktex/Core/FileType
    miktex/Core/Fndb
    miktex/Core/GzipStream
    miktex/Core/Help
    miktex/Core/IntegerTypes
    miktex/Core/LibraryVersion
    miktex/Core/LockFile
    miktex/Core/LzmaStream
    miktex/Core/MD5
    miktex/Core/MemoryMappedFile
    miktex/Core/Paths
    miktex/Core/Process
    miktex/Core/Quoter
    miktex/Core/RootDirectoryInfo
    miktex/Core/Session
    miktex/Core/Stream
    miktex/Core/StreamReader
    miktex/Core/StreamWriter
    miktex/Core/TemporaryDirectory
    miktex/Core/TemporaryFile
    miktex/Core/Test
    miktex/Core/Text
    miktex/Core/Uri
    miktex/Core/Urls
    miktex/Core/Utils
    miktex/Core/VersionNumber
    miktex/Core/ci_string
    miktex/Core/equal_icase
    miktex/Core/hash_icase
    miktex/Core/less_icase_dos
    miktex/Core/vi/Version
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND headers_no_ext
        miktex/Core/win/COMInitializer
        miktex/Core/win/ConsoleCodePageSwitcher
        miktex/Core/win/DllProc
        miktex/Core/win/HResult
        miktex/Core/win/Registry
        miktex/Core/win/WindowsVersion
        miktex/Core/win/winAutoResource
    )
endif()

if(APPLE)
    list(APPEND headers_no_ext
        miktex/Core/mac/MacOsVersion
    )
endif()

if(MIKTEX_LINUX)
    list(APPEND headers_no_ext
        miktex/Core/tux/LinuxOsRelease
    )
endif()

foreach(h ${headers_no_ext})
    set(_path ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
    list(APPEND public_headers_no_ext ${_path})
    if (NOT EXISTS ${_path})
        file(WRITE ${_path}
        "#pragma once\n#include <${h}.h>\n"
        )
    endif()
endforeach()

set(public_headers_core
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/IntegerTypes.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/Paths.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/Urls.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/vi/Version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/AutoResource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/BZip2Stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/BufferSizes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Cfg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/CommandLineBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/CsvList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Directory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/DirectoryLister.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Environment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Exceptions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/File.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/FileStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/FileType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Fndb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/GzipStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Help.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/LibraryVersion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/LockFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/LzmaStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/MD5.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/MemoryMappedFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Process.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Quoter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/RootDirectoryInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StreamReader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StreamWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StructChecker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TemporaryDirectory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TemporaryFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Test.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Text.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Uri.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/VersionNumber.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/ci_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/equal_icase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/hash_icase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/less_icase_dos.h
)

set(public_headers_c
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/c/api.h
)

set(public_headers
    ${public_headers_c}
    ${public_headers_core}
    
    ${public_headers_no_ext}
)

if(MIKTEX_NATIVE_WINDOWS)
    set(public_headers_win
        ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/win/COMInitializer
        ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/win/ConsoleCodePageSwitcher
        ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/win/DllProc
        ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/win/HResult
        ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/win/Registry
        ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/win/WindowsVersion
        ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/win/winAutoResource
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/COMInitializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/ConsoleCodePageSwitcher.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/DllProc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/HResult.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/Registry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/WindowsVersion.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/winAutoResource.h
    )
    list(APPEND public_headers ${public_headers_win})
endif()

if(APPLE)
    set(public_headers_mac
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/mac/MacOsVersion
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/mac/MacOsVersion.h
    )
    list(APPEND public_headers ${public_headers_mac})
endif()

if(MIKTEX_LINUX)
    set(public_headers_tux
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/tux/LinuxOsRelease
        ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/tux/LinuxOsRelease.h
    )
    list(APPEND public_headers ${public_headers_tux})
endif()

set(cfg_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Cfg/Cfg.cpp
)

set(commandlinebuilder_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineBuilder/ArgvImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineBuilder/CommandLineBuilder.cpp
)

set(csvlist_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/CsvList/CsvList.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND commandlinebuilder_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineBuilder/win/winCommandLineBuilder.cpp
    )
else()
    list(APPEND commandlinebuilder_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineBuilder/unx/unxCommandLineBuilder.cpp
    )
endif()

set(directory_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Directory/Directory.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND directory_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Directory/win/winDirectory.cpp
    )
else()
    list(APPEND directory_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Directory/unx/unxDirectory.cpp
    )
endif()

set(directorylister_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/DirectoryLister/DirectoryLister.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND directorylister_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/DirectoryLister/win/winDirectoryLister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DirectoryLister/win/winDirectoryLister.h
    )
else()
    list(APPEND directorylister_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/DirectoryLister/unx/unxDirectoryLister.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DirectoryLister/unx/unxDirectoryLister.h
    )
endif()

set(file_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/File/File.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND file_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/File/win/winFile.cpp
    )
else()
    list(APPEND file_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/File/unx/unxFile.cpp
    )
endif()

set(filesystemwatcher_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/FileSystemWatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/FileSystemWatcherBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/FileSystemWatcherBase.h
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND filesystemwatcher_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/win/winFileSystemWatcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/win/winFileSystemWatcher.h
    )
elseif(APPLE)
    list(APPEND filesystemwatcher_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/mac/macFileSystemWatcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/mac/macFileSystemWatcher.h
    )
else()
    list(APPEND filesystemwatcher_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/unx/unxFileSystemWatcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemWatcher/unx/unxFileSystemWatcher.h
    )
endif()

set(fndb_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/FileNameDatabase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/FileNameDatabase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/Fndb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/fndbmem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/makefndb.cpp
)

set(lockfile_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/LockFile/LockFile.cpp
)

set(md5_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/MD5/MD5.cpp
)

set(memorymappedfile_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/MemoryMappedFile/MemoryMappedFile.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND memorymappedfile_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/MemoryMappedFile/win/winMemoryMappedFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MemoryMappedFile/win/winMemoryMappedFile.h
    )
else()
    list(APPEND memorymappedfile_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/MemoryMappedFile/unx/unxMemoryMappedFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MemoryMappedFile/unx/unxMemoryMappedFile.h
    )
endif()

set(misc_sources
    ${CMAKE_CURRENT_BINARY_DIR}/core-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vi/Runtime.cpp
    ${miktex_config_ini_h}
    ${miktex_der_h}
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND misc_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/win/COMInitializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/ConsoleCodePageSwitcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/DllProc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/WindowsVersion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/winRegistry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/winRegistry.h
    )
else()
    list(APPEND misc_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/unx/unx.cpp
    )
    if(APPLE)
        list(APPEND misc_sources
            ${CMAKE_CURRENT_SOURCE_DIR}/mac/MacOsVersion.cpp
        )
    endif()
    if(MIKTEX_LINUX)
        list(APPEND misc_sources
            ${CMAKE_CURRENT_SOURCE_DIR}/tux/LinuxOsRelease.cpp
        )
    endif()
endif()

set(process_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Process/Process.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND process_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Process/win/winProcess.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Process/win/winProcess.h
    )
else()
    list(APPEND process_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Process/unx/unxProcess.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Process/unx/unxProcess.h
    )
endif()

set(temporarydirectory_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/TemporaryDirectory/TemporaryDirectory.cpp
)

set(temporaryfile_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/TemporaryFile/TemporaryFile.cpp
)

set(session_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/RootDirectoryInternals.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/SessionImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/StartupConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/appnames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/files.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/filetypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/findfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/fontinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/formats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/graphics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/gsinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/languages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/mfmodes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/miktex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/papersize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/runexe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/rungs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/runperl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/searchpath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/texmfroot.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND session_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Session/win/runbat.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Session/win/winSession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Session/win/winStartupConfig.cpp
    )
else()
    list(APPEND session_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Session/unx/runsh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Session/unx/unxSession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Session/unx/unxStartupConfig.cpp
    )
endif()

set(stream_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/BZip2Stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/CompressedStreamBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/CompressedStreamBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/FileStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/GzipStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/LzmaStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/Stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/StreamReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream/StreamWriter.cpp
)

set(uri_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Uri/Uri.cpp
)

set(utils_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/CoreStopWatch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Pipe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/inliners.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/uncompress.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND utils_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils/win/winUtils.cpp
    )
else()  
    list(APPEND utils_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils/unx/unxUtils.cpp
    )
endif()

set(versionnumber_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/VersionNumber/VersionNumber.cpp
)

set(c_api_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/c/api.cpp
)

set(core_sources
    ${c_api_sources}
    ${cfg_sources}
    ${commandlinebuilder_sources}
    ${csvlist_sources}
    ${directory_sources}
    ${directorylister_sources}
    ${file_sources}
    ${filesystemwatcher_sources}
    ${fndb_sources}
    ${lockfile_sources}
    ${md5_sources}
    ${memorymappedfile_sources}
    ${misc_sources}
    ${process_sources}
    ${public_headers}
    ${session_sources}
    ${stream_sources}
    ${temporarydirectory_sources}
    ${temporaryfile_sources}
    ${uri_sources}
    ${utils_sources}
    ${versionnumber_sources}
)

add_custom_command(
    OUTPUT
        ${miktex_der_h}
    COMMAND
        bintoc miktex_der
        < ${CMAKE_SOURCE_DIR}/Resources/PublicKeys/miktex.der
        > ${CMAKE_CURRENT_BINARY_DIR}/miktex.der.h.tmp
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/miktex.der.h.tmp
        ${miktex_der_h}
    COMMAND
        ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/miktex.der.h.tmp
    DEPENDS
        bintoc
        ${CMAKE_SOURCE_DIR}/Resources/PublicKeys/miktex.der
    VERBATIM
)

list(APPEND generated_core_sources
    ${miktex_der_h}
)

add_custom_command(
    OUTPUT
        ${miktex_config_ini_h}
    COMMAND
        bintoc miktex_config_ini
        < ${CMAKE_BINARY_DIR}/${MIKTEX_REL_CONFIG_DIR}/miktex-config.ini
        > ${CMAKE_CURRENT_BINARY_DIR}/miktex-config.ini.h.tmp
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/miktex-config.ini.h.tmp
        ${miktex_config_ini_h}
    COMMAND
        ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/miktex-config.ini.h.tmp
    DEPENDS
        bintoc
        ${CMAKE_BINARY_DIR}/${MIKTEX_REL_CONFIG_DIR}/miktex-config.ini
    VERBATIM
)

list(APPEND generated_core_sources
    ${miktex_config_ini_h}
)

if(REPORT_EVENTS)
    add_custom_command(
        OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/MiKTeXEvents.h
        ${CMAKE_CURRENT_BINARY_DIR}/MiKTeXEvents.rc
        ${CMAKE_CURRENT_BINARY_DIR}/MSG00001.bin
        COMMAND
        ${MC_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/MiKTeXEvents.mc
        DEPENDS
        MiKTeXEvents.mc
        VERBATIM
    )
    list(APPEND generated_core_sources
        ${core_binary_dir}/MiKTeXEvents.h
    )
endif()

list(APPEND core_sources ${generated_core_sources})

add_custom_target(gen-core-sources DEPENDS
    ${generated_core_sources}
)

set_property(TARGET gen-core-sources PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

list(APPEND core_sources ${MIKTEX_GITINFO_H})

if(INSTALL_MIKTEX_HEADERS)
    install(
        FILES
            ${public_headers_core}
        DESTINATION
            ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/Core
    )
    install(
        FILES
            ${public_headers_c}
        DESTINATION
            ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/Core/c
    )
    if(MIKTEX_NATIVE_WINDOWS)
        install(
            FILES
                ${public_headers_win}
            DESTINATION
                ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/Core/win
        )
    endif()
    if(APPLE)
        install(
            FILES
                ${public_headers_mac}
            DESTINATION
                ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/Core/mac
        )
    endif()
    if(MIKTEX_LINUX)
        install(
            FILES
                ${public_headers_tux}
            DESTINATION
                ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/Core/tux
        )
    endif()
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_subdirectory(shared)
add_subdirectory(test)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
