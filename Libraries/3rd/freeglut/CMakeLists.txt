## CMakeLists.txt
##
## Copyright (C) 2018-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/freeglut")

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src
)

set(public_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

add_definitions(
    -DHAVE_CONFIG_H
)

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DUNICODE
        -D_UNICODE
    )
endif()

list(APPEND freeglut_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/freeglut.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/freeglut_ext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/freeglut_std.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/freeglut_ucall.h

    ${CMAKE_CURRENT_SOURCE_DIR}/source/include/GL/glut.h

    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/fg_callback_macros.h
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
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/mswin/fg_cmap_mswin.c
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

configure_file(
    miktex-freeglut-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-freeglut-version.h
)

set(miktex_sources
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-freeglut-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex/freeglut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/miktex/freeglut.h
)

list(APPEND freeglut_sources ${miktex_sources})

if(NOT LINK_EVERYTHING_STATICALLY)
    add_subdirectory(shared)
endif()

#add_subdirectory(static)
