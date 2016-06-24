## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2016 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

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

add_definitions(
  -DHAVE_CONFIG_H
)

set(ADMIN_CONTROLS_USER_CONFIG TRUE)
set(REPORT_EVENTS FALSE)

configure_file(
  include/miktex/Core/Paths.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/Paths.h
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
  StdAfx.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h
)  

set(miktex_der_h ${CMAKE_CURRENT_BINARY_DIR}/miktex.der.h)

set(public_headers_core
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/IntegerTypes.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/Paths.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/Core/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/AutoResource
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/AutoResource.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/BZip2Stream
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/BZip2Stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/BufferSizes
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/BufferSizes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/CSVList
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/CSVList.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Cfg
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Cfg.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/CommandLineBuilder
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/CommandLineBuilder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Debug
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Debug.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Directory
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Directory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/DirectoryLister
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/DirectoryLister.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Environment
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Environment.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Exceptions
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Exceptions.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/File
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/File.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/FileStream
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/FileStream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/FileType
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/FileType.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Fndb
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Fndb.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/GzipStream
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/GzipStream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/HasNamedValues
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/HasNamedValues.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Help
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Help.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/IntegerTypes
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/LzmaStream
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/LzmaStream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/MD5
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/MD5.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/MemoryMappedFile
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/MemoryMappedFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/OptionSet
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/OptionSet.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/PathName
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/PathName.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/PathNameParser
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/PathNameParser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Paths
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Process.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Quoter
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Quoter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Registry
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Registry.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Session
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Session.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Stream
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StreamReader
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StreamReader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StreamWriter
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StreamWriter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/StructChecker.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TemporaryDirectory
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TemporaryDirectory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TemporaryFile
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TemporaryFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Test
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Test.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Text
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Text.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TriState
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/TriState.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Uri
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Uri.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Urls
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Urls.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Utils
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/Utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/VersionNumber
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/VersionNumber.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/ci_string
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/ci_string.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/equal_icase
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/equal_icase.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/hash_icase
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/hash_icase.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/less_icase_dos
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/less_icase_dos.h
)

set(public_headers_c
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/c/api.h
)

set(public_headers
  ${public_headers_c}
  ${public_headers_core}
)

if(MIKTEX_NATIVE_WINDOWS)
  set(public_headers_win
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/DllProc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/DllProc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/HResult
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/HResult.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/WindowsVersion
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/WindowsVersion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/winAutoResource
    ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Core/win/winAutoResource.h
  )
  list(APPEND public_headers ${public_headers_win})
endif()

set(cfg_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Cfg/Cfg.cpp
)

set(commandlinebuilder_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineBuilder/CommandLineBuilder.cpp
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

set(fndb_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/FileNameDatabase.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/FileNameDatabase.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/Fndb.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/fndbmem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Fndb/makefndb.cpp
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
  ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Debug.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Exceptions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${miktex_der_h}
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND misc_sources
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
endif()

set(pathname_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/PathName/PathName.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND pathname_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/PathName/win/winPathName.cpp
  )
else()
  list(APPEND pathname_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/PathName/unx/unxPathName.cpp
  )
endif()

set(pathnameparser_sources
 ${CMAKE_CURRENT_SOURCE_DIR}/PathNameParser/PathNameParser.cpp
)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/FormatInfo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/LanguageInfo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/RootDirectory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/Session.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/SessionImpl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/appnames.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/config.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/error.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/files.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/filetypes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/findfile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/fontinfo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/graphics.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/gsinfo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/mfmodes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/miktex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/papersize.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/runperl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/searchpath.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Session/texmfroot.cpp
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND session_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/win/runbat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/win/winSession.cpp
  )
else()
  list(APPEND session_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Session/unx/unxSession.cpp
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
  ${CMAKE_CURRENT_SOURCE_DIR}/Utils/AutoTraceTime.h
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
  ${directory_sources}
  ${directorylister_sources}
  ${file_sources}
  ${fndb_sources}
  ${md5_sources}
  ${memorymappedfile_sources}
  ${misc_sources}
  ${pathname_sources}
  ${pathnameparser_sources}
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
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
  add_subdirectory(test)
endif()

add_subdirectory(static)
