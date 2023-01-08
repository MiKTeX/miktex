/**
 * @file config.h
 * @author Christian Schenk
 * @brief Internal configuration
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include <miktex/First>

#cmakedefine HAVE_ATLBASE_H 1
#cmakedefine HAVE_DIRENT_H 1
#cmakedefine HAVE_INTTYPES_H 1
#cmakedefine HAVE_SYS_MMAN_H 1
#cmakedefine HAVE_SYS_STATVFS_H 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_TIME_H 1
#cmakedefine HAVE_SYS_UTIME_H 1
#cmakedefine HAVE_SYS_UTSNAME_H 1
#cmakedefine HAVE_SYS_WAIT_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_UTIME_H 1

#cmakedefine HAVE_CHOWN 1
#cmakedefine HAVE_CONFSTR 1
#cmakedefine HAVE_FORK 1
#cmakedefine HAVE_FUTIMES 1
#cmakedefine HAVE_MMAP 1
#cmakedefine HAVE_STATVFS 1
#cmakedefine HAVE_UNAME_SYSCALL 1
#cmakedefine HAVE_VFORK 1

#cmakedefine HAVE_STRUCT_DIRENT_D_TYPE 1

#cmakedefine CMAKE_USE_PTHREADS_INIT ${CMAKE_USE_PTHREADS_INIT}

#if defined(CMAKE_USE_PTHREADS_INIT) && CMAKE_USE_PTHREADS_INIT
#  define HAVE_PTHREAD 1
#endif

#cmakedefine REPORT_EVENTS 1

#cmakedefine USE_SYSTEM_OPENSSL_CRYPTO 1
#cmakedefine WITH_LIBRESSL_CRYPTO 1

#if defined(WITH_LIBRESSL_CRYPTO) || defined(USE_SYSTEM_OPENSSL_CRYPTO)
#  define ENABLE_OPENSSL 1
#endif

#define MIKTEX_SESSION_TLB "${session_tlb_name}.tlb"

#define MIKTEX_SOURCE_DIR "${CMAKE_SOURCE_DIR}"
#define MIKTEX_BINARY_DIR "${CMAKE_BINARY_DIR}"

#define MIKTEX_BINARY_DESTINATION_DIR "${MIKTEX_BINARY_DESTINATION_DIR}"
#define MIKTEX_INTERNAL_BINARY_DESTINATION_DIR "${MIKTEX_INTERNAL_BINARY_DESTINATION_DIR}"

#if defined(MIKTEX_MACOS_BUNDLE)
#  define MIKTEX_MACOS_DESTINATION_DIR "@MIKTEX_MACOS_DESTINATION_DIR@"
#endif

#cmakedefine MIKTEX_FNDB_VERSION @MIKTEX_FNDB_VERSION@

#define EAD86981C92C904D808A5E6CEC64B90E

#if defined(MIKTEX_CORE_SHARED)
#  define MIKTEXCOREEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXCOREEXPORT
#endif
