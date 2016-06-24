## CMakeLists.txt					-*- CMake -*-
##
## Copyright (C) 2015-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/log4cxx")

set(apr_have_ipv6_10 1)
set(win32_winnt_str "0x0501")

add_definitions(
  -DWINNT
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/arch/unix
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/arch/win32
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/private
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
  include/apr_allocator.h
  include/apr_atomic.h
  include/apr_dso.h
  include/apr_env.h
  include/apr_errno.h
  include/apr_escape.h
  include/apr_file_info.h
  include/apr_file_io.h
  include/apr_fnmatch.h
  include/apr_general.h
  include/apr_getopt.h
  include/apr_global_mutex.h
  include/apr_hash.h
  include/apr_inherit.h
  include/apr_lib.h
  include/apr_mmap.h
  include/apr_network_io.h
  include/apr_poll.h
  include/apr_pools.h
  include/apr_portable.h
  include/apr_proc_mutex.h
  include/apr_random.h
  include/apr_ring.h
  include/apr_shm.h
  include/apr_signal.h
  include/apr_skiplist.h
  include/apr_strings.h
  include/apr_support.h
  include/apr_tables.h
  include/apr_thread_cond.h
  include/apr_thread_mutex.h
  include/apr_thread_proc.h
  include/apr_thread_rwlock.h
  include/apr_time.h
  include/apr_user.h
  include/apr_version.h
  include/apr_want.h
)

foreach(h ${public_headers})
  configure_file(
    source/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/${h}
    COPYONLY
  )
endforeach()

configure_file(
  source/include/apr.hwc
  ${CMAKE_CURRENT_BINARY_DIR}/include/apr.h
)

add_executable(gen_test_char source/tools/gen_test_char.c)

add_custom_command(
  COMMAND
    gen_test_char > ${CMAKE_CURRENT_BINARY_DIR}/apr_escape_test_char.h
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/apr_escape_test_char.h
  DEPENDS
    gen_test_char
)

set(generated_headers
  ${CMAKE_CURRENT_BINARY_DIR}/apr_escape_test_char.h
)

add_custom_target(
  gen-apr-headers
  DEPENDS ${generated_headers}
)

set_property(TARGET gen-apr-headers PROPERTY FOLDER ${MIKTEX_CURRENT_FOLDER}/gen)

set(apr_sources
  ${generated_headers}

  ${CMAKE_CURRENT_SOURCE_DIR}/source/atomic/win32/apr_atomic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dso/win32/dso.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/encoding/apr_escape.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/unix/copy.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/unix/fileacc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/unix/filepath_util.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/unix/fullrw.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/unix/mktemp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/unix/tempdir.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/buffer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/dir.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/filedup.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/filepath.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/filestat.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/filesys.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/flock.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/open.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/pipe.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/readwrite.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/file_io/win32/seek.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/locks/win32/proc_mutex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/locks/win32/thread_cond.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/locks/win32/thread_mutex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/locks/win32/thread_rwlock.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/memory/unix/apr_pools.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/unix/errorcodes.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/unix/getopt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/unix/otherchild.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/unix/version.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/win32/charset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/win32/env.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/win32/internal.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/win32/misc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/win32/rand.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/win32/start.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/win32/utf8.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mmap/unix/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/mmap/win32/mmap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/unix/inet_ntop.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/unix/inet_pton.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/unix/multicast.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/unix/sockaddr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/unix/socket_util.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/win32/sendrecv.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/win32/sockets.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/network_io/win32/sockopt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/passwd/apr_getpass.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poll/unix/poll.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poll/unix/pollcb.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poll/unix/pollset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poll/unix/select.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/random/unix/apr_random.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/random/unix/sha2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/random/unix/sha2_glue.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/shmem/win32/shm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/strings/apr_cpystrn.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/strings/apr_fnmatch.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/strings/apr_snprintf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/strings/apr_strings.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/strings/apr_strnatcmp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/strings/apr_strtok.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/tables/apr_hash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/tables/apr_skiplist.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/tables/apr_tables.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/threadproc/win32/proc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/threadproc/win32/signals.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/threadproc/win32/thread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/threadproc/win32/threadpriv.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/time/win32/time.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/time/win32/timestr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/user/win32/groupinfo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/user/win32/userinfo.c
)

ignore_warnings()

if(MIKTEX_NATIVE_WINDOWS)
  list(APPEND system_libraries
    ws2_32
    mswsock
    rpcrt4
  )
endif()

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
