## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2010-2016 Christian Schenk
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
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mp
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mp.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmath.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmath.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmath
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmath.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathbinary.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathbinary
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmathbinary.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdecimal.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathdecimal
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmathdecimal.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpmathdouble.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpmathdouble
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpmathdouble.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpstrings.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpstrings
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpstrings.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/pngout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mppngout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibpng.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/pngout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/pngout.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/psout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mppsout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibps.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/psout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/psout.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/svgout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpsvgout.h
    ${CMAKE_CURRENT_BINARY_DIR}/mplibsvg.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/svgout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/svgout.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/tfmin.c
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/tfmin
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/tfmin.w
  DEPENDS
    ctangle
  VERBATIM
)

set(mplib_sources
  ${mplib_cweb_sources}
  source/avl.c
  source/avl.h
  source/decContext.c
  source/decContext.h
  source/decNumber.c
  source/decNumber.h
  source/decNumberLocal.h
)

add_library(${mp_lib_name} STATIC ${mplib_sources})

set_property(TARGET ${mp_lib_name} PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_include_directories(${mp_lib_name}
  PUBLIC
    ${CMAKE_BINARY_DIR}/${MIKTEX_REL_MP_DIR}
)

if(USE_SYSTEM_CAIRO)
  target_link_libraries(${mp_lib_name} PUBLIC MiKTeX::Imported::CAIRO)
else()
  target_link_libraries(${mp_lib_name} PUBLIC ${cairo_dll_name})
endif()

if(USE_SYSTEM_GMP)
  target_link_libraries(${mp_lib_name} PUBLIC MiKTeX::Imported::GMP)
else()
  target_link_libraries(${mp_lib_name} PUBLIC ${gmp_lib_name})
endif()

if(USE_SYSTEM_MPFR)
  target_link_libraries(${mp_lib_name} PUBLIC MiKTeX::Imported::MPFR)
else()
  target_link_libraries(${mp_lib_name} PUBLIC ${mpfr_lib_name})
endif()

if(USE_SYSTEM_PIXMAN)
  target_link_libraries(${mp_lib_name} PUBLIC MiKTeX::Imported::PIXMAN)
else()
  target_link_libraries(${mp_lib_name} PUBLIC ${pixman_lib_name})
endif()

if(USE_SYSTEM_PNG)
  target_link_libraries(${mp_lib_name} PUBLIC MiKTeX::Imported::PNG)
else()
  target_link_libraries(${mp_lib_name} PUBLIC ${png_dll_name})
endif()

target_link_libraries(${mp_lib_name}
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
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpost
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpost.w
  DEPENDS
    ctangle
  VERBATIM
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/mpxout.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpxout.h
  COMMAND
    ctangle ${CMAKE_CURRENT_SOURCE_DIR}/source/mpxout
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
  MAIN_DEPENDENCY
    source/mpxout.w
  DEPENDS
    ctangle
  VERBATIM
)

set(mpost_sources
  ${mpost_cweb_sources}
  ${MIKTEX_LIBRARY_WRAPPER}
  mpost-version.h
)

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND mpost_sources mpost.rc)
endif()

add_executable(${MIKTEX_PREFIX}mpost ${mpost_sources})

set_property(TARGET ${MIKTEX_PREFIX}mpost PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER})

target_link_libraries(${MIKTEX_PREFIX}mpost
  ${app_dll_name}
  ${core_dll_name}
  ${kpsemu_dll_name}
  ${mp_lib_name}
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
