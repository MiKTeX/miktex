## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2018 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/freeglut")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

add_definitions(
  -DHAVE_CONFIG_H
)

configure_file(
  freeglut-version.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/freeglut-version.h
)

list(APPEND freeglut_sources
  ${CMAKE_CURRENT_BINARY_DIR}/freeglut-version.h
)

list(APPEND freeglut_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/freeglut.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/freeglut_ext.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/freeglut_std.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/glut.h

  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_callbacks.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_cursor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_display.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_ext.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_font_data.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_gamemode.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_geometry.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_gl2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_gl2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_init.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_init.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_input_devices.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_joystick.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_misc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_overlay.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_spaceball.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_state.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_stroke_mono_roman.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_stroke_roman.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_structure.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_teapot.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_teapot_data.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_videoresize.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_window.c
)

if(NOT HAVE_XPARSEGEOMETRY)
  list(APPEND freeglut_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/xparsegeometry_repl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/util/xparsegeometry_repl.h)
  set(NEED_XPARSEGEOMETRY_IMPL TRUE)
endif()

if(NOT FREEGLUT_GLES)
  list(APPEND freeglut_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_menu.c
  )
else()
  list(APPEND freeglut_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/gles_stubs.c
  )
endif()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND freeglut_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_cursor_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_display_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_ext_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_gamemode_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_init_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_internal_mswin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_input_devices_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_joystick_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_main_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_menu_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_spaceball_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_state_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_structure_mswin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_window_mswin.c
  )
endif()

configure_file(
  source/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

list(APPEND freeglut_sources
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

#add_subdirectory(static)
