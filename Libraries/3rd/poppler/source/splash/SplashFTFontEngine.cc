//========================================================================
//
// SplashFTFontEngine.cc
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
// Copyright (C) 2009, 2011, 2012, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdio>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "fofi/FoFiTrueType.h"
#include "fofi/FoFiType1C.h"
#include "SplashFTFontFile.h"
#include "SplashFTFontEngine.h"

//------------------------------------------------------------------------
// SplashFTFontEngine
//------------------------------------------------------------------------

SplashFTFontEngine::SplashFTFontEngine(bool aaA, bool enableFreeTypeHintingA, bool enableSlightHintingA, FT_Library libA)
{
    aa = aaA;
    enableFreeTypeHinting = enableFreeTypeHintingA;
    enableSlightHinting = enableSlightHintingA;
    lib = libA;
}

SplashFTFontEngine *SplashFTFontEngine::init(bool aaA, bool enableFreeTypeHintingA, bool enableSlightHintingA)
{
    FT_Library libA;

    if (FT_Init_FreeType(&libA)) {
        return nullptr;
    }
    return new SplashFTFontEngine(aaA, enableFreeTypeHintingA, enableSlightHintingA, libA);
}

SplashFTFontEngine::~SplashFTFontEngine()
{
    FT_Done_FreeType(lib);
}

SplashFontFile *SplashFTFontEngine::loadType1Font(SplashFontFileID *idA, SplashFontSrc *src, const char **enc)
{
    return SplashFTFontFile::loadType1Font(this, idA, src, enc);
}

SplashFontFile *SplashFTFontEngine::loadType1CFont(SplashFontFileID *idA, SplashFontSrc *src, const char **enc)
{
    return SplashFTFontFile::loadType1Font(this, idA, src, enc);
}

SplashFontFile *SplashFTFontEngine::loadOpenTypeT1CFont(SplashFontFileID *idA, SplashFontSrc *src, const char **enc)
{
    return SplashFTFontFile::loadType1Font(this, idA, src, enc);
}

SplashFontFile *SplashFTFontEngine::loadCIDFont(SplashFontFileID *idA, SplashFontSrc *src)
{
    return SplashFTFontFile::loadCIDFont(this, idA, src, nullptr, 0);
}

SplashFontFile *SplashFTFontEngine::loadOpenTypeCFFFont(SplashFontFileID *idA, SplashFontSrc *src, int *codeToGID, int codeToGIDLen)
{
    return SplashFTFontFile::loadCIDFont(this, idA, src, codeToGID ? codeToGID : nullptr, codeToGID ? codeToGIDLen : 0);
}

SplashFontFile *SplashFTFontEngine::loadTrueTypeFont(SplashFontFileID *idA, SplashFontSrc *src, int *codeToGID, int codeToGIDLen, int faceIndex)
{
    SplashFontFile *ret;
    ret = SplashFTFontFile::loadTrueTypeFont(this, idA, src, codeToGID, codeToGIDLen, faceIndex);
    return ret;
}
