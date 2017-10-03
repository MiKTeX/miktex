//========================================================================
//
// ArthurOutputDev.h
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2005 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Pino Toscano <pino@kde.org>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Mihai Niculescu <q.quark@gmail.com>
// Copyright (C) 2017 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef ARTHUROUTPUTDEV_H
#define ARTHUROUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <memory>
#include <map>

#include "goo/gtypes.h"
#include "OutputDev.h"
#include "GfxState.h"

#include <QtGui/QPainter>

class GfxState;
class GfxPath;
class Gfx8BitFont;
struct GfxRGB;

class SplashFontEngine;

class QRawFont;

//------------------------------------------------------------------------
// ArthurOutputDev - Qt 5 QPainter renderer
//------------------------------------------------------------------------

class ArthurOutputDev: public OutputDev {
public:
  /**
   * Describes how fonts are distorted (aka hinted) to fit the pixel grid.
   * More hinting means sharper edges and less adherence to the true letter shapes.
   */
  enum FontHinting {
    NoHinting = 0, ///< Font shapes are left unchanged
    SlightHinting, ///< Font shapes are distorted vertically only
    FullHinting ///< Font shapes are distorted horizontally and vertically
  };

  // Constructor.
  ArthurOutputDev(QPainter *painter );

  // Destructor.
  ~ArthurOutputDev();

  void setFontHinting(FontHinting hinting) { m_fontHinting = hinting; }

  //----- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  GBool upsideDown() override { return gTrue; }

  // Does this device use drawChar() or drawString()?
  GBool useDrawChar() override { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  GBool interpretType3Chars() override { return gTrue; }

  //----- initialization and control

  // Set Current Transformation Matrix to a fixed matrix given in ctm[0],...,ctm[5]
  void setDefaultCTM(double *ctm) override;

  // Start a page.
  void startPage(int pageNum, GfxState *state, XRef *xref) override;

  // End a page.
  void endPage() override;

  //----- save/restore graphics state
  void saveState(GfxState *state) override;
  void restoreState(GfxState *state) override;

  //----- update graphics state
  void updateAll(GfxState *state) override;
  void updateCTM(GfxState *state, double m11, double m12,
		 double m21, double m22, double m31, double m32) override;
  void updateLineDash(GfxState *state) override;
  void updateFlatness(GfxState *state) override;
  void updateLineJoin(GfxState *state) override;
  void updateLineCap(GfxState *state) override;
  void updateMiterLimit(GfxState *state) override;
  void updateLineWidth(GfxState *state) override;
  void updateFillColor(GfxState *state) override;
  void updateStrokeColor(GfxState *state) override;
  void updateFillOpacity(GfxState *state) override;
  void updateStrokeOpacity(GfxState *state) override;

  //----- update text state
  void updateFont(GfxState *state) override;

  //----- path painting
  void stroke(GfxState *state) override;
  void fill(GfxState *state) override;
  void eoFill(GfxState *state) override;

  //----- path clipping
  void clip(GfxState *state) override;
  void eoClip(GfxState *state) override;

  //----- text drawing
  //   virtual void drawString(GfxState *state, GooString *s);
  void drawChar(GfxState *state, double x, double y,
		double dx, double dy,
		double originX, double originY,
		CharCode code, int nBytes, Unicode *u, int uLen) override;
  GBool beginType3Char(GfxState *state, double x, double y,
		       double dx, double dy,
		       CharCode code, Unicode *u, int uLen) override;
  void endType3Char(GfxState *state) override;
  void endTextObject(GfxState *state) override;

  //----- image drawing
  void drawImageMask(GfxState *state, Object *ref, Stream *str,
		     int width, int height, GBool invert,
		     GBool interpolate, GBool inlineImg) override;
  void drawImage(GfxState *state, Object *ref, Stream *str,
		 int width, int height, GfxImageColorMap *colorMap,
		 GBool interpolate, int *maskColors, GBool inlineImg) override;

  void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
                           int width, int height,
                           GfxImageColorMap *colorMap,
                           GBool interpolate,
                           Stream *maskStr,
                           int maskWidth, int maskHeight,
                           GfxImageColorMap *maskColorMap,
                           GBool maskInterpolate) override;

  //----- Type 3 font operators
  void type3D0(GfxState *state, double wx, double wy) override;
  void type3D1(GfxState *state, double wx, double wy,
	       double llx, double lly, double urx, double ury) override;

  //----- special access

  // Called to indicate that a new PDF document has been loaded.
  void startDoc(XRef *xrefA);
 
  GBool isReverseVideo() { return gFalse; }
  
private:
  QPainter *m_painter;
  FontHinting m_fontHinting;
  QFont m_currentFont;
  QPen m_currentPen;
  QBrush m_currentBrush;
  GBool m_needFontUpdate;		// set when the font needs to be updated
  SplashFontEngine *m_fontEngine;
  XRef *xref;			// xref table for current document

  // The current font in use
  QRawFont* m_rawFont;

  // Identify a font by its 'Ref' and its font size
  struct ArthurFontID
  {
    Ref ref;
    double fontSize;

    bool operator<(const ArthurFontID& other) const
    {
      return (ref.num < other.ref.num)
             ||  ((ref.num == other.ref.num) && (fontSize < other.fontSize));
    }
  };

  // Cache all fonts
  std::map<ArthurFontID,std::unique_ptr<QRawFont> > m_rawFontCache;

  // The table that maps character codes to glyph indexes
  int* m_codeToGID;
};

#endif
