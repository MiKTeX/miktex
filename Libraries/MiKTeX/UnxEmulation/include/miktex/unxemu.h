/* miktex/unxemu.h: Unx emulation                       -*- C++ -*-

   Copyright (C) 2007-2016 Christian Schenk

   This file is part of the MiKTeX UNXEMU Library.

   The MiKTeX UNXEMU Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX UNXEMU Library is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX UNXEMU Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(CD70C7B43023454AA6033DFEA94551FE)
#define CD70C7B43023454AA6033DFEA94551FE

#include <miktex/unxemu-config.h>

#if defined(__cplusplus)
#  include <miktex/Util/inliners.h>
#else
#  include <miktex/Core/c/api.h>
#endif

#include <direct.h>
#include <fcntl.h>
#include <float.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <wchar.h>

// DLL import/export switch
#if !defined(D2A2BA842ACE40C6A8A17A9358F2147E)
#  define MIKTEXUNXEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported functions
#define MIKTEXUNXCEEAPI(type) MIKTEXUNXEXPORT type MIKTEXCEECALL

/* _________________________________________________________________________
 *
 * alloca.h
 *
 */

#if !HAVE_ALLOCA && !defined(alloca)
static inline void * alloca(size_t size)
{
  return _malloca(size);
}
#endif

/* _________________________________________________________________________
 *
 * dirent.h
 *
 */

MIKTEX_BEGIN_EXTERN_C_BLOCK;

struct DIR_;
struct WDIR_;

typedef struct DIR_ DIR;
typedef struct WDIR_ WDIR;

struct dirent
{
  int d_ino;
  char d_name[260];
};

struct wdirent
{
  int d_ino;
  wchar_t d_name[260];
};

MIKTEXUNXCEEAPI(int) closedir(DIR * pDir);

MIKTEXUNXCEEAPI(DIR *) opendir(const char * lpszPath);

MIKTEXUNXCEEAPI(struct dirent *) readdir(DIR * pDir);

MIKTEXUNXCEEAPI(void) rewinddir(DIR * pDir);

MIKTEXUNXCEEAPI(int) wclosedir(WDIR * pDir);

MIKTEXUNXCEEAPI(WDIR *) wopendir(const wchar_t * lpszPath);

MIKTEXUNXCEEAPI(struct wdirent *) wreaddir(WDIR * pDir);

MIKTEXUNXCEEAPI(void) wrewinddir(WDIR * pDir);

MIKTEX_END_EXTERN_C_BLOCK;

/* _________________________________________________________________________
 *
 * math.h
 *
 */

#if !HAVE_FINITE && !defined(finite)
#undef HAVE_FINITE
#define HAVE_FINITE 1
static inline int finite(double x)
{
  return _finite(x);
}
#endif

/* _________________________________________________________________________
 *
 * stdio.h
 *
 */

#if !HAVE_PCLOSE && !defined(pclose)
#undef HAVE_PCLOSE
#define HAVE_PCLOSE 1
static inline int pclose(FILE * stream)
{
  return _pclose(stream);
}
#endif

#if !HAVE_POPEN && !defined(popen)
#undef HAVE_POPEN
#define HAVE_POPEN 1
static inline FILE * popen(const char * command, const char * mode)
{
  return _popen(command, mode);
}
#endif

/* _________________________________________________________________________
 *
 * strings.h
 *
 */

MIKTEX_EXTERN_C MIKTEXUNXCEEAPI(int) miktex_strncasecmp(const char * lpsz1, const char * lpsz2, size_t n);

#if !HAVE_BZERO && !defined(bzero)
#undef HAVE_BZERO
#define HAVE_BZERO 1
static inline void bzero(void * s, size_t n)
{
  memset(s, 0, n);
}
#endif

#if !HAVE_STRCASECMP && !defined(strcasecmp)
#undef HAVE_STRCASECMP
#define HAVE_STRCASECMP 1
static inline int strcasecmp(const char * s1, const char * s2)
{
#if defined(__cplusplus)
  return MiKTeX::Util::StringCompare(s1, s2, true);
#else
  return _strcmpi(s1, s2);
#endif
}
#endif

#if !HAVE_STRNCASECMP && !defined(strncasecmp)
#undef HAVE_STRNCASECMP
#define HAVE_STRNCASECMP 1
static inline int strncasecmp(const char * s1, const char * s2, size_t n)
{
#if defined(__cplusplus)
  return MiKTeX::Util::StringCompare(s1, s2, n, true);
#else
  return miktex_strncasecmp(s1, s2, n);
#endif
}
#endif

/* _________________________________________________________________________
 *
 * sys/stat.h
 *
 */

#define S_ISDIR(m) (((m) & _S_IFDIR) != 0)
#define S_ISREG(m) (((m) & _S_IFREG) != 0)

#if !HAVE_OPEN && !defined(open)
#undef HAVE_OPEN
#define HAVE_OPEN 1
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
  return _open(path, oflag, pmode);
}
#endif

#if !HAVE_STAT && !defined(stat)
#undef HAVE_STAT
#define HAVE_STAT 1
static inline int stat(const char * path, struct _stat * buf)
{
  return _stat(path, buf);
}
#endif

/* _________________________________________________________________________
 *
 * sys/time.h
 *
 */

MIKTEX_EXTERN_C MIKTEXUNXCEEAPI(int) miktex_gettimeofday(struct timeval * ptv, void * pNull);

#if !HAVE_GETTIMEOFDAY && !defined(gettimeofday)
static inline int gettimeofday(struct timeval * tp, void * tzp)
{
  return miktex_gettimeofday(tp, tzp);
}
#endif

/* _________________________________________________________________________
 *
 * sys/types.h
 *
 */

#if !HAVE_CHMOD && !defined(chmod)
static inline int chmod(const char * path, int mode)
{
  return _chmod(path, mode);
}
#endif

#if !HAVE_MKDIR && !defined(mkdir)
static inline int mkdir(const char * path, int mode)
{
  return _mkdir(path);
}
#endif

/* _________________________________________________________________________
 *
 * unistd.h
 *
 */

#define F_OK 0
#define W_OK 2
#define R_OK 4
#define X_OK W_OK

#if !HAVE_ACCESS && !defined(access)
static inline int access(const char * path, int amode)
{
  return _access(path, amode);
}
#endif

#if !HAVE_CHDIR && !defined(chdir)
static inline int chdir(const char * path)
{
  return _chdir(path);
}
#endif

#if !HAVE_GETCWD && !defined(getcwd)
static inline char * getcwd(char * buf, size_t size)
{
  return _getcwd(buf, size);
}
#endif

#if !HAVE_GETPID && !defined(getpid)
static inline int getpid()
{
  return _getpid();
}
#endif

#if !HAVE_RMDIR && !defined(rmdir)
static inline int rmdir(const char * path)
{
  return _rmdir(path);
}
#endif

#if !HAVE_UNLINK && !defined(unlink)
static inline int unlink(const char * path)
{
  return _unlink(path);
}
#endif

/* _________________________________________________________________________
 *
 * utime.h
 *
 */

#if !HAVE_UTIME && !defined(utime)
static inline int utime(const char * path, const struct _utimbuf * times)
{
  struct _utimbuf copyTimes = *times;
  return _utime(path, &copyTimes);
}
#endif

#endif
