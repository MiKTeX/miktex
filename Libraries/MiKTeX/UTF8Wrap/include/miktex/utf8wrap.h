/* miktex/utf8wrap.h: Unx emulation                     -*- C++ -*-

   Copyright (C) 2011-2016 Christian Schenk

   This file is part of the MiKTeX UTF8Wrap Library.

   The MiKTeX UTF8Wrap Library is free software; you can redistribute
   it and/or modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX UTF8Wrap Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UTF8Wrap Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(B2E524AD08FE4528BE66DC6ED9CF716C)
#define B2E524AD08FE4528BE66DC6ED9CF716C

#include <miktex/utf8wrap-config.h>

#if MIKTEX_UTF8_WRAP_ACCESS
#  define access access_hidden
#endif
#if MIKTEX_UTF8_WRAP__ACCESS
#  define _access _access_hidden
#endif
#if MIKTEX_UTF8_WRAP_CHDIR
#  define chdir chdir_hidden
#endif
#if MIKTEX_UTF8_WRAP__CHDIR
#  define _chdir _chdir_hidden
#endif
#if MIKTEX_UTF8_WRAP_CHMOD
#  define chmod chmod_hidden
#endif
#if MIKTEX_UTF8_WRAP__CHMOD
#  define _chmod _chmod_hidden
#endif
#if MIKTEX_UTF8_WRAP_FOPEN
#  define fopen fopen_hidden
#endif
#if MIKTEX_UTF8_WRAP_FPUTC
#  define fputc fputc_hidden
#endif
#if MIKTEX_UTF8_WRAP_GETCWD
#  define getcwd getcwd_hidden
#endif
#if MIKTEX_UTF8_WRAP__GETCWD
#  define _getcwd _getcwd_hidden
#endif
#if MIKTEX_UTF8_WRAP_GETENV
#  define getenv getenv_hidden
#endif
#if MIKTEX_UTF8_WRAP_MKDIR
#  define mkdir mkdir_hidden
#endif
#if MIKTEX_UTF8_WRAP__MKDIR
#  define _mkdir _mkdir_hidden
#endif
#if MIKTEX_UTF8_WRAP_OPEN
#  define open open_hidden
#endif
#if MIKTEX_UTF8_WRAP__OPEN
#  define _open _open_hidden
#endif
#if MIKTEX_UTF8_WRAP_POPEN
#  define popen popen_hidden
#endif
#if MIKTEX_UTF8_WRAP__POPEN
#  define _popen _popen_hidden
#endif
#if MIKTEX_UTF8_WRAP_PUTC && !IS_DEFINED_PUTC
#  define putc putc_hidden
#endif
#if MIKTEX_UTF8_WRAP_PUTCHAR && !IS_DEFINED_PUTCHAR
#  define putchar putchar_hidden
#endif
#if MIKTEX_UTF8_WRAP_RENAME
#  define rename rename_hidden
#endif
#if MIKTEX_UTF8_WRAP_REMOVE
#  define remove remove_hidden
#endif
#if MIKTEX_UTF8_WRAP_RMDIR
#  define rmdir rmdir_hidden
#endif
#if MIKTEX_UTF8_WRAP__RMDIR
#  define _rmdir _rmdir_hidden
#endif
#if MIKTEX_UTF8_WRAP_SPAWNVP
#  define spawnvp spawnvp_hidden
#endif
#if MIKTEX_UTF8_WRAP__SPAWNVP
#  define _spawnvp _spawnvp_hidden
#endif
#if MIKTEX_UTF8_WRAP_STAT
#  define stat(f, s) stat_hidden(f, s)
#endif
#if MIKTEX_UTF8_WRAP__STAT64I32
#  define _stat64i32 _stat64i32_hidden
#endif
#if MIKTEX_UTF8_WRAP_SYSTEM
#  define system system_hidden
#endif
#if MIKTEX_UTF8_WRAP_UNLINK
#  define unlink unlink_hidden
#endif
#if MIKTEX_UTF8_WRAP__UNLINK
#  define _unlink _unlink_hidden
#endif
#if MIKTEX_UTF8_WRAP__UTIME64
#  define _utime64 _utime64_hidden
#endif
#if MIKTEX_UTF8_WRAP_UTIME
#  define utime utime_hidden
#endif

#if defined(_INC_STDIO) && MIKTEX_UTF8_WRAP_STDIO
#error stdio.h must not be included before miktex/utf8wrap.h
#endif

#if defined(_stat) && MIKTEX_UTF8_WRAP_STAT
#error sys/stat.h must not be included before miktex/utf8wrap.h
#endif

#include <stdio.h>
#include <stdlib.h>

#if defined(__cplusplus)
#include <cstdio>
#include <cstdlib>
#endif

#if defined(__cplusplus)
#include <cstdarg>
#else
#include <stdarg.h>
#endif

#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include <sys/utime.h>

MIKTEX_BEGIN_EXTERN_C_BLOCK;

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__access(const char * path, int mode);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__chdir(const char * path);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__chmod(const char * path, int mode);

MIKTEXUTF8WRAPCEEAPI(FILE *) miktex_utf8_fopen(const char * path, const char * mode);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_fputc(int ch, FILE * file);

MIKTEXUTF8WRAPCEEAPI(char *) miktex_utf8__getcwd(char * path, size_t maxSize);

MIKTEXUTF8WRAPCEEAPI(char *) miktex_utf8_getenv(const char * name);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__mkdir(const char * path);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__open(const char * path, int flags, ...);

MIKTEXUTF8WRAPCEEAPI(FILE *) miktex_utf8__popen(const char * command, const char * mode);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_putc(int ch, FILE * file);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_putchar(int ch);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_rename(const char * oldPathName, const char * newPathName);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_remove(const char * path);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__rmdir(const char * path);

MIKTEXUTF8WRAPCEEAPI(intptr_t) miktex_utf8__spawnvp(int mode, const char * path, const char * const * argv);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__stat64i32(const char * path, struct _stat64i32 * pStat);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8_system(const char * command);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__unlink(const char * path);

MIKTEXUTF8WRAPCEEAPI(int) miktex_utf8__utime64(const char * path, struct __utimbuf64 * pTime);

MIKTEX_END_EXTERN_C_BLOCK;

#if MIKTEX_UTF8_WRAP_ACCESS
#undef access
static inline int access(const char * path, int mode)
{
  return miktex_utf8__access(path, mode);
}
#endif

#if MIKTEX_UTF8_WRAP__ACCESS
#undef _access
static inline int _access(const char * path, int mode)
{
  return miktex_utf8__access(path, mode);
}
#endif

#if MIKTEX_UTF8_WRAP_CHDIR
#undef chdir
static inline int chdir(const char * path)
{
  return miktex_utf8__chdir(path);
}
#endif

#if MIKTEX_UTF8_WRAP__CHDIR
#undef _chdir
static inline int _chdir(const char * path)
{
  return miktex_utf8__chdir(path);
}
#endif

#if MIKTEX_UTF8_WRAP_CHMOD
#undef chmod
static inline int chmod(const char * path, int mode)
{
  return miktex_utf8__chmod(path, mode);
}
#endif

#if MIKTEX_UTF8_WRAP__CHMOD
#undef _chmod
static inline int _chmod(const char * path, int mode)
{
  return miktex_utf8__chmod(path, mode);
}
#endif

#if MIKTEX_UTF8_WRAP_FOPEN
#undef fopen
static inline FILE * fopen(const char * path, const char * mode)
{
  return miktex_utf8_fopen(path, mode);
}
#endif

#if MIKTEX_UTF8_WRAP_FPUTC
#undef fputc
static inline int fputc(int ch, FILE * file)
{
  return miktex_utf8_fputc(ch, file);
}
#endif

#if MIKTEX_UTF8_WRAP_GETCWD
#undef getcwd
static inline char * getcwd(char * path, int maxSize)
{
  return miktex_utf8__getcwd(path, maxSize);
}
#endif

#if MIKTEX_UTF8_WRAP__GETCWD
#undef _getcwd
static inline char * _getcwd(char * path, int maxSize)
{
  return miktex_utf8__getcwd(path, maxSize);
}
#endif

#if MIKTEX_UTF8_WRAP_GETENV
#undef getenv
static inline char * getenv(const char * name)
{
  return miktex_utf8_getenv(name);
}
#endif

#if MIKTEX_UTF8_WRAP_MKDIR
#undef mkdir
static inline int mkdir(const char * path)
{
  return miktex_utf8__mkdir(path);
}
#endif

#if MIKTEX_UTF8_WRAP__MKDIR
#undef _mkdir
static inline int _mkdir(const char * path)
{
  return miktex_utf8__mkdir(path);
}
#endif

#if MIKTEX_UTF8_WRAP_OPEN
#undef open
static inline int open(const char * path, int oflag, ...)
{
  int pmode = 0;
  if ((oflag & _O_CREAT) != 0)
  {
    va_list ap;
    va_start(ap, oflag);
    pmode = va_arg(ap, int);
    va_end(ap);
  }
  return miktex_utf8__open(path, oflag, pmode);
}
#endif

#if MIKTEX_UTF8_WRAP__OPEN
#undef _open
static inline int _open(const char * path, int oflag, ...)
{
  int pmode = 0;
  if ((oflag & _O_CREAT) != 0)
  {
    va_list ap;
    va_start(ap, oflag);
    pmode = va_arg(ap, int);
    va_end(ap);
  }
  return miktex_utf8__open(path, oflag, pmode);
}
#endif

#if MIKTEX_UTF8_WRAP_POPEN
#undef popen
static inline FILE * popen(const char * command, const char * mode)
{
  return miktex_utf8__popen(command, mode);
}
#endif

#if MIKTEX_UTF8_WRAP__POPEN
#undef _popen
static inline FILE * _popen(const char * command, const char * mode)
{
  return miktex_utf8__popen(command, mode);
}
#endif

#if MIKTEX_UTF8_WRAP_PUTC
#undef putc
static inline int putc(int ch, FILE * file)
{
  return miktex_utf8_putc(ch, file);
}
#endif

#if MIKTEX_UTF8_WRAP_PUTCHAR
#undef putchar
static inline int putchar(int ch)
{
  return miktex_utf8_putchar(ch);
}
#endif

#if MIKTEX_UTF8_WRAP_RENAME
#undef rename
static inline int rename(const char * oldPathName, const char * newPathName)
{
  return miktex_utf8_rename(oldPathName, newPathName);
}
#endif

#if MIKTEX_UTF8_WRAP_REMOVE
#undef remove
static inline int remove(const char * path)
{
  return miktex_utf8_remove(path);
}
#endif

#if MIKTEX_UTF8_WRAP_RMDIR
#undef rmdir
static inline int rmdir(const char * path)
{
  return miktex_utf8__rmdir(path);
}
#endif

#if MIKTEX_UTF8_WRAP__RMDIR
#undef _rmdir
static inline int _rmdir(const char * path)
{
  return miktex_utf8__rmdir(path);
}
#endif

#if MIKTEX_UTF8_WRAP_SPAWNVP
#undef spawnvp
static inline intptr_t spawnvp(int mode, const char * path, const char * const * argv)
{
  return miktex_utf8__spawnvp(mode, path, argv);
}
#endif

#if MIKTEX_UTF8_WRAP__SPAWNVP
#undef _spawnvp
static inline intptr_t _spawnvp(int mode, const char * path, const char * const * argv)
{
  return miktex_utf8__spawnvp(mode, path, argv);
}
#endif

#if MIKTEX_UTF8_WRAP__STAT64i32
#undef _stat64i32
static inline int _stat64i32(const char * path, struct _stat64i32 * pStat)
{
  return miktex_utf8__stat64i32(path, pStat);
}
#endif

#if MIKTEX_UTF8_WRAP_STAT
#undef stat
static inline int stat(const char * path, struct stat * pStat)
{
  return miktex_utf8__stat64i32(path, (struct _stat64i32*)pStat);
}
#endif

#if MIKTEX_UTF8_WRAP_SYSTEM
#undef system
static inline int system(const char * command)
{
  return miktex_utf8_system(command);
}
#endif

#if MIKTEX_UTF8_WRAP_UNLINK
#undef unlink
static inline int unlink(const char * path)
{
  return miktex_utf8__unlink(path);
}
#endif

#if MIKTEX_UTF8_WRAP__UNLINK
#undef _unlink
static inline int _unlink(const char * path)
{
  return miktex_utf8__unlink(path);
}
#endif

#if MIKTEX_UTF8_WRAP_UTIME
#undef utime
static inline int utime(const char * path, struct utimbuf * pTime)
{
  return miktex_utf8__utime64(path, (struct __utimbuf64*)pTime);
}
#endif

#if MIKTEX_UTF8_WRAP__UTIME64
#undef _utime64
static inline int _utime64(const char * path, struct __utimbuf64 * pTime)
{
  return miktex_utf8__utime64(path, pTime);
}
#endif

#endif
