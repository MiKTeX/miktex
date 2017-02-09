## CreateWebApp.cmake
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

macro(create_web_app _name)

  hide_symbols()

  string(TOLOWER "${_name}" _name_l)
  string(TOUPPER "${_name}" _name_u)

  if(${ARGC} GREATER 1)
    set(_short_name ${ARGV1})
  else()
    set(_short_name ${_name})
  endif()

  string(TOLOWER "${_short_name}" _short_name_l)
  string(TOUPPER "${_short_name}" _short_name_u)

  set(_invocation_name ${MIKTEX_PREFIX}${_short_name_l})

  if(MIKTEX_CURRENT_FOLDER)
    set(_folder "${MIKTEX_CURRENT_FOLDER}")
  else()
    set(_folder ${_name})
  endif()

  set(${_short_name_l}_prog g_${_name}Prog)
  set(${_short_name_l}_progclass ${_name}Program)

  set(${_short_name_l}_app g_${_name_u}App)
  set(${_short_name_l}_appclass ${_name_u})

  if(NOT ${_short_name_l}_web_file)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}.web)
      set(${_short_name_l}_web_file
        ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}.web)
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/source/${_short_name_l}.web)
      set(${_short_name_l}_web_file
        ${CMAKE_CURRENT_SOURCE_DIR}/source/${_short_name_l}.web)
    else()
      set(${_short_name_l}_web_file
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.web)
    endif()
  endif()

  if(NOT ${_short_name_l}_change_file)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-miktex.ch)
      set(${_short_name_l}_change_file
        ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-miktex.ch)
    else()
      set(${_short_name_l}_change_file ${MIKTEX_DEV_NULL})
    endif()
  endif()

  if(NOT ${_short_name_l}_header_file)
    set(${_short_name_l}_header_file
      ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.h)
  endif()

  if(NOT ${_short_name_l}_include_file)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-miktex.h)
      set(${_short_name_l}_include_file
        ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-miktex.h)
    else()
      set(${_short_name_l}_include_file
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}-miktex.h)
      if(NOT EXISTS ${${_short_name_l}_include_file})
        file(WRITE ${${_short_name_l}_include_file}
          "#include <miktex/TeXAndFriends/WebApp>
using namespace MiKTeX::TeXAndFriends;
class ${${_short_name_l}_appclass} : public WebApp {};"
        )
      endif()
    endif()
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}main.cpp)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}main.cpp
      "#define C4PEXTERN extern
#include \"${${_short_name_l}_header_file}\"
#include \"${${_short_name_l}_include_file}\"
MIKTEX_DEFINE_WEBAPP(MiKTeX_${_name_u},
                     ${${_short_name_l}_appclass},
                     ${${_short_name_l}_app},
                     ${${_short_name_l}_progclass},
                     ${${_short_name_l}_prog})
"
    )
  endif()

  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}-miktex-config.h)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}-miktex-config.h
      "#pragma once
