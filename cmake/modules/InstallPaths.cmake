## InstallPaths.cmake
##
## Copyright (C) 2006-2017 Christian Schenk
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

if(MIKTEX_NATIVE_WINDOWS)
  set(MIKTEX_TEXMF_DIR "texmf")
elseif(MIKTEX_SELF_CONTAINED)
  set(MIKTEX_TEXMF_DIR "texmfs/install")
else()
  set(MIKTEX_TEXMF_DIR "share/${MIKTEX_PREFIX}texmf")
endif()

set(MIKTEX_PORTABLE_REL_TEXMF_ROOT "texmfs")
set(MIKTEX_PORTABLE_REL_INSTALL_DIR "${MIKTEX_PORTABLE_REL_TEXMF_ROOT}/install")
set(MIKTEX_PORTABLE_REL_CONFIG_DIR "${MIKTEX_PORTABLE_REL_TEXMF_ROOT}/config")
set(MIKTEX_PORTABLE_REL_DATA_DIR "${MIKTEX_PORTABLE_REL_TEXMF_ROOT}/data")

if(MIKTEX_NATIVE_WINDOWS AND NOT CMAKE_CL_64)
  set(MIKTEX_REL_MIKTEX_BIN_DIR "miktex/bin")
elseif(MIKTEX_NATIVE_WINDOWS AND CMAKE_CL_64)
  set(MIKTEX_REL_MIKTEX_BIN_DIR "miktex/bin/x64")
else()
  set(MIKTEX_REL_MIKTEX_BIN_DIR "miktex/bin/${MIKTEX_TARGET_SYSTEM_TAG}")
endif()

set(MIKTEX_BINARY_DIR "${MIKTEX_TEXMF_DIR}/${MIKTEX_REL_MIKTEX_BIN_DIR}")

set(MIKTEX_REL_MIKTEX_INTERNAL_BIN_DIR "${MIKTEX_REL_MIKTEX_BIN_DIR}/internal")

set(MIKTEX_INTERNAL_BINARY_DIR "${MIKTEX_TEXMF_DIR}/${MIKTEX_REL_MIKTEX_INTERNAL_BIN_DIR}")

set(MIKTEX_REL_MIKTEX_TEXWORKS_PLUGINS_DIR "${MIKTEX_REL_MIKTEX_BIN_DIR}/texworks-plugins")

set(MIKTEX_TEXWORKS_PLUGINS_DIR "${MIKTEX_TEXMF_DIR}/${MIKTEX_REL_MIKTEX_TEXWORKS_PLUGINS_DIR}")

if(MIKTEX_NATIVE_WINDOWS)
  set(MIKTEX_BINARY_DESTINATION_DIR ${MIKTEX_BINARY_DIR})
  set(MIKTEX_INTERNAL_BINARY_DESTINATION_DIR ${MIKTEX_INTERNAL_BINARY_DIR})
else()
  set(MIKTEX_BINARY_DESTINATION_DIR "bin")
  #OPTION: set(MIKTEX_INTERNAL_BINARY_DESTINATION_DIR "libexec/miktex")
  set(MIKTEX_INTERNAL_BINARY_DESTINATION_DIR "lib/miktex")
endif()

set(MIKTEX_HEADER_DESTINATION_DIR "include")
set(MIKTEX_LIBRARY_DESTINATION_DIR "lib")
if(MIKTEX_SELF_CONTAINED)
  set(MIKTEX_MAN_DESTINATION_DIR "man")
else()
  set(MIKTEX_MAN_DESTINATION_DIR "share/man")
endif()

if(MIKTEX_WORDS_BIGENDIAN)
  set(MIKTEX_REL_MIKTEX_BASE_DIR "miktex/data/be")
  set(MIKTEX_REL_MIKTEX_FMT_DIR "miktex/data/be")
  set(MIKTEX_REL_MIKTEX_FNDB_DIR "miktex/data/be")
else()
  set(MIKTEX_REL_MIKTEX_BASE_DIR "miktex/data/le")
  set(MIKTEX_REL_MIKTEX_FMT_DIR "miktex/data/le")
  set(MIKTEX_REL_MIKTEX_FNDB_DIR "miktex/data/le")
endif()

set(MIKTEX_REL_DOC_DIR "doc")

set(MIKTEX_REL_MIKTEX_DOC_DIR "${MIKTEX_REL_DOC_DIR}/miktex")

set(MIKTEX_DOC_DESTINATION_DIR "${MIKTEX_TEXMF_DIR}/${MIKTEX_REL_MIKTEX_DOC_DIR}")

set(MIKTEX_REL_MIKTEX_CONFIG_DIR "miktex/config")

set(MIKTEX_CONFIG_DESTINATION_DIR "${MIKTEX_TEXMF_DIR}/${MIKTEX_REL_MIKTEX_CONFIG_DIR}")

set(MIKTEX_REL_MIKTEX_TPM_DIR "tpm/packages")

set(MIKTEX_TPM_DESTINATION_DIR "${MIKTEX_TEXMF_DIR}/${MIKTEX_REL_MIKTEX_TPM_DIR}")
