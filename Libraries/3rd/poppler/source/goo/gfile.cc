//========================================================================
//
// gfile.cc
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2008 Adam Batkin <adam@batkin.net>
// Copyright (C) 2008, 2010, 2012, 2013 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009, 2012, 2014, 2017, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2013, 2018 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2013, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Peter Breitenlohner <peb@mppmu.mpg.de>
// Copyright (C) 2013, 2017 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2017 Christoph Cullmann <cullmann@kde.org>
// Copyright (C) 2018 Mojca Miklavec <mojca@macports.org>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#if defined(MIKTEX)
#  if defined(MIKTEX_WINDOWS)
#    define MIKTEX_UTF8_WRAP_ALL 1
#    include <miktex/utf8wrap.h>
#    include <miktex/unxemu.h>
#  endif
#  include <miktex/Core/Directory>
#  include <miktex/Util/PathName>
#endif

#ifndef _WIN32
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#    include <climits>
#    include <cstring>
#    include <pwd.h>
#endif // _WIN32
#include <cstdio>
#include <limits>
#include "GooString.h"
#include "gfile.h"
#include "gdir.h"

// Some systems don't define this, so just make it something reasonably
// large.
#ifndef PATH_MAX
#    define PATH_MAX 1024
#endif

#ifndef _WIN32

using namespace std::string_literals;

namespace {

template<typename...>
struct void_type
{
    using type = void;
};

template<typename... Args>
using void_t = typename void_type<Args...>::type;

template<typename Stat, typename = void_t<>>
struct StatMtim
{
    static const struct timespec &value(const Stat &stbuf) { return stbuf.st_mtim; }
};

// Mac OS X uses a different field name than POSIX and this detects it.
template<typename Stat>
struct StatMtim<Stat, void_t<decltype(Stat::st_mtimespec)>>
{
    static const struct timespec &value(const Stat &stbuf) { return stbuf.st_mtimespec; }
};

inline const struct timespec &mtim(const struct stat &stbuf)
{
    return StatMtim<struct stat>::value(stbuf);
}

}

#endif

//------------------------------------------------------------------------

GooString *appendToPath(GooString *path, const char *fileName)
{
#ifdef _WIN32
    //---------- Win32 ----------
    GooString *tmp;
    char buf[256];
    char *fp;

    tmp = new GooString(path);
    tmp->append('/');
    tmp->append(fileName);
#if defined(MIKTEX_WINDOWS)
    MiKTeX::Util::PathName absPath(tmp->c_str());
    absPath.MakeFullyQualified();
    delete tmp;
    path->clear();
    path->append(absPath.GetData());
#else
    GetFullPathNameA(tmp->c_str(), sizeof(buf), buf, &fp);
    delete tmp;
    path->clear();
    path->append(buf);
#endif
    return path;

#else
    //---------- Unix ----------
    int i;

    // appending "." does nothing
    if (!strcmp(fileName, "."))
        return path;

    // appending ".." goes up one directory
    if (!strcmp(fileName, "..")) {
        for (i = path->getLength() - 2; i >= 0; --i) {
            if (path->getChar(i) == '/')
                break;
        }
        if (i <= 0) {
            if (path->getChar(0) == '/') {
                path->del(1, path->getLength() - 1);
            } else {
                path->clear();
                path->append("..");
            }
        } else {
            path->del(i, path->getLength() - i);
        }
        return path;
    }

    // otherwise, append "/" and new path component
    if (path->getLength() > 0 && path->getChar(path->getLength() - 1) != '/')
        path->append('/');
    path->append(fileName);
    return path;
#endif
}

static bool makeFileDescriptorCloexec(int fd)
{
#ifdef FD_CLOEXEC
    int flags = fcntl(fd, F_GETFD);
    if (flags >= 0 && !(flags & FD_CLOEXEC))
        flags = fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

    return flags >= 0;
#else
    return true;
#endif
}

#ifndef _WIN32

int openFileDescriptor(const char *path, int flags)
{
#    ifdef O_CLOEXEC
    return open(path, flags | O_CLOEXEC);
#    else
    int fd = open(path, flags);
    if (fd == -1)
        return fd;

    if (!makeFileDescriptorCloexec(fd)) {
        close(fd);
        return -1;
    }

    return fd;
#    endif
}

#endif

