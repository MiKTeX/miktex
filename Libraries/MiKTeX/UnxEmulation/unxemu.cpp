/**
 * @file unxemu.cpp
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

#include "internal.h"

#include <errno.h>

#include <WinSock2.h>

#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Session>
#include <miktex/Util/StringUtil>
#include <miktex/Util/inliners.h>

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

struct DIR_
{
    unique_ptr<DirectoryLister> directoryLister;
    struct dirent direntry;
    PathName path;
    DIR_(const char* path) :
        path(path),
        directoryLister(DirectoryLister::Open(PathName(path), nullptr, static_cast<int>(DirectoryLister::Options::IncludeDotAndDotDot)))
    {
    }
};

struct WDIR_
{
    unique_ptr<DirectoryLister> directoryLister;
    struct wdirent direntry;
    PathName path;
    WDIR_(const wchar_t* path) :
        path(path),
        directoryLister(DirectoryLister::Open(PathName(path), nullptr, static_cast<int>(DirectoryLister::Options::IncludeDotAndDotDot)))
    {
    }
};

MIKTEXUNXCEEAPI(int) closedir(DIR* dir)
{
    C_FUNC_BEGIN();
    delete dir;
    return 0;
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(int) wclosedir(WDIR* dir)
{
    C_FUNC_BEGIN();
    delete dir;
    return 0;
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(DIR*) opendir(const char* path)
{
    C_FUNC_BEGIN();
    if (!Directory::Exists(PathName(path)))
    {
        errno = ENOENT;
        return nullptr;
    }
    return new DIR_(path);
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(WDIR*) wopendir(const wchar_t* path)
{
    C_FUNC_BEGIN();
    if (!Directory::Exists(PathName(path)))
    {
        errno = ENOENT;
        return nullptr;
    }
    return new WDIR_(path);
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(struct dirent*) readdir(DIR* dir)
{
    C_FUNC_BEGIN();
    DirectoryEntry directoryEntry;
    if (!dir->directoryLister->GetNext(directoryEntry))
    {
        return nullptr;
    }
    StringUtil::CopyCeeString(dir->direntry.d_name, sizeof(dir->direntry.d_name) / sizeof(dir->direntry.d_name[0]), directoryEntry.name.c_str());
    return &dir->direntry;
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(void) seekdir(DIR* dir, long int loc)
{
    // not yet implemented
    MIKTEX_UNEXPECTED();
}

MIKTEXUNXCEEAPI(long int) telldir(DIR* dir)
{
    // not yet implemented
    MIKTEX_UNEXPECTED();
}

MIKTEXUNXCEEAPI(struct wdirent*) wreaddir(WDIR* dir)
{
    C_FUNC_BEGIN();
    DirectoryEntry directoryEntry;
    if (!dir->directoryLister->GetNext(directoryEntry))
    {
        return nullptr;
    }
    StringUtil::CopyCeeString(dir->direntry.d_name, sizeof(dir->direntry.d_name) / sizeof(dir->direntry.d_name[0]), directoryEntry.wname.c_str());
    return &dir->direntry;
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(void) rewinddir(DIR* dir)
{
    C_FUNC_BEGIN();
    dir->directoryLister->Close();
    dir->directoryLister = DirectoryLister::Open(dir->path);
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(void) wrewinddir(WDIR* dir)
{
    C_FUNC_BEGIN();
    dir->directoryLister->Close();
    dir->directoryLister = DirectoryLister::Open(dir->path);
    C_FUNC_END();
}

MIKTEXUNXCEEAPI(int) miktex_strncasecmp(const char* lpsz1, const char* lpsz2, size_t n)
{
    return MiKTeX::Util::CeeStringCompare(lpsz1, lpsz2, n, true);
}

MIKTEXUNXCEEAPI(int) miktex_gettimeofday(struct timeval* ptv, void* null)
{
    MIKTEX_ASSERT(null == nullptr);
    MIKTEX_ASSERT(ptv != nullptr);
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = systemTime.wYear;
    tm.tm_mon = systemTime.wMonth - 1;
    tm.tm_mday = systemTime.wDay;
    tm.tm_hour = systemTime.wHour;
    tm.tm_min = systemTime.wMinute;
    tm.tm_sec = systemTime.wSecond;
    ptv->tv_sec = static_cast<long>(mktime(&tm));
    ptv->tv_usec = systemTime.wMilliseconds;
    return 0;
}

// derived from glibc 2.3.6 libc/sysdeps/posix/tempname.c
// Copyright (C) 1991-1999, 2000, 2001 Free Software Foundation, Inc.
MIKTEXUNXCEEAPI(int) miktex_mkstemp(char* tmpl)
{
    size_t len = strlen(tmpl);
    if (len < 6 || strcmp(&tmpl[len - 6], "XXXXXX") != 0)
    {
        // TODO
        return -1;
    }
    char* const XXXXXX = &tmpl[len - 6];
    static const char letters[] = "ABCDEFGHIJKLMnopqrstuvwxyz0123456789";
    const int lettercount = sizeof(letters) - 1;
    uint64_t value = time(nullptr) ^ GetCurrentProcessId();
    for (int rounds = 1000; rounds > 0; rounds--, value += 7777)
    {
        uint64_t v = value;
        for (char* x = XXXXXX; *x != 0; ++x, v /= lettercount)
        {
            *x = letters[v % lettercount];
        }
        int fd = _open(tmpl, _O_CREAT | _O_EXCL | _O_RDWR, _S_IREAD | _S_IWRITE);
        if (fd >= 0)
        {
            return fd;
        }
        else if (fd != EEXIST)
        {
            // TODO
            return -1;
        }
    }
    // TODO
    return -1;
}
