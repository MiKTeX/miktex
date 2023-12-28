//========================================================================
//
// InMemoryFile.h
//
// Represents a file in-memory with GNU's stdio wrappers.
// NOTE as of this writing, open() depends on the glibc 'fopencookie'
// extension and is not supported on other platforms. The
// HAVE_IN_MEMORY_FILE macro is intended to reflect whether this class is
// usable.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2018, 2019 Greg Knight <lyngvi@gmail.com>
// Copyright (C) 2022 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef IN_MEMORY_FILE_H
#define IN_MEMORY_FILE_H

#include <cstdio>
#include <string>
#include <vector>

#if defined(__USE_GNU) && !defined(__ANDROID_API__)
#    define HAVE_IN_MEMORY_FILE (1)
#    define HAVE_IN_MEMORY_FILE_FOPENCOOKIE (1) // used internally
#endif

class InMemoryFile
{
private:
#ifdef HAVE_IN_MEMORY_FILE_FOPENCOOKIE
    size_t iohead = 0;
    FILE *fptr = nullptr;
#endif
    std::vector<char> data;

#ifdef HAVE_IN_MEMORY_FILE_FOPENCOOKIE
    ssize_t _read(char *buf, size_t sz);
    ssize_t _write(const char *buf, size_t sz);
    int _seek(off64_t *offset, int whence);
#endif

public:
    InMemoryFile();

public:
    /* Returns a file handle for this file. This is scoped to this object
     * and must be fclosed() by the caller before destruction. */
    FILE *open(const char *mode);

    const std::vector<char> &getBuffer() const { return data; }
};

#endif // IN_MEMORY_FILE_H
