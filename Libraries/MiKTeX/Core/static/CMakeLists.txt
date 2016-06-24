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

use_static_crt()

add_library(${core_lib_name} STATIC ${core_sources})

set_property(TARGET ${core_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${core_lib_name} gen-core-sources)

set_source_files_properties(
  ${generated_core_sources}
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

target_link_libraries(${core_lib_name}
  PUBLIC
    ${botan_lib_name}
    ${bzip2_lib_name}
    ${lzma_lib_name}
    ${md5_lib_name}
    ${trace_lib_name}
    ${uriparser_lib_name}
    ${util_lib_name}
    ${zlib_lib_name}
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

source_group("C API" FILES ${c_api_sources})
source_group(Cfg FILES ${cfg_sources})
source_group(CommandLineBuilder FILES ${commandlinebuilder_sources})
source_group(Directory FILES ${directory_sources})
source_group(DirectoryLister FILES ${directorylister_sources})
source_group(File FILES ${file_sources})
source_group(Fndb FILES ${fndb_sources})
source_group(MD5 FILES ${md5_sources})
source_group(MemoryMappedFile FILES ${memorymappedfile_sources})
source_group(Misc FILES ${misc_sources})
source_group(PathName FILES ${pathname_sources})
source_group(PathNameParser FILES ${pathnameparser_sources})
source_group(Process FILES ${process_sources})
source_group(Public FILES ${public_headers})
source_group(Session FILES ${session_sources})
source_group(Stream FILES ${stream_sources})
source_group(TemporaryDirectory FILES ${temporarydirectory_sources})
source_group(TemporaryFile FILES ${temporaryfile_sources})
source_group(Uri FILES ${uri_sources})
source_group(Utils FILES ${utils_sources})
source_group(VersionNumber FILES ${versionnumber_sources})
