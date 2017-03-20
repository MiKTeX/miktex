//========================================================================
//
// SplashFTFont.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007-2009, 2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHFTFONT_H
#define SPLASHFTFONT_H

#include "poppler-config.h"

#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include "SplashFont.h"

class SplashFTFontFile;

//------------------------------------------------------------------------
// SplashFTFont
//------------------------------------------------------------------------

class SplashFTFont: public SplashFont {
public:

  SplashFTFont(SplashFTFontFile *fontFileA, SplashCoord *matA,
	       SplashCoord *textMatA);

  virtual ~SplashFTFont();

  // Munge xFrac and yFrac before calling SplashFont::getGlyph.
  GBool getGlyph(int c, int xFrac, int yFrac,
		 SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes) override;

  // Rasterize a glyph.  The <xFrac> and <yFrac> values are the same
  // as described for getGlyph.
  GBool makeGlyph(int c, int xFrac, int yFrac,
		  SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes) override;

  // Return the path for a glyph.
  SplashPath *getGlyphPath(int c) override;

  // Return the advance of a glyph. (in 0..1 range)
  double getGlyphAdvance(int c) override;

private:

  FT_Size sizeObj;
  FT_Matrix matrix;
  FT_Matrix textMatrix;
  SplashCoord textScale;
  int size;
  GBool enableFreeTypeHinting;
  GBool enableSlightHinting;
};

#endif // HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H

#endif
