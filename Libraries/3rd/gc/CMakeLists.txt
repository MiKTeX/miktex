## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/gc")

set(enable_threads FALSE)
set(enable_parallel_mark FALSE)
set(enable_gcj_support FALSE)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(public_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
)

set(gc_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/allchblk.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/alloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/blacklst.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/checksums.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbg_mlc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dyn_load.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/finalize.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/gc_cpp.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/headers.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mach_dep.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/malloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mallocx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mark.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mark_rts.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/new_hblk.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/obj_map.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/os_dep.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/ptr_chck.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/reclaim.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/stubborn.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/thread_local_alloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/typd_mlc.c
)

add_definitions(
  -DALL_INTERIOR_POINTERS
)

if(enable_threads AND CMAKE_USE_WIN32_THREADS_INIT)
  list(APPEND gc_sources win32_threads.c)
  add_definitions(
    -DEMPTY_GETENV_RESULTS
    -DGC_THREADS
  )
  if (${enable_parallel_mark})
    add_definitions(
      -DPARALLEL_MARK
      -DTHREAD_LOCAL_ALLOC
    )
  endif()
  add_definitions()
endif()

if(enable_gcj_support)
  add_definitions(-DGC_GCJ_SUPPORT)
endif()

add_subdirectory(static)
