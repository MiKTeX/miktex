## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2021 Christian Schenk
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

use_static_crt()

add_library(${core_lib_name} STATIC ${core_sources})

set_property(TARGET ${core_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${core_lib_name} gen-core-sources)

set_source_files_properties(
  ${generated_core_sources}
  PROPERTIES
    GENERATED TRUE
)

add_dependencies(${core_lib_name} gen-git-info)

set_source_files_properties(
  ${MIKTEX_GITINFO_H}
  PROPERTIES
    GENERATED TRUE
)

target_compile_definitions(${core_lib_name}
  PUBLIC
    -DMIKTEX_CORE_STATIC
)

target_include_directories(${core_lib_name}
  PUBLIC
    ${public_include_directories}
)

if(USE_SYSTEM_BZIP2)
  target_link_libraries(${core_lib_name} PUBLIC MiKTeX::Imported::BZIP2)
else()
  target_link_libraries(${core_lib_name} PUBLIC ${bzip2_lib_name})
endif()

if(USE_SYSTEM_FMT)
  target_link_libraries(${core_lib_name} PUBLIC MiKTeX::Imported::FMT)
else()
  target_link_libraries(${core_lib_name} PUBLIC ${fmt_lib_name})
endif()

if(USE_SYSTEM_LZMA)
  target_link_libraries(${core_lib_name} PUBLIC MiKTeX::Imported::LZMA)
else()
  target_link_libraries(${core_lib_name} PUBLIC ${lzma_lib_name})
endif()

if(USE_SYSTEM_URIPARSER)
  target_link_libraries(${core_lib_name} PUBLIC MiKTeX::Imported::URIPARSER)
else()
  target_link_libraries(${core_lib_name} PUBLIC ${uriparser_lib_name})
endif()

if(USE_SYSTEM_ZLIB)
  target_link_libraries(${core_lib_name} PUBLIC MiKTeX::Imported::ZLIB)
else()
  target_link_libraries(${core_lib_name} PUBLIC ${zlib_lib_name})
endif()

target_link_libraries(${core_lib_name}
  PUBLIC
    ${config_lib_name}
    ${md5_lib_name}
    ${trace_lib_name}
    ${util_lib_name}
    Threads::Threads
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${core_lib_name}
    PUBLIC
      SHFolder.lib
      shlwapi.lib
  )
  if(MSVC)
    target_link_libraries(${core_lib_name}
      PUBLIC
        htmlhelp.lib
    )
  endif()
endif()

if(MIKTEX_MACOS_BUNDLE)
  target_link_libraries(${core_lib_name}
    PUBLIC
      "-framework CoreServices"
  )
endif()

if(MIKTEX_FREEBSD)
  target_link_libraries(${core_lib_name}
    PUBLIC
      kvm
      procstat
      ${Inotify_LIBRARIES}
  )
endif()

if(USE_SYSTEM_OPENSSL_CRYPTO)
  target_link_libraries(${core_lib_name} PUBLIC MiKTeX::Imported::OPENSSL::CRYPTO)
elseif(WITH_LIBRESSL_CRYPTO)
  target_link_libraries(${core_lib_name} PUBLIC ${openssl_crypto_lib_name})
endif()

source_group("C API" FILES ${c_api_sources})
source_group(Cfg FILES ${cfg_sources})
source_group(CommandLineBuilder FILES ${commandlinebuilder_sources})
source_group(CsvList FILES ${csvlist_sources})
source_group(Directory FILES ${directory_sources})
source_group(DirectoryLister FILES ${directorylister_sources})
source_group(File FILES ${file_sources})
source_group(Fndb FILES ${fndb_sources})
source_group(LockFile FILES ${lockfile_sources})
source_group(MD5 FILES ${md5_sources})
source_group(MemoryMappedFile FILES ${memorymappedfile_sources})
source_group(Misc FILES ${misc_sources})
source_group(PathName FILES ${pathname_sources})
source_group(PathNameParser FILES ${pathnameparser_sources})
source_group(Process FILES ${process_sources})
source_group(Public/Core FILES ${public_headers_c} ${public_headers_core})
source_group(Public/noext FILES ${public_headers_no_ext})
source_group(Session FILES ${session_sources})
source_group(Stream FILES ${stream_sources})
source_group(TemporaryDirectory FILES ${temporarydirectory_sources})
source_group(TemporaryFile FILES ${temporaryfile_sources})
source_group(Uri FILES ${uri_sources})
source_group(Utils FILES ${utils_sources})
source_group(VersionNumber FILES ${versionnumber_sources})
