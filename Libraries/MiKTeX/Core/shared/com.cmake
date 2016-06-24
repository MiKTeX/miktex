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

set(sessionidl_h MiKTeX${MIKTEX_SERIES_INT}-sessionidl.h)
set(session_idl MiKTeX${MIKTEX_SERIES_INT}-session.idl)

configure_file(
  ${core_source_dir}/COM/session.idl.in
  ${CMAKE_CURRENT_BINARY_DIR}/${session_idl}
)

configure_file(
  ${core_source_dir}/COM/session.rgs.in
  ${CMAKE_CURRENT_BINARY_DIR}/session.rgs
)

configure_file(
  ${core_source_dir}/COM/comSession.rgs.in
  ${CMAKE_CURRENT_BINARY_DIR}/comSession.rgs
)

set(generated_core_com_sources
  ${CMAKE_CURRENT_BINARY_DIR}/session_i.c
  ${core_binary_dir}/include/${session_tlb_name}.tlb
  ${core_binary_dir}/include/miktexidl.h
  ${core_source_dir}/COM/comSession.cpp
  ${core_source_dir}/COM/comSession.h
  ${core_source_dir}/COM/session.cpp
  ${core_source_dir}/COM/session.h
)

set(core_com_sources
  ${core_source_dir}/COM/session.def
  ${generated_core_com_sources}
)

set(generated_core_ps_sources
  ${CMAKE_CURRENT_BINARY_DIR}/dlldata.c
  ${CMAKE_CURRENT_BINARY_DIR}/session_i.c
  ${CMAKE_CURRENT_BINARY_DIR}/session_p.c
  ${core_binary_dir}/include/${sessionidl_h}
)

set(core_ps_sources
  ${core_source_dir}/shared/ps/ProxyStub.def
  ${generated_core_ps_sources}
)

if(CMAKE_CL_64)
  set(env "amd64")
else()
  set(env "win32")
endif()

file(MAKE_DIRECTORY ${core_binary_dir}/include)

add_custom_command(
  OUTPUT
    ${core_binary_dir}/include/miktexidl.h
  COMMAND
    ${MIDL_EXECUTABLE}
    /nologo
    /I ${core_source_dir}/include
    /char signed
    /env ${env}
    /Oicf
    /h ${core_binary_dir}/include/miktexidl.h
    /no_robust
    ${core_source_dir}/include/miktexidl.idl
  DEPENDS
    ${core_source_dir}/include/miktexidl.idl
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/dlldata.c
    ${CMAKE_CURRENT_BINARY_DIR}/session_i.c
    ${CMAKE_CURRENT_BINARY_DIR}/session_p.c
    ${core_binary_dir}/include/${session_tlb_name}.tlb
    ${core_binary_dir}/include/${sessionidl_h}
  COMMAND
    ${MIDL_EXECUTABLE}
    /nologo
    /I ${core_source_dir}/include
    /char signed
    /env ${env}
    /Oicf
    /tlb ${core_binary_dir}/include/${session_tlb_name}.tlb
    /h ${core_binary_dir}/include/${sessionidl_h}
    /iid ${CMAKE_CURRENT_BINARY_DIR}/session_i.c
    /proxy ${CMAKE_CURRENT_BINARY_DIR}/session_p.c
    /no_robust
    ${CMAKE_CURRENT_BINARY_DIR}/${session_idl}
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/${session_idl}
    ${core_source_dir}/include/miktexidl.idl
  VERBATIM
)

add_custom_target(gen-core-com
  DEPENDS
    ${generated_core_com_sources}
    ${generated_core_ps_sources}
)

set_property(TARGET gen-core-com PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

source_group(COM FILES ${core_com_sources})
