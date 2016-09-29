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

hide_symbols()

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(WITH_COM)
  include(com.cmake)
  add_subdirectory(ps)
endif()

set(${core_dll_name}_sources
  ${core_com_sources}
  ${core_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    ../core.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/core.rc
  )
  list(APPEND ${core_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/core.rc
    ${core_source_dir}/Resource.h
  )
  list(APPEND resource_files
    ${CMAKE_CURRENT_BINARY_DIR}/core.rc
    ${core_source_dir}/Resource.h
  )
endif()

add_library(${core_dll_name} SHARED ${${core_dll_name}_sources})

set_property(TARGET ${core_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

add_dependencies(${core_dll_name} gen-core-sources)

set_source_files_properties(
  ${generated_core_sources}
  PROPERTIES
    GENERATED TRUE
)

set_target_properties(${core_dll_name}
  PROPERTIES
    VERSION "${MIKTEX_COMP_INTERFACE_VERSION}.${MIKTEX_COMP_J2000_VERSION}"
    SOVERSION "${MIKTEX_COMP_INTERFACE_VERSION}"
)

target_compile_definitions(${core_dll_name}
  PUBLIC
    -DMIKTEX_CORE_SHARED
)

target_include_directories(${core_dll_name}
  PUBLIC
    ${public_include_directories}
)

target_link_libraries(${core_dll_name}
  PRIVATE
    ${botan_dll_name}
    ${bzip2_dll_name}
    ${lzma_dll_name}
    ${uriparser_dll_name}
    ${zlib_dll_name}
    Threads::Threads
  PUBLIC
    ${md5_dll_name}
    ${trace_dll_name}
    ${util_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${core_dll_name}
    PRIVATE
      SHFolder.lib
      shlwapi.lib
  )
  if(MSVC)
    target_link_libraries(${core_dll_name}
      PRIVATE
        htmlhelp.lib
    )
  endif()
endif()

if(MIKTEX_NATIVE_WINDOWS OR USE_SYSTEM_OPENSSL)
  target_link_libraries(${core_dll_name}
    PRIVATE
      ${openssl_crypto_dll_name}
  )
endif()

delay_load(${core_dll_name}
  ${bzip2_dll_name}
  ${lzma_dll_name}
  ${uriparser_dll_name}
  ${zlib_dll_name}
)

rebase(${core_dll_name})

install(TARGETS ${core_dll_name}
  RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
  LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
)

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
source_group(Resources FILES ${resource_files})
source_group(Session FILES ${session_sources})
source_group(Stream FILES ${stream_sources})
source_group(TemporaryDirectory FILES ${temporarydirectory_sources})
source_group(TemporaryFile FILES ${temporaryfile_sources})
source_group(Uri FILES ${uri_sources})
source_group(Utils FILES ${utils_sources})
source_group(VersionNumber FILES ${versionnumber_sources})