FILE *openFile(const char *path, const char *mode)
{
#ifdef _WIN32
#if defined(MIKTEX)
  return fopen(path, mode);
#else
    OSVERSIONINFO version;
    wchar_t wPath[_MAX_PATH + 1];
    char nPath[_MAX_PATH + 1];
    wchar_t wMode[8];
    const char *p;
    size_t i;

    // NB: _wfopen is only available in NT
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx(&version);
    if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        for (p = path, i = 0; *p && i < _MAX_PATH; ++i) {
            if ((p[0] & 0xe0) == 0xc0 && p[1] && (p[1] & 0xc0) == 0x80) {
                wPath[i] = (wchar_t)(((p[0] & 0x1f) << 6) | (p[1] & 0x3f));
                p += 2;
            } else if ((p[0] & 0xf0) == 0xe0 && p[1] && (p[1] & 0xc0) == 0x80 && p[2] && (p[2] & 0xc0) == 0x80) {
                wPath[i] = (wchar_t)(((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f));
                p += 3;
            } else {
                wPath[i] = (wchar_t)(p[0] & 0xff);
                p += 1;
            }
        }
        wPath[i] = (wchar_t)0;
        for (i = 0; (i < sizeof(mode) - 1) && mode[i]; ++i) {
            wMode[i] = (wchar_t)(mode[i] & 0xff);
        }
        wMode[i] = (wchar_t)0;
        return _wfopen(wPath, wMode);
    } else {
        for (p = path, i = 0; *p && i < _MAX_PATH; ++i) {
            if ((p[0] & 0xe0) == 0xc0 && p[1] && (p[1] & 0xc0) == 0x80) {
                nPath[i] = (char)(((p[0] & 0x1f) << 6) | (p[1] & 0x3f));
                p += 2;
            } else if ((p[0] & 0xf0) == 0xe0 && p[1] && (p[1] & 0xc0) == 0x80 && p[2] && (p[2] & 0xc0) == 0x80) {
                nPath[i] = (char)(((p[1] & 0x3f) << 6) | (p[2] & 0x3f));
                p += 3;
            } else {
                nPath[i] = p[0];
                p += 1;
            }
        }
        nPath[i] = '\0';
        return fopen(nPath, mode);
    }
#endif
#else
    // First try to atomically open the file with CLOEXEC
    const std::string modeStr = mode + "e"s;
    FILE *file = fopen(path, modeStr.c_str());
    if (file != nullptr)
        return file;

    // Fall back to the provided mode and apply CLOEXEC afterwards
    file = fopen(path, mode);
    if (file == nullptr)
        return nullptr;

    if (!makeFileDescriptorCloexec(fileno(file))) {
        fclose(file);
        return nullptr;
    }

    return file;
#endif
}

char *getLine(char *buf, int size, FILE *f)
{
    int c, i;

    i = 0;
    while (i < size - 1) {
        if ((c = fgetc(f)) == EOF) {
            break;
        }
        buf[i++] = (char)c;
        if (c == '\x0a') {
            break;
        }
        if (c == '\x0d') {
            c = fgetc(f);
            if (c == '\x0a' && i < size - 1) {
                buf[i++] = (char)c;
            } else if (c != EOF) {
                ungetc(c, f);
            }
            break;
        }
    }
    buf[i] = '\0';
    if (i == 0) {
        return nullptr;
    }
    return buf;
}

int Gfseek(FILE *f, Goffset offset, int whence)
{
#if defined(HAVE_FSEEKO)
    return fseeko(f, offset, whence);
#elif defined(HAVE_FSEEK64)
    return fseek64(f, offset, whence);
#elif defined(__MINGW32__)
    return fseeko64(f, offset, whence);
#elif defined(_WIN32)
    return _fseeki64(f, offset, whence);
#else
    return fseek(f, offset, whence);
#endif
}

Goffset Gftell(FILE *f)
{
#if defined(HAVE_FSEEKO)
    return ftello(f);
#elif defined(HAVE_FSEEK64)
    return ftell64(f);
#elif defined(__MINGW32__)
    return ftello64(f);
#elif defined(_WIN32)
    return _ftelli64(f);
#else
    return ftell(f);
#endif
}

Goffset GoffsetMax()
{
#if defined(HAVE_FSEEKO)
    return (std::numeric_limits<off_t>::max)();
#elif defined(HAVE_FSEEK64) || defined(__MINGW32__)
    return (std::numeric_limits<off64_t>::max)();
#elif defined(_WIN32)
    return (std::numeric_limits<__int64>::max)();
#else
    return (std::numeric_limits<long>::max)();
#endif
}

//------------------------------------------------------------------------
// GooFile
//------------------------------------------------------------------------

#ifdef _WIN32

GooFile::GooFile(HANDLE handleA) : handle(handleA)
{
    GetFileTime(handleA, nullptr, nullptr, &modifiedTimeOnOpen);
}

int GooFile::read(char *buf, int n, Goffset offset) const
{
    DWORD m;

    LARGE_INTEGER largeInteger = {};
    largeInteger.QuadPart = offset;

    OVERLAPPED overlapped = {};
    overlapped.Offset = largeInteger.LowPart;
    overlapped.OffsetHigh = largeInteger.HighPart;

    return FALSE == ReadFile(handle, buf, n, &m, &overlapped) ? -1 : m;
}

Goffset GooFile::size() const
{
    LARGE_INTEGER size = { (DWORD)-1, -1 };

    GetFileSizeEx(handle, &size);

    return size.QuadPart;
}

