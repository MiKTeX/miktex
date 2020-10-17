//========================================================================
//
// gbasename.cc
//
// Wrapper for libgen's basename() call which returns a std::string.
// This is a convenience method working around questionable behavior
// in the copy of basename() provided by libgen.h.
//
// According to man 3 basename:
//
//    Both dirname() and basename() may modify the contents of path, so it
//    may be desirable to pass a copy when calling one of these functions.
//
//    ...
//
//    These functions may return pointers to statically allocated memory
//    which may be overwritten by subsequent calls.  Alternatively, they
//    may return a pointer to some part of path, so that the string
//    referred to by path should not be modified or freed until the pointer
//    returned by the function is no longer required.
//
// Because basename can modify filename (for some reason), we have to
// duplicate our input into a mutable buffer before we can call it.
// The return value might be part of this mutable temporary, but not
// generally the front, so 'char *' cannot be used as our return value.
// The return value might also be a statically allocated string,
// rendering basename (and thus gbasename) non-thread-safe. Because
// we don't know how basename()'s return value is lifecycled, we need
// to duplicate it again into something whose lifecycle we can predict.
//
// This is how a method that should amount to finding the last slash
// in a string ends up requiring two memory allocations while managing
// not to be thread-safe. In a way, it's kind of impressive.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2018, 2019 Greg Knight <lyngvi@gmail.com>
// Copyright (C) 2019 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include "gbasename.h"
#ifndef _MSC_VER
#    include <libgen.h>
#endif
#include <cstdlib>
#include <cstring>

std::string gbasename(const char *filename)
{
#ifdef _MSC_VER
    char fname[_MAX_FNAME] = {}, fext[_MAX_EXT] = {};
    errno_t z = _splitpath_s(filename, NULL, 0, NULL, 0, fname, _countof(fname), fext, _countof(fext));
    return std::string(fname) + std::string(fext);
#else
    char *mutabl = strdup(filename);
    std::string retu = basename(mutabl);
    free(mutabl);
    return retu;
#endif
}
