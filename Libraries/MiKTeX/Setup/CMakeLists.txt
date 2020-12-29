## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2013-2020 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_MIKTEX_LIBRARIES_FOLDER}/setup")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService.h
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
  setup-version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/setup-version.h
)

set(setup_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
  ${CMAKE_CURRENT_BINARY_DIR}/setup-version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LogFile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SetupService.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/miktex/Setup/SetupService.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internal.h
  ${public_headers}
)

list(APPEND l_sources LogFile.cpp SetupService.cpp)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND setup_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winSetupService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win/winSetupService.h
  )
  list(APPEND l_sources win/winSetupService.cpp)
else()
  list(APPEND setup_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/unx/unxSetupService.cpp
  )
endif()

add_custom_target(setuplib-pot
  COMMAND
    ${XGETTEXT_EXECUTABLE} --join-existing --keyword=T_ --from-code=UTF-8 --add-comments --output=po/setuplib.pot ${l_sources}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set_property(TARGET setuplib-pot PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

add_custom_target(gen-setuplib-translations)
set_property(TARGET gen-setuplib-translations PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
add_dependencies(gen-setuplib-translations setuplib-pot)

add_custom_target(setuplib-mo ALL)
set_property(TARGET setuplib-mo PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

foreach(l ${MIKTEX_TRANSLATION_LOCALES})
  add_custom_target(setuplib-po-${l}
    COMMAND
      ${MSGMERGE_EXECUTABLE} --update --backup=none po/${l}/setuplib.po po/setuplib.pot
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}
  )
  set_property(TARGET setuplib-po-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
  add_dependencies(gen-setuplib-translations setuplib-po-${l})
  add_custom_target(setuplib-mo-${l} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/setuplib.mo)
  set_property(TARGET setuplib-mo-${l} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)
  add_dependencies(setuplib-mo setuplib-mo-${l})
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/setuplib.mo
    COMMAND
      ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}
    COMMAND
      ${MSGFMT_EXECUTABLE} --output-file=locale/${l}/setuplib.mo ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/setuplib.po
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/po/${l}/setuplib.po
  )
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/setuplib_${l}_mo.h
    COMMAND
      bintoc setuplib_${l}_mo
      < ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/setuplib.mo
      > ${CMAKE_CURRENT_BINARY_DIR}/setuplib_${l}_mo.h.tmp
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_BINARY_DIR}/setuplib_${l}_mo.h.tmp
      ${CMAKE_CURRENT_BINARY_DIR}/setuplib_${l}_mo.h
    COMMAND
      ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/setuplib_${l}_mo.h.tmp
    DEPENDS
      bintoc
      ${CMAKE_CURRENT_BINARY_DIR}/locale/${l}/setuplib.mo
    VERBATIM
  )
  list(APPEND generated_setuplib_sources ${CMAKE_CURRENT_BINARY_DIR}/setuplib_${l}_mo.h)
  set(resource_includes
    "${resource_includes}#include \"${CMAKE_CURRENT_BINARY_DIR}/setuplib_${l}_mo.h\""
  )
  set(resource_adders
    "${resource_adders}addResource(\"/i18n/${l}/LC_MESSAGES/setuplib.mo\", { setuplib_${l}_mo, sizeof(setuplib_${l}_mo) });"
  )
endforeach()

configure_file(
  ResourceRepository.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/ResourceRepository.cpp
)

add_custom_target(gen-setuplib-sources DEPENDS
  ${generated_setuplib_sources}
)

set_property(TARGET gen-setuplib-sources PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

list(APPEND setup_sources
  ${CMAKE_CURRENT_BINARY_DIR}/ResourceRepository.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ResourceRepository.h
  ${generated_setuplib_sources}
)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
