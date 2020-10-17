//========================================================================
//
// SplashFontFile.cc
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
// Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#include <cstdio>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "SplashFontFile.h"
#include "SplashFontFileID.h"

//------------------------------------------------------------------------
// SplashFontFile
//------------------------------------------------------------------------

SplashFontFile::SplashFontFile(SplashFontFileID *idA, SplashFontSrc *srcA)
{
    id = idA;
    src = srcA;
    src->ref();
    refCnt = 0;
    doAdjustMatrix = false;
}

SplashFontFile::~SplashFontFile()
{
    src->unref();
    delete id;
}

void SplashFontFile::incRefCnt()
{
    ++refCnt;
}

void SplashFontFile::decRefCnt()
{
    if (!--refCnt) {
        delete this;
    }
}

//

SplashFontSrc::SplashFontSrc()
{
    isFile = false;
    deleteSrc = false;
    fileName = nullptr;
    buf = nullptr;
    refcnt = 1;
}

SplashFontSrc::~SplashFontSrc()
{
    if (deleteSrc) {
        if (isFile) {
            if (fileName)
                unlink(fileName->c_str());
        } else {
            if (buf)
                gfree(buf);
        }
    }

    if (isFile && fileName)
        delete fileName;
}

void SplashFontSrc::ref()
{
    refcnt++;
}

void SplashFontSrc::unref()
{
    if (!--refcnt)
        delete this;
}

void SplashFontSrc::setFile(GooString *file, bool del)
{
    isFile = true;
    fileName = file->copy();
    deleteSrc = del;
}

void SplashFontSrc::setFile(const char *file, bool del)
{
    isFile = true;
    fileName = new GooString(file);
    deleteSrc = del;
}

void SplashFontSrc::setBuf(char *bufA, int bufLenA, bool del)
{
    isFile = false;
    buf = bufA;
    bufLen = bufLenA;
    deleteSrc = del;
}