GooFile *GooFile::open(const GooString *fileName)
{
#if defined(MIKTEX_WINDOWS)
  return GooFile::open(MiKTeX::Util::StringUtil::UTF8ToWideChar(fileName->c_str()).c_str());
#else
    HANDLE handle = CreateFileA(fileName->c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    return handle == INVALID_HANDLE_VALUE ? nullptr : new GooFile(handle);
#endif
}

GooFile *GooFile::open(const wchar_t *fileName)
{
    HANDLE handle = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    return handle == INVALID_HANDLE_VALUE ? nullptr : new GooFile(handle);
}

bool GooFile::modificationTimeChangedSinceOpen() const
{
    struct _FILETIME lastModified;
    GetFileTime(handle, nullptr, nullptr, &lastModified);

    return modifiedTimeOnOpen.dwHighDateTime != lastModified.dwHighDateTime || modifiedTimeOnOpen.dwLowDateTime != lastModified.dwLowDateTime;
}

#else

int GooFile::read(char *buf, int n, Goffset offset) const
{
#    ifdef HAVE_PREAD64
    return pread64(fd, buf, n, offset);
#    else
    return pread(fd, buf, n, offset);
#    endif
}

Goffset GooFile::size() const
{
#    ifdef HAVE_LSEEK64
    return lseek64(fd, 0, SEEK_END);
#    else
    return lseek(fd, 0, SEEK_END);
#    endif
}

GooFile *GooFile::open(const GooString *fileName)
{
    int fd = openFileDescriptor(fileName->c_str(), O_RDONLY);

    return fd < 0 ? nullptr : new GooFile(fd);
}

GooFile::GooFile(int fdA) : fd(fdA)
{
    struct stat statbuf;
    fstat(fd, &statbuf);
    modifiedTimeOnOpen = mtim(statbuf);
}

bool GooFile::modificationTimeChangedSinceOpen() const
{
    struct stat statbuf;
    fstat(fd, &statbuf);

    return modifiedTimeOnOpen.tv_sec != mtim(statbuf).tv_sec || modifiedTimeOnOpen.tv_nsec != mtim(statbuf).tv_nsec;
}

#endif // _WIN32

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

GDirEntry::GDirEntry(const char *dirPath, const char *nameA, bool doStat)
{
#ifdef _WIN32
    DWORD fa;
#else
    struct stat st;
#endif

    name = new GooString(nameA);
    dir = false;
    fullPath = new GooString(dirPath);
    appendToPath(fullPath, nameA);
    if (doStat) {
#if defined(MIKTEX)
      dir = MiKTeX::Core::Directory::Exists(MiKTeX::Util::PathName(fullPath->c_str()));
#else
#ifdef _WIN32
        fa = GetFileAttributesA(fullPath->c_str());
        dir = (fa != 0xFFFFFFFF && (fa & FILE_ATTRIBUTE_DIRECTORY));
#else
        if (stat(fullPath->c_str(), &st) == 0)
            dir = S_ISDIR(st.st_mode);
#endif
#endif
    }
}

GDirEntry::~GDirEntry()
{
    delete fullPath;
    delete name;
}

GDir::GDir(const char *name, bool doStatA)
{
    path = new GooString(name);
    doStat = doStatA;
#if !defined(MIKTEX) && defined(_WIN32)
    GooString *tmp;

    tmp = path->copy();
    tmp->append("/*.*");
    hnd = FindFirstFileA(tmp->c_str(), &ffd);
    delete tmp;
#else
    dir = opendir(name);
#endif
}

GDir::~GDir()
{
    delete path;
#if !defined(MIKTEX) && defined(_WIN32)
    if (hnd != INVALID_HANDLE_VALUE) {
        FindClose(hnd);
        hnd = INVALID_HANDLE_VALUE;
    }
#else
    if (dir)
        closedir(dir);
#endif
}

GDirEntry *GDir::getNextEntry()
{
    GDirEntry *e = nullptr;

#if !defined(MIKTEX) && defined(_WIN32)
    if (hnd != INVALID_HANDLE_VALUE) {
        e = new GDirEntry(path->c_str(), ffd.cFileName, doStat);
        if (!FindNextFileA(hnd, &ffd)) {
            FindClose(hnd);
            hnd = INVALID_HANDLE_VALUE;
        }
    }
#else
    struct dirent *ent;
    if (dir) {
        do {
            ent = readdir(dir);
        } while (ent && (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")));
        if (ent) {
            e = new GDirEntry(path->c_str(), ent->d_name, doStat);
        }
    }
#endif

    return e;
}

void GDir::rewind()
{
#if !defined(MIKTEX) && defined(_WIN32)
    GooString *tmp;

    if (hnd != INVALID_HANDLE_VALUE)
        FindClose(hnd);
    tmp = path->copy();
    tmp->append("/*.*");
    hnd = FindFirstFileA(tmp->c_str(), &ffd);
    delete tmp;
#else
    if (dir)
        rewinddir(dir);
#endif
}
