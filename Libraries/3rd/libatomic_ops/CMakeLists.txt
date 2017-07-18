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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/libatomic_ops")

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MIKTEX_NATIVE_WINDOWS)
  set(compile_definitions -DAO_ASSUME_WINDOWS98)
endif()

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
  atomic_ops/ao_version.h
  atomic_ops/generalize-arithm.h
  atomic_ops/generalize-small.h
  atomic_ops/generalize.h
  atomic_ops/sysdeps/all_acquire_release_volatile.h
  atomic_ops/sysdeps/all_aligned_atomic_load_store.h
  atomic_ops/sysdeps/all_atomic_load_store.h
  atomic_ops/sysdeps/all_atomic_only_load.h
  atomic_ops/sysdeps/ao_t_is_int.h
  atomic_ops/sysdeps/armcc/arm_v6.h
  atomic_ops/sysdeps/emul_cas.h
  atomic_ops/sysdeps/gcc/aarch64.h
  atomic_ops/sysdeps/gcc/alpha.h
  atomic_ops/sysdeps/gcc/arm.h
  atomic_ops/sysdeps/gcc/avr32.h
  atomic_ops/sysdeps/gcc/cris.h
  atomic_ops/sysdeps/gcc/generic-arithm.h
  atomic_ops/sysdeps/gcc/generic-small.h
  atomic_ops/sysdeps/gcc/generic.h
  atomic_ops/sysdeps/gcc/hexagon.h
  atomic_ops/sysdeps/gcc/hppa.h
  atomic_ops/sysdeps/gcc/ia64.h
  atomic_ops/sysdeps/gcc/m68k.h
  atomic_ops/sysdeps/gcc/mips.h
  atomic_ops/sysdeps/gcc/powerpc.h
  atomic_ops/sysdeps/gcc/s390.h
  atomic_ops/sysdeps/gcc/sh.h
  atomic_ops/sysdeps/gcc/sparc.h
  atomic_ops/sysdeps/gcc/x86.h
  atomic_ops/sysdeps/generic_pthread.h
  atomic_ops/sysdeps/hpc/hppa.h
  atomic_ops/sysdeps/hpc/ia64.h
  atomic_ops/sysdeps/ibmc/powerpc.h
  atomic_ops/sysdeps/icc/ia64.h
  atomic_ops/sysdeps/loadstore/acquire_release_volatile.h
  atomic_ops/sysdeps/loadstore/atomic_load.h
  atomic_ops/sysdeps/loadstore/atomic_store.h
  atomic_ops/sysdeps/loadstore/char_acquire_release_volatile.h
  atomic_ops/sysdeps/loadstore/char_atomic_load.h
  atomic_ops/sysdeps/loadstore/char_atomic_store.h
  atomic_ops/sysdeps/loadstore/double_atomic_load_store.h
  atomic_ops/sysdeps/loadstore/int_acquire_release_volatile.h
  atomic_ops/sysdeps/loadstore/int_atomic_load.h
  atomic_ops/sysdeps/loadstore/int_atomic_store.h
  atomic_ops/sysdeps/loadstore/ordered_loads_only.h
  atomic_ops/sysdeps/loadstore/ordered_stores_only.h
  atomic_ops/sysdeps/loadstore/short_acquire_release_volatile.h
  atomic_ops/sysdeps/loadstore/short_atomic_load.h
  atomic_ops/sysdeps/loadstore/short_atomic_store.h
  atomic_ops/sysdeps/msftc/arm.h
  atomic_ops/sysdeps/msftc/common32_defs.h
  atomic_ops/sysdeps/msftc/x86.h
  atomic_ops/sysdeps/msftc/x86_64.h
  atomic_ops/sysdeps/ordered.h
  atomic_ops/sysdeps/ordered_except_wr.h
  atomic_ops/sysdeps/read_ordered.h
  atomic_ops/sysdeps/standard_ao_double_t.h
  atomic_ops/sysdeps/sunc/sparc.h
  atomic_ops/sysdeps/sunc/x86.h
  atomic_ops/sysdeps/test_and_set_t_is_ao_t.h
  atomic_ops/sysdeps/test_and_set_t_is_char.h
  atomic_ops.h
  atomic_ops_malloc.h
  atomic_ops_stack.h
)

set(libatomic_ops_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atomic_ops.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atomic_ops_malloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/atomic_ops_stack.c
)

configure_file(
  config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

list(APPEND libatomic_ops_sources ${CMAKE_CURRENT_BINARY_DIR}/config.h)

foreach(h ${public_headers})
  configure_file(
    source/src/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
  list(APPEND libatomic_ops_sources ${CMAKE_CURRENT_SOURCE_DIR}/source/src/${h})
endforeach()

add_subdirectory(static)