#define MIKTEX_${_name_u}
#define MIKTEX_${_short_name_u}
#define ${_short_name_u}PROG ${${_short_name_l}_prog}
#define ${_short_name_u}PROGCLASS ${${_short_name_l}_progclass}
#define ${_short_name_u}APP ${${_short_name_l}_app}
#define ${_short_name_u}APPCLASS ${${_short_name_l}_appclass}
"
    )
  endif()

  configure_file(
    ${MIKTEX_ALIAS_WRAPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}wrapper.cpp
    COPYONLY
  )

  if(LINK_EVERYTHING_STATICALLY)
    set(_target_name ${${_short_name_l}_lib_name})
  else()
    set(_target_name ${${_short_name_l}_dll_name})
  endif()

  list(APPEND ${_target_name}_sources
    ${${_short_name_l}_header_file}
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}-miktex-config.h
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.cc
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}defs.h
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}main.cpp
  )

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-miktex.h)
    list(APPEND ${_target_name}_sources
      ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-miktex.h
    )
  endif() 

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-version.h)
    list(APPEND ${_target_name}_sources
      ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}-version.h
    )
  endif() 
  
  set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.cc
    ${${_short_name_l}_header_file}
    PROPERTIES GENERATED TRUE
  )

  set_source_files_properties(
    ${${_short_name_l}_header_file}
    PROPERTIES
      GENERATED TRUE
      HEADER_FILE_ONLY TRUE
  )

  set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.cc
    ${${_short_name_l}_header_file}
    ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}main.cpp
    PROPERTIES COMPILE_FLAGS
      "-DMIKTEX_${_name_u} -DMIKTEX_${_short_name_u} -D${_short_name_u}PROG=${${_short_name_l}_prog} -D${_short_name_u}PROGCLASS=${${_short_name_l}_progclass} -D${_short_name_u}APP=${${_short_name_l}_app} -D${_short_name_u}APPCLASS=${${_short_name_l}_appclass}"
  )

  if(MSVC)
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}wrapper.cpp
      PROPERTIES COMPILE_FLAGS
      "-DDLLMAIN=MiKTeX_${_name_u} -DMIKTEX_${_name_u} -D_UNICODE"
    )
  else()
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}wrapper.cpp
      PROPERTIES COMPILE_FLAGS
      "-DDLLMAIN=MiKTeX_${_name_u} -DMIKTEX_${_name_u}"
    )
  endif()

  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed)
    set(_sed_script ${CMAKE_CURRENT_BINARY_DIR}/dyn.sed)
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dyn.sed)
    set(_sed_script ${CMAKE_CURRENT_SOURCE_DIR}/dyn.sed)
  else()
    set(_sed_script /dev/null)
  endif()

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.cc
      ${${_short_name_l}_header_file}
      ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}defs.h
    COMMAND
      c4p
      --def-filename=${_short_name_l}defs.h
      --dll
      --include-filename=${${_short_name_l}_include_file}
      --header-file=${${_short_name_l}_header_file}
      --one=${_short_name_l}
      --using-namespace=MiKTeX::TeXAndFriends
      -C
      --class=${_name}Program
      ${C4P_FLAGS}
      ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.p
    COMMAND
      ${SED_EXECUTABLE}
      -f ${_sed_script}
      ${${_short_name_l}_header_file}
      > tmp
    COMMAND
      ${CMAKE_COMMAND} -E copy tmp ${${_short_name_l}_header_file}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    MAIN_DEPENDENCY
      ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.p
    DEPENDS
      c4p
    VERBATIM
  )

  if(NOT ${_short_name_l} STREQUAL "tangle")
    add_custom_command(
      OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.p
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.pool
      COMMAND
        ${MIKTEX_PREFIX}tangle
        ${TANGLE_FLAGS}
        ${${_short_name_l}_web_file}
        ${${_short_name_l}_change_file}
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.p
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.pool
      MAIN_DEPENDENCY
        ${${_short_name_l}_web_file}
      DEPENDS
        ${MIKTEX_PREFIX}tangle
        ${${_short_name_l}_change_file}
      VERBATIM
    )
  endif()

  if(MIKTEX_NATIVE_WINDOWS)
    if(EXISTS ${_short_name_l}.rc)
      list(APPEND ${_target_name}_sources
        ${_short_name_l}.rc)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}.rc.in)
      configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${_short_name_l}.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.rc
      )
      list(APPEND ${_target_name}_sources
        ${CMAKE_CURRENT_BINARY_DIR}/${_short_name_l}.rc)
    endif()
  endif()

  if(LINK_EVERYTHING_STATICALLY)
    add_library(${_target_name}
      STATIC ${${_target_name}_sources})
  else()
    add_library(${_target_name}
      SHARED ${${_target_name}_sources})
    set_target_properties(${_target_name}
      PROPERTIES
        VERSION "1.${MIKTEX_J2000_VERSION}"
        SOVERSION "1"
      )
    target_link_libraries(${_target_name}
      PUBLIC
        ${app_dll_name}
        ${core_dll_name}
        ${texmf_dll_name}
        miktex-popt-wrapper
    )
    rebase(${_target_name})
  endif()

  set_property(TARGET ${_target_name} PROPERTY FOLDER ${_folder})

  if(NOT LINK_EVERYTHING_STATICALLY OR INSTALL_STATIC_LIBRARIES)
    install(
      TARGETS ${_target_name}
      RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
      LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
      ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    )
  endif()

  add_executable(${_invocation_name} ${_short_name_l}wrapper.cpp)
  set_property(TARGET ${_invocation_name} PROPERTY FOLDER ${_folder})
#  set_target_properties(${_invocation_name}
#    PROPERTIES
#      VERSION "${MIKTEX_SERIES_STR}"
#  )
  add_dependencies(${_invocation_name} ${_target_name})

  if(LINK_EVERYTHING_STATICALLY)
    target_link_libraries(${_invocation_name}
      ${core_lib_name}
      ${_target_name}
    )
  else()
    target_link_libraries(${_invocation_name}
      ${core_dll_name}
      ${_target_name}
    )
  endif()

  merge_manifests(${_invocation_name} asInvoker)

  install(
    TARGETS ${_invocation_name}
    RUNTIME DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}"
    LIBRARY DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
    ARCHIVE DESTINATION "${MIKTEX_LIBRARY_DESTINATION_DIR}"
  )
endmacro()
