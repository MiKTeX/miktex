//========================================================================
//
// FoFiBase.cc
//
// Copyright 1999-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008 Ed Avis <eda@waniasset.com>
// Copyright (C) 2011 Jim Meyering <jim@meyering.net>
// Copyright (C) 2016, 2018, 2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
// Copyright (C) 2019 LE GARREC Vincent <legarrec.vincent@gmail.com>
// Copyright (C) 2022 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdio>
#include <climits>
#include "goo/gfile.h"
#include "goo/gmem.h"
#include "poppler/Error.h"
#include "FoFiBase.h"

//------------------------------------------------------------------------
// FoFiBase
//------------------------------------------------------------------------

FoFiBase::FoFiBase(const unsigned char *fileA, int lenA, bool freeFileDataA)
{
    file = fileA;
    len = lenA;
    freeFileData = freeFileDataA;
}

FoFiBase::~FoFiBase()
{
    if (freeFileData) {
        gfree((char *)file);
    }
}

char *FoFiBase::readFile(const char *fileName, int *fileLen)
{
    FILE *f;
    char *buf;
    int n;

    if (!(f = openFile(fileName, "rb"))) {
        error(errIO, -1, "Cannot open '{0:s}'", fileName);
        return nullptr;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        error(errIO, -1, "Cannot seek to end of '{0:s}'", fileName);
        fclose(f);
        return nullptr;
    }
    n = (int)ftell(f);
    if (n < 0) {
        error(errIO, -1, "Cannot determine length of '{0:s}'", fileName);
        fclose(f);
        return nullptr;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        error(errIO, -1, "Cannot seek to start of '{0:s}'", fileName);
        fclose(f);
        return nullptr;
    }
    buf = (char *)gmalloc(n);
    if ((int)fread(buf, 1, n, f) != n) {
        gfree(buf);
        fclose(f);
        return nullptr;
    }
    fclose(f);
    *fileLen = n;
    return buf;
}

int FoFiBase::getS8(int pos, bool *ok) const
{
    int x;

    if (pos < 0 || pos >= len) {
        *ok = false;
        return 0;
    }
    x = file[pos];
    if (x & 0x80) {
        x |= ~0xff;
    }
    return x;
}

int FoFiBase::getU8(int pos, bool *ok) const
{
    if (pos < 0 || pos >= len) {
        *ok = false;
        return 0;
    }
    return file[pos];
}

int FoFiBase::getS16BE(int pos, bool *ok) const
{
    int x;

    if (pos < 0 || pos > INT_MAX - 1 || pos + 1 >= len) {
        *ok = false;
        return 0;
    }
    x = file[pos];
    x = (x << 8) + file[pos + 1];
    if (x & 0x8000) {
        x |= ~0xffff;
    }
    return x;
}

int FoFiBase::getU16BE(int pos, bool *ok) const
{
    int x;

    if (pos < 0 || pos > INT_MAX - 1 || pos + 1 >= len) {
        *ok = false;
        return 0;
    }
    x = file[pos];
    x = (x << 8) + file[pos + 1];
    return x;
}

int FoFiBase::getS32BE(int pos, bool *ok) const
{
    int x;

    if (pos < 0 || pos > INT_MAX - 3 || pos + 3 >= len) {
        *ok = false;
        return 0;
    }
    x = file[pos];
    x = (x << 8) + file[pos + 1];
    x = (x << 8) + file[pos + 2];
    x = (x << 8) + file[pos + 3];
    if (x & 0x80000000) {
        x |= ~0xffffffff;
    }
    return x;
}

unsigned int FoFiBase::getU32BE(int pos, bool *ok) const
{
    unsigned int x;

    if (pos < 0 || pos > INT_MAX - 3 || pos + 3 >= len) {
        *ok = false;
        return 0;
    }
    x = file[pos];
    x = (x << 8) + file[pos + 1];
    x = (x << 8) + file[pos + 2];
    x = (x << 8) + file[pos + 3];
    return x;
}

unsigned int FoFiBase::getU32LE(int pos, bool *ok) const
{
    unsigned int x;

    if (pos < 0 || pos > INT_MAX - 3 || pos + 3 >= len) {
        *ok = false;
        return 0;
    }
    x = file[pos + 3];
    x = (x << 8) + file[pos + 2];
    x = (x << 8) + file[pos + 1];
    x = (x << 8) + file[pos];
    return x;
}

unsigned int FoFiBase::getUVarBE(int pos, int size, bool *ok) const
{
    unsigned int x;
    int i;

    if (pos < 0 || pos > INT_MAX - size || pos + size > len) {
        *ok = false;
        return 0;
    }
    x = 0;
    for (i = 0; i < size; ++i) {
        x = (x << 8) + file[pos + i];
    }
    return x;
}

bool FoFiBase::checkRegion(int pos, int size) const
{
    return pos >= 0 && size >= 0 && pos < INT_MAX - size && size < INT_MAX - pos && pos + size >= pos && pos + size <= len;
}
