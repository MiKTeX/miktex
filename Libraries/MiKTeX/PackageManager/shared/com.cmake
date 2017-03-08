## CMakeLists.txt					-*- CMake -*-
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

add_definitions(
  -DUSE_LOCAL_SERVER
  -D_USRDLL
  -D_WINDLL
)

set(mpmidl_h MiKTeX${MIKTEX_MAJOR_MINOR_INT}-packagemanageridl.h)
set(mpm_idl MiKTeX${MIKTEX_MAJOR_MINOR_INT}-packagemanager.idl)

configure_file(
  ${mpm_source_dir}/COM/mpm.idl.in
  ${CMAKE_CURRENT_BINARY_DIR}/${mpm_idl}
)

configure_file(
  ${mpm_source_dir}/COM/mpm.rgs.in
  ${CMAKE_CURRENT_BINARY_DIR}/mpm.rgs
)

configure_file(
  ${mpm_source_dir}/COM/comPackageManager.rgs.in
  ${CMAKE_CURRENT_BINARY_DIR}/comPackageManager.rgs
)

set(generated_mpm_com_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mpm_i.c
  ${mpm_binary_dir}/include/${mpm_tlb_name}.tlb
  ${mpm_source_dir}/COM/comPackageInstaller.cpp
  ${mpm_source_dir}/COM/comPackageInstaller.h
  ${mpm_source_dir}/COM/comPackageIterator.cpp
  ${mpm_source_dir}/COM/comPackageIterator.h
  ${mpm_source_dir}/COM/comPackageManager.cpp
  ${mpm_source_dir}/COM/comPackageManager.h
  ${mpm_source_dir}/COM/mpm.cpp
)

set(mpm_com_sources
  ${generated_mpm_com_sources}
  ${mpm_source_dir}/COM/mpm.def
)

set(generated_mpm_ps_sources
  ${CMAKE_CURRENT_BINARY_DIR}/dlldata.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpm_i.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpm_p.c
  ${mpm_binary_dir}/include/${mpmidl_h}
)

set(mpm_ps_sources
  ${mpm_source_dir}/shared/ps/ProxyStub.def
  ${generated_mpm_ps_sources}
)

if(CMAKE_CL_64)
  set(env "amd64")
else()
  set(env "win32")
endif()

file(MAKE_DIRECTORY ${mpm_binary_dir}/include)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/dlldata.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpm_i.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpm_p.c
    ${mpm_binary_dir}/include/${mpm_tlb_name}.tlb
    ${mpm_binary_dir}/include/${mpmidl_h}
  COMMAND
    ${MIDL_EXECUTABLE}
    /nologo
    /I ${CMAKE_SOURCE_DIR}/${MIKTEX_REL_CORE_DIR}/include
    /char signed
    /env ${env}
    /Oicf
    /tlb ${mpm_binary_dir}/include/${mpm_tlb_name}.tlb
    /h ${mpm_binary_dir}/include/${mpmidl_h}
    /iid ${CMAKE_CURRENT_BINARY_DIR}/mpm_i.c
    /proxy ${CMAKE_CURRENT_BINARY_DIR}/mpm_p.c
    /no_robust
    ${CMAKE_CURRENT_BINARY_DIR}/${mpm_idl}
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/${mpm_idl}
  VERBATIM
)

add_custom_target(gen-mpm-com
  DEPENDS
    ${generated_mpm_com_sources}
    ${generated_mpm_ps_sources}
)

set_property(TARGET gen-mpm-com PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})
