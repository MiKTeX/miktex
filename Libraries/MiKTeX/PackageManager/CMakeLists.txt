## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2019 Christian Schenk
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

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/packagemanager")

set(mpm_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(mpm_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_definitions(
  -DHAVE_LIBCURL
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  mpm-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/mpm-version.h
)

configure_file(
  include/miktex/PackageManager/vi/Version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/PackageManager/vi/Version.h
)

set(public_headers
  ${CMAKE_CURRENT_BINARY_DIR}/include/miktex/PackageManager/vi/Version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageInstaller.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageIterator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageManager
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageManager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/RepositoryInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/definitions.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/vi/Version
)

set(mpm_sources
  ${public_headers}
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpm-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ComboCfg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ComboCfg.h
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebFile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebSession.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebSession.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ExpatTpmParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ExpatTpmParser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/NoRemoteService.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageDataStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageDataStore.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageInstallerImpl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageInstallerImpl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageIteratorImpl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageIteratorImpl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageManagerImpl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageManagerImpl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageRepositoryDataStore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageRepositoryDataStore.h
  ${CMAKE_CURRENT_SOURCE_DIR}/RemoteService.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/RemoteService.h
  ${CMAKE_CURRENT_SOURCE_DIR}/RepositoryManifest.h
  ${CMAKE_CURRENT_SOURCE_DIR}/RestRemoteService.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/RestRemoteService.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TpmParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TpmParser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/WebFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/WebSession.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/WebSession.h
  ${CMAKE_CURRENT_SOURCE_DIR}/exceptions.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/text.h
  ${CMAKE_CURRENT_SOURCE_DIR}/vi/Runtime.cpp
)

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      ${public_headers}
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/PackageManager
  )
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
