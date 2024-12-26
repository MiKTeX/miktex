/**
 * @file miktex/unxemu.h
 * @defgroup Unx emulation
 * @author Christian Schenk
 * @brief Utilities for emulating Unx functions
 *
 * @copyright Copyright © 2007-2024 Christian Schenk
 *
 * This file is part of the MiKTeX UNXEMU Library.
 *
 * MiKTeX UNXEMU Library is licensed under GNU General Public License version 2
 * or any later version.
 */

#pragma once

#include <miktex/unxemu-config.h>

#if defined(__cplusplus)
#include <miktex/Util/inliners.h>
#else
#include <miktex/Core/c/api.h>
#endif

#include <direct.h>
#include <fcntl.h>
#include <float.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <wchar.h>

#if defined(__cplusplus)
#include <chrono>
#include <thread>
#endif

// DLL import/export switch
#if !defined(D2A2BA842ACE40C6A8A17A9358F2147E)
#define MIKTEXUNXEXPORT MIKTEXDLLIMPORT
#endif

// API decoration for exported functions
#define MIKTEXUNXCEEAPI(type) MIKTEXUNXEXPORT type MIKTEXCEECALL

/* alloca.h */

#if !HAVE_ALLOCA && !defined(alloca)
static inline void* alloca(size_t size)
{
    return _malloca(size);
}
#endif

/* dirent.h */

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

MIKTEXUNXCEEAPI(int) closedir(DIR* pDir);
MIKTEXUNXCEEAPI(DIR*) opendir(const char* path);
MIKTEXUNXCEEAPI(struct dirent*) readdir(DIR* dir);
MIKTEXUNXCEEAPI(void) rewinddir(DIR* dir);
MIKTEXUNXCEEAPI(void) seekdir(DIR* dir, long int loc);
MIKTEXUNXCEEAPI(long int) telldir(DIR* dir);
MIKTEXUNXCEEAPI(int) wclosedir(WDIR* dir);
MIKTEXUNXCEEAPI(WDIR*) wopendir(const wchar_t* path);
MIKTEXUNXCEEAPI(struct wdirent*) wreaddir(WDIR* dir);
MIKTEXUNXCEEAPI(void) wrewinddir(WDIR* dir);

MIKTEX_END_EXTERN_C_BLOCK;

/* math.h */

#if !HAVE_FINITE && !defined(finite)
#undef HAVE_FINITE
#define HAVE_FINITE 1
static inline int finite(double x)
{
    return _finite(x);
}
#endif

/* stdlib.h */

MIKTEX_EXTERN_C MIKTEXUNXCEEAPI(int) miktex_mkstemp(char* tmpl);

#if !HAVE_MKSTEMP && !defined(mkstemp)
#undef HAVE_MKSTEMP
#define HAVE_MKSTEMP 1
static inline int mkstemp(char* tmpl)
{
    return miktex_mkstemp(tmpl);
}
#endif

/* strings.h */

MIKTEX_EXTERN_C MIKTEXUNXCEEAPI(int) miktex_strncasecmp(const char* lpsz1, const char* lpsz2, size_t n);

#if !HAVE_BZERO && !defined(bzero)
#undef HAVE_BZERO
#define HAVE_BZERO 1
static inline void bzero(void* s, size_t n)
{
    memset(s, 0, n);
}
#endif

#if !HAVE_STRCASECMP && !defined(strcasecmp)
#undef HAVE_STRCASECMP
#define HAVE_STRCASECMP 1
static inline int strcasecmp(const char* s1, const char* s2)
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
static inline int strncasecmp(const char* s1, const char* s2, size_t n)
{
#if defined(__cplusplus)
    return MiKTeX::Util::CeeStringCompare(s1, s2, n, true);
#else
    return miktex_strncasecmp(s1, s2, n);
#endif
}
#endif

#if !defined(HAVE_INDEX) && !defined(index)
#undef HAVE_INDEX
#define HAVE_INDEX 1
static inline const char* index(const char* s, int c)
{
    return strchr(s, c);
}
#endif

#if !defined(HAVE_RINDEX) && !defined(rindex)
#undef HAVE_RINDEX
#define HAVE_RINDEX 1
static inline const char* rindex(const char* s, int c)
{
    return strrchr(s, c);
}
#endif

/* sys/stat.h */

#define S_ISDIR(m) (((m) & _S_IFDIR) != 0)
#define S_ISREG(m) (((m) & _S_IFREG) != 0)

#define S_IWGRP 0
#define S_IWOTH 0

#if !HAVE_OPEN && !defined(open)
#undef HAVE_OPEN
#define HAVE_OPEN 1
static inline int open(const char* path, int oflag, ...)
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
static inline int stat(const char* path, struct _stat* buf)
{
    return _stat(path, buf);
}
#endif

/* sys/time.h */

MIKTEX_EXTERN_C MIKTEXUNXCEEAPI(int) miktex_gettimeofday(struct timeval* ptv, void* null);

#if !HAVE_GETTIMEOFDAY && !defined(gettimeofday)
static inline int gettimeofday(struct timeval* tp, void* tzp)
{
    return miktex_gettimeofday(tp, tzp);
}
#endif

/* sys/types.h */

typedef intptr_t ssize_t;

typedef short uid_t;

#if !HAVE_CHMOD && !defined(chmod)
static inline int chmod(const char* path, int mode)
{
    return _chmod(path, mode);
}
#endif

#if !HAVE_MKDIR && !defined(mkdir)
static inline int mkdir(const char* path, int mode)
{
    return _mkdir(path);
}
#endif

/* unistd.h */

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define F_OK 0
#define W_OK 2
#define R_OK 4
#define X_OK W_OK

#if !HAVE_ACCESS && !defined(access)
static inline int access(const char* path, int amode)
{
    return _access(path, amode);
}
#endif

#if !HAVE_CHDIR && !defined(chdir)
static inline int chdir(const char* path)
{
    return _chdir(path);
}
#endif

#if !HAVE_GETCWD && !defined(getcwd)
static inline char* getcwd(char* buf, size_t size)
{
    return _getcwd(buf, size);
}
#endif

#if !HAVE_GETEUID && !defined(geteuid)
static inline uid_t geteuid()
{
    return 0;
}
#endif

#if !HAVE_GETPID && !defined(getpid)
static inline int getpid()
{
    return _getpid();
}
#endif

#if !HAVE_GETUID && !defined(getuid)
static inline uid_t getuid()
{
    return 0;
}
#endif

#if !HAVE_RMDIR && !defined(rmdir)
static inline int rmdir(const char* path)
{
    return _rmdir(path);
}
#endif

#if !HAVE_UNLINK && !defined(unlink)
static inline int unlink(const char* path)
{
    return _unlink(path);
}
#endif

#if !HAVE_USLEEP && !defined(usleep) && defined(__cplusplus)
static inline int usleep(unsigned long useconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(useconds));
    return 0;
}
#endif

/* utime.h */

#if !HAVE_UTIME && !defined(utime)
static inline int utime(const char* path, const struct _utimbuf* times)
{
    struct _utimbuf copyTimes = *times;
    return _utime(path, &copyTimes);
}
#endif
