 /**
 * @file miktex/utf8wrap-config.h
 * @author Christian Schenk
 * @brief Configuration header file
 *
 * @copyright Copyright © 2011-2022 Christian Schenk
 *
 * This file is part of the MiKTeX UTF8Wrap Library.
 *
 * The MiKTeX UTF8Wrap Library is licensed under GNU General Public
 * License version 2 or any later version.
 */

/* This file was generated from miktex/utf8wrap-config.h.cmake. */

#pragma once

#include <miktex/Definitions.h>

#if !defined(BF56453E041E4B58A0EA455A65DD28B1)
#   if defined(MIKTEX_UTF8WRAP_SHARED)
#       define MIKTEXUTF8WRAPEXPORT MIKTEXDLLIMPORT
#   else
#       define MIKTEXUTF8WRAPEXPORT
#   endif
#endif

// API decoration for exported functions
#define MIKTEXUTF8WRAPCEEAPI(type) MIKTEXUTF8WRAPEXPORT type MIKTEXCEECALL

#cmakedefine HAVE_ACCESS 1
#cmakedefine HAVE_CHDIR 1
#cmakedefine HAVE_CHMOD 1
#cmakedefine HAVE_GETCWD 1
#cmakedefine HAVE_MKDIR 1
#cmakedefine HAVE_OPEN 1
#cmakedefine HAVE_POPEN 1
#cmakedefine HAVE_RENAME 1
#cmakedefine HAVE_RMDIR 1
#cmakedefine HAVE_SPAWNVP 1
#cmakedefine HAVE_STAT 1
#cmakedefine HAVE_UNLINK 1
#cmakedefine HAVE_UTIME 1

#define IS_DEFINED_PUTC 1
#define IS_DEFINED_PUTCHAR 1

#include <miktex/Definitions.h>

#if MIKTEX_UTF8_WRAP_ALL
#   if !defined(MIKTEX_UTF8_WRAP_ACCESS) && HAVE_ACCESS
#       define MIKTEX_UTF8_WRAP_ACCESS 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__ACCESS)
#       define MIKTEX_UTF8_WRAP__ACCESS 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_CHDIR) && HAVE_CHDIR
#      define MIKTEX_UTF8_WRAP_CHDIR 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__CHDIR)
#      define MIKTEX_UTF8_WRAP__CHDIR 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_CHMOD) && HAVE_CHMOD
#      define MIKTEX_UTF8_WRAP_CHMOD 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__CHMOD)
#      define MIKTEX_UTF8_WRAP__CHMOD 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_FOPEN)
#      define MIKTEX_UTF8_WRAP_FOPEN 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_FPUTC)
#      define MIKTEX_UTF8_WRAP_FPUTC 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_GETCWD) && HAVE_GETCWD
#      define MIKTEX_UTF8_WRAP_GETCWD 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__GETCWD)
#      define MIKTEX_UTF8_WRAP__GETCWD 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_GETENV)
#      define MIKTEX_UTF8_WRAP_GETENV 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_MKDIR) && HAVE_MKDIR
#      define MIKTEX_UTF8_WRAP_MKDIR 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__MKDIR)
#      define MIKTEX_UTF8_WRAP__MKDIR 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_OPEN) && HAVE_OPEN
#      define MIKTEX_UTF8_WRAP_OPEN 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__OPEN)
#      define MIKTEX_UTF8_WRAP__OPEN 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_POPEN) && HAVE_POPEN
#      define MIKTEX_UTF8_WRAP_POPEN 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__POPEN)
#      define MIKTEX_UTF8_WRAP__POPEN 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_PUTC)
#      define MIKTEX_UTF8_WRAP_PUTC 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_PUTCHAR)
#      define MIKTEX_UTF8_WRAP_PUTCHAR 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_PUTENV)
#      define MIKTEX_UTF8_WRAP_PUTENV 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_RENAME)
#      define MIKTEX_UTF8_WRAP_RENAME 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_REMOVE)
#      define MIKTEX_UTF8_WRAP_REMOVE 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_RMDIR) && HAVE_RMDIR
#      define MIKTEX_UTF8_WRAP_RMDIR 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__RMDIR)
#      define MIKTEX_UTF8_WRAP__RMDIR 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_SPAWNVP) && HAVE_SPAWNVP
#      define MIKTEX_UTF8_WRAP_SPAWNVP 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__SPAWNVP)
#      define MIKTEX_UTF8_WRAP__SPAWNVP 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_STAT) && HAVE_STAT
#      define MIKTEX_UTF8_WRAP_STAT 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__STAT64I32)
#      define MIKTEX_UTF8_WRAP__STAT64I32 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__STAT64)
#      define MIKTEX_UTF8_WRAP__STAT64 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_SYSTEM)
#      define MIKTEX_UTF8_WRAP_SYSTEM 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_UNLINK) && HAVE_UNLINK
#      define MIKTEX_UTF8_WRAP_UNLINK 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__UNLINK)
#      define MIKTEX_UTF8_WRAP__UNLINK 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP__UTIME64)
#      define MIKTEX_UTF8_WRAP__UTIME64 1
#   endif
#   if !defined(MIKTEX_UTF8_WRAP_UTIME) && HAVE_UTIME
#      define MIKTEX_UTF8_WRAP_UTIME 1
#   endif
#endif
