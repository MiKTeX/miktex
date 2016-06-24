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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/packagemanager")

set(mpm_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(mpm_binary_dir ${CMAKE_CURRENT_BINARY_DIR})

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_GSOAP_DIR}/source/gsoap
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
  StdAfx.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h
)

set(public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageInstaller.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageIterator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageManager
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/PackageManager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/RepositoryInfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/PackageManager/config.h
)

set(mpm_sources
  ${public_headers}
  ${CMAKE_CURRENT_BINARY_DIR}/StdAfx.h
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebFile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebSession.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CurlWebSession.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageInstaller.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageIterator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PackageManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TpmParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TpmParser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/WebFile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/WebSession.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/WebSession.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/mpm-version.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MTREP_DIR}/RepositorySoap.nsmap
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MTREP_DIR}/repositoryC.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MTREP_DIR}/repositoryH.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MTREP_DIR}/repositoryRepositorySoapProxy.cpp
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MTREP_DIR}/repositoryRepositorySoapProxy.h
  ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_MTREP_DIR}/repositoryStub.h
  ${MIKTEX_GSOAP_SOURCE}
  ${MIKTEX_GSOAP_HEADER}
)

if(MIKTEX_NATIVE_WINDOWS AND WITH_WININET)
  list(APPEND mpm_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winWebFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winWebFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winWebSession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winWebSession.h
  )
endif()

if(INSTALL_MIKTEX_HEADERS)
  install(
    FILES
      ${public_headers}
    DESTINATION
      ${MIKTEX_HEADER_DESTINATION_DIR}/miktex/PackageManager
  )
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
