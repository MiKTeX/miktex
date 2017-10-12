## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2010-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_TEX_AND_FRIENDS_FOLDER}/MetaPost")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)

add_definitions(
  -DAPPTAG=mpost
  -DUSE_MIKTEX_EXIT
)

if(MIKTEX_NATIVE_WINDOWS)
  add_definitions(
    -DUNICODE
    -D_UNICODE
  )
endif()

ignore_warnings()

set(mplib_cweb_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mp.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpmath.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.c
  ${CMAKE_CURRENT_BINARY_DIR}/pngout.c
  ${CMAKE_CURRENT_BINARY_DIR}/psout.c
  ${CMAKE_CURRENT_BINARY_DIR}/svgout.c
  ${CMAKE_CURRENT_BINARY_DIR}/tfmin.c

  ${CMAKE_CURRENT_BINARY_DIR}/mplib.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpmp.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpmath.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.h
  ${CMAKE_CURRENT_BINARY_DIR}/mppngout.h
  ${CMAKE_CURRENT_BINARY_DIR}/mplibpng.h
  ${CMAKE_CURRENT_BINARY_DIR}/mppsout.h
  ${CMAKE_CURRENT_BINARY_DIR}/mplibps.h
  ${CMAKE_CURRENT_BINARY_DIR}/mpsvgout.h
  ${CMAKE_CURRENT_BINARY_DIR}/mplibsvg.h
)

set_source_files_properties(
  ${mplib_cweb_sources}
  PROPERTIES GENERATED TRUE
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mp.c
    ${CMAKE_CURRENT_BINARY_DIR}/mplib.h
    ${CMAKE_CURRENT_BINARY_DIR}/mpmp.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mp.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmath.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmath.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmath
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmath.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathbinary
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmathbinary.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathdecimal
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmathdecimal.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathdouble
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmathdouble.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpstrings
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpstrings.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/pngout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mppngout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibpng.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/pngout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/pngout.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/psout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mppsout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibps.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/psout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/psout.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/svgout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpsvgout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibsvg.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/svgout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/svgout.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tfmin.c
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/tfmin
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/tfmin.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

configure_file(
  metapost-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/metapost-version.h
)

set(mplib_sources
  ${CMAKE_CURRENT_BINARY_DIR}/metapost-version.h
  ${mplib_cweb_sources}
  source/avl.c
  source/avl.h
  source/decContext.c
  source/decContext.h
  source/decNumber.c
  source/decNumber.h
  source/decNumberLocal.h
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    mplib.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mplib.rc
  )
  list(APPEND mpllib_sources mplib.rc)
endif()

add_library(${metapost_dll_name} SHARED ${mplib_sources})

set_property(TARGET ${metapost_dll_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

set_property(TARGET ${metapost_dll_name} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  
target_include_directories(${metapost_dll_name}
  PUBLIC
    ${CMAKE_BINARY_DIR}/${MIKTEX_REL_MP_DIR}
)

if(USE_SYSTEM_CAIRO)
  target_link_libraries(${metapost_dll_name} PUBLIC MiKTeX::Imported::CAIRO)
else()
  target_link_libraries(${metapost_dll_name} PUBLIC ${cairo_dll_name})
endif()

if(USE_SYSTEM_GMP)
  target_link_libraries(${metapost_dll_name} PUBLIC MiKTeX::Imported::GMP)
else()
  target_link_libraries(${metapost_dll_name} PUBLIC ${gmp_lib_name})
endif()

if(USE_SYSTEM_MPFR)
  target_link_libraries(${metapost_dll_name} PUBLIC MiKTeX::Imported::MPFR)
else()
  target_link_libraries(${metapost_dll_name} PUBLIC ${mpfr_lib_name})
endif()

if(USE_SYSTEM_PIXMAN)
  target_link_libraries(${metapost_dll_name} PUBLIC MiKTeX::Imported::PIXMAN)
else()
  target_link_libraries(${metapost_dll_name} PUBLIC ${pixman_lib_name})
endif()

if(USE_SYSTEM_PNG)
  target_link_libraries(${metapost_dll_name} PUBLIC MiKTeX::Imported::PNG)
else()
  target_link_libraries(${metapost_dll_name} PUBLIC ${png_dll_name})
endif()

target_link_libraries(${metapost_dll_name}
  PUBLIC
    ${core_dll_name}
    ${kpsemu_dll_name}
    ${w2cemu_dll_name}
)

set(mpost_cweb_sources
  ${CMAKE_CURRENT_BINARY_DIR}/mpost.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpxout.c
  ${CMAKE_CURRENT_BINARY_DIR}/mpxout.h
)

set_source_files_properties(
  ${mpost_cweb_sources}
  PROPERTIES GENERATED TRUE
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpost.c
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpost
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpost.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpxout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpxout.h
  COMMAND
    ${MIKTEX_PREFIX}ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpxout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpxout.w
  DEPENDS
    ${MIKTEX_PREFIX}ctangle
  VERBATIM
)

set(mpost_sources
  ${CMAKE_CURRENT_BINARY_DIR}/metapost-version.h
  ${MIKTEX_LIBRARY_WRAPPER}
  ${mpost_cweb_sources}
)

if(MIKTEX_NATIVE_WINDOWS)
  configure_file(
    mpost.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mpost.rc
  )
  list(APPEND mpost_sources mpost.rc)
endif()

add_executable(${MIKTEX_PREFIX}mpost ${mpost_sources})

set_property(TARGET ${MIKTEX_PREFIX}mpost PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}mpost
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${metapost_dll_name}
  ${w2cemu_dll_name}
)

if(MIKTEX_NATIVE_WINDOWS)
  target_link_libraries(${MIKTEX_PREFIX}mpost
    ${getopt_dll_name}
    ${utf8wrap_dll_name}
  )
endif()

merge_trustinfo_manifest(${MIKTEX_PREFIX}mpost asInvoker)
merge_compatibility_manifest(${MIKTEX_PREFIX}mpost)

install(TARGETS ${MIKTEX_PREFIX}mpost DESTINATION "${MIKTEX_BINARY_DESTINATION_DIR}")
