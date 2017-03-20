//========================================================================
//
// SplashOutputDev.h
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
// Copyright (C) 2005 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2009-2016 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2011 Andrea Canciani <ranma42@gmail.com>
// Copyright (C) 2011 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012, 2015 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2015, 2016 William Bader <williambader@hotmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHOUTPUTDEV_H
#define SPLASHOUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "splash/SplashTypes.h"
#include "splash/SplashPattern.h"
#include "poppler-config.h"
#include "OutputDev.h"
#include "GfxState.h"
#include "GlobalParams.h"

class PDFDoc;
class Gfx8BitFont;
class SplashBitmap;
class Splash;
class SplashPath;
class SplashFontEngine;
class SplashFont;
class T3FontCache;
struct T3FontCacheTag;
struct T3GlyphStack;
struct SplashTransparencyGroup;

//------------------------------------------------------------------------
// Splash dynamic pattern
//------------------------------------------------------------------------

class SplashFunctionPattern: public SplashPattern {
public:

  SplashFunctionPattern(SplashColorMode colorMode, GfxState *state, GfxFunctionShading *shading);

  SplashPattern *copy() override { return new SplashFunctionPattern(colorMode, state, (GfxFunctionShading *) shading); }

  ~SplashFunctionPattern();

  GBool testPosition(int x, int y) override { return gTrue; }

  GBool isStatic() override { return gFalse; }

  GBool getColor(int x, int y, SplashColorPtr c) override;

  virtual GfxFunctionShading *getShading() { return shading; }

  GBool isCMYK() override { return gfxMode == csDeviceCMYK; }

protected:
  Matrix ictm;
  double xMin, yMin, xMax, yMax;
  GfxFunctionShading *shading;
  GfxState *state;
  SplashColorMode colorMode;
  GfxColorSpaceMode gfxMode;
};

class SplashUnivariatePattern: public SplashPattern {
public:

  SplashUnivariatePattern(SplashColorMode colorMode, GfxState *state, GfxUnivariateShading *shading);

  ~SplashUnivariatePattern();

  GBool getColor(int x, int y, SplashColorPtr c) override;

  GBool testPosition(int x, int y) override;

  GBool isStatic() override { return gFalse; }

  virtual GBool getParameter(double xs, double ys, double *t) = 0;

  virtual GfxUnivariateShading *getShading() { return shading; }

  GBool isCMYK() override { return gfxMode == csDeviceCMYK; }

protected:
  Matrix ictm;
  double t0, t1, dt;
  GfxUnivariateShading *shading;
  GfxState *state;
  SplashColorMode colorMode;
  GfxColorSpaceMode gfxMode;
};

class SplashAxialPattern: public SplashUnivariatePattern {
public:

  SplashAxialPattern(SplashColorMode colorMode, GfxState *state, GfxAxialShading *shading);

  SplashPattern *copy() override { return new SplashAxialPattern(colorMode, state, (GfxAxialShading *) shading); }

  ~SplashAxialPattern();

  GBool getParameter(double xs, double ys, double *t) override;

private:
  double x0, y0, x1, y1;
  double dx, dy, mul;
};

// see GfxState.h, GfxGouraudTriangleShading
class SplashGouraudPattern: public SplashGouraudColor {
public:

  SplashGouraudPattern(GBool bDirectColorTranslation, GfxState *state, GfxGouraudTriangleShading *shading, SplashColorMode mode);

  SplashPattern *copy() override { return new SplashGouraudPattern(bDirectColorTranslation, state, shading, mode); }

  ~SplashGouraudPattern();

  GBool getColor(int x, int y, SplashColorPtr c) override { return gFalse; }

  GBool testPosition(int x, int y) override { return gFalse; }

  GBool isStatic() override { return gFalse; }

  GBool isCMYK() override { return gfxMode == csDeviceCMYK; }

  GBool isParameterized() override { return shading->isParameterized(); }
  int getNTriangles() override { return shading->getNTriangles(); }
   void getTriangle(int i, double *x0, double *y0, double *color0,
                            double *x1, double *y1, double *color1,
                            double *x2, double *y2, double *color2) override
  { return shading->getTriangle(i, x0, y0, color0, x1, y1, color1, x2, y2, color2); }

  void getParameterizedColor(double t, SplashColorMode mode, SplashColorPtr c) override;

private:
  GfxGouraudTriangleShading *shading;
  GfxState *state;
  GBool bDirectColorTranslation;
  SplashColorMode mode;
  GfxColorSpaceMode gfxMode;
};

// see GfxState.h, GfxRadialShading
class SplashRadialPattern: public SplashUnivariatePattern {
public:

  SplashRadialPattern(SplashColorMode colorMode, GfxState *state, GfxRadialShading *shading);

  SplashPattern *copy() override { return new SplashRadialPattern(colorMode, state, (GfxRadialShading *) shading); }

  ~SplashRadialPattern();

  GBool getParameter(double xs, double ys, double *t) override;

private:
  double x0, y0, r0, dx, dy, dr;
  double a, inva;
};

//------------------------------------------------------------------------

// number of Type 3 fonts to cache
#define splashOutT3FontCacheSize 8

//------------------------------------------------------------------------
// SplashOutputDev
//------------------------------------------------------------------------

class SplashOutputDev: public OutputDev {
public:

  // Constructor.
  SplashOutputDev(SplashColorMode colorModeA, int bitmapRowPadA,
		  GBool reverseVideoA, SplashColorPtr paperColorA,
		  GBool bitmapTopDownA = gTrue,
		  SplashThinLineMode thinLineMode = splashThinLineDefault,
		  GBool overprintPreviewA = globalParams->getOverprintPreview());

  // Destructor.
  ~SplashOutputDev();

  //----- get info about output device

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  GBool useTilingPatternFill() override { return gTrue; }

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
  GBool useShadedFills(int type) override
  { return (type >= 1 && type <= 5) ? gTrue : gFalse; }

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  GBool upsideDown() override { return bitmapTopDown ^ bitmapUpsideDown; }

  // Does this device use drawChar() or drawString()?
  GBool useDrawChar() override { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  GBool interpretType3Chars() override { return gTrue; }

  //----- initialization and control

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
  void updateStrokeAdjust(GfxState *state) override;
  void updateFillColorSpace(GfxState *state) override;
  void updateStrokeColorSpace(GfxState *state) override;
  void updateFillColor(GfxState *state) override;
  void updateStrokeColor(GfxState *state) override;
  void updateBlendMode(GfxState *state) override;
  void updateFillOpacity(GfxState *state) override;
  void updateStrokeOpacity(GfxState *state) override;
  void updatePatternOpacity(GfxState *state) override;
  void clearPatternOpacity(GfxState *state) override;
  void updateFillOverprint(GfxState *state) override;
  void updateStrokeOverprint(GfxState *state) override;
  void updateOverprintMode(GfxState *state) override;
  void updateTransfer(GfxState *state) override;

  //----- update text state
  void updateFont(GfxState *state) override;

  //----- path painting
  void stroke(GfxState *state) override;
  void fill(GfxState *state) override;
  void eoFill(GfxState *state) override;
  GBool tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *catalog, Object *str,
				  double *pmat, int paintType, int tilingType, Dict *resDict,
				  double *mat, double *bbox,
				  int x0, int y0, int x1, int y1,
				  double xStep, double yStep) override;
  GBool functionShadedFill(GfxState *state, GfxFunctionShading *shading) override;
  GBool axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax) override;
  GBool radialShadedFill(GfxState *state, GfxRadialShading *shading, double tMin, double tMax) override;
  GBool gouraudTriangleShadedFill(GfxState *state, GfxGouraudTriangleShading *shading) override;

  //----- path clipping
  void clip(GfxState *state) override;
  void eoClip(GfxState *state) override;
  void clipToStrokePath(GfxState *state) override;

  //----- text drawing
  void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen) override;
  GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen) override;
  void endType3Char(GfxState *state) override;
  void beginTextObject(GfxState *state) override;
  void endTextObject(GfxState *state) override;

  //----- image drawing
  void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg) override;
  void setSoftMaskFromImageMask(GfxState *state,
					Object *ref, Stream *str,
					int width, int height, GBool invert,
					GBool inlineImg, double *baseMatrix) override;
  void unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix) override;
  void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg) override;
  void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       Stream *maskStr, int maskWidth, int maskHeight,
			       GBool maskInvert, GBool maskInterpolate) override;
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

  //----- transparency groups and soft masks
  GBool checkTransparencyGroup(GfxState *state, GBool knockout) override;
  void beginTransparencyGroup(GfxState *state, double *bbox,
				      GfxColorSpace *blendingColorSpace,
				      GBool isolated, GBool knockout,
				      GBool forSoftMask) override;
  void endTransparencyGroup(GfxState *state) override;
  void paintTransparencyGroup(GfxState *state, double *bbox) override;
  void setSoftMask(GfxState *state, double *bbox, GBool alpha,
			   Function *transferFunc, GfxColor *backdropColor) override;
  void clearSoftMask(GfxState *state) override;

  //----- special access

  // Called to indicate that a new PDF document has been loaded.
  void startDoc(PDFDoc *docA);
 
  void setPaperColor(SplashColorPtr paperColorA);

  GBool isReverseVideo() { return reverseVideo; }
  void setReverseVideo(GBool reverseVideoA) { reverseVideo = reverseVideoA; }

  // Get the bitmap and its size.
  SplashBitmap *getBitmap() { return bitmap; }
  int getBitmapWidth();
  int getBitmapHeight();

  // Returns the last rasterized bitmap, transferring ownership to the
  // caller.
  SplashBitmap *takeBitmap();

  // Set this flag to true to generate an upside-down bitmap (useful
  // for Windows BMP files).
  void setBitmapUpsideDown(GBool f) { bitmapUpsideDown = f; }

  // Get the Splash object.
  Splash *getSplash() { return splash; }

  // Get the modified region.
  void getModRegion(int *xMin, int *yMin, int *xMax, int *yMax);

  // Clear the modified region.
  void clearModRegion();

  SplashFont *getCurrentFont() { return font; }

  // If <skipTextA> is true, don't draw horizontal text.
  // If <skipRotatedTextA> is true, don't draw rotated (non-horizontal) text.
  void setSkipText(GBool skipHorizTextA, GBool skipRotatedTextA)
    { skipHorizText = skipHorizTextA; skipRotatedText = skipRotatedTextA; }

  int getNestCount() { return nestCount; }

#if 1 //~tmp: turn off anti-aliasing temporarily
  GBool getVectorAntialias() override;
  void setVectorAntialias(GBool vaa) override;
#endif

  GBool getFontAntialias() { return fontAntialias; }
  void setFontAntialias(GBool anti) { fontAntialias = anti; }

  void setFreeTypeHinting(GBool enable, GBool enableSlightHinting);

protected:
  void doUpdateFont(GfxState *state);

private:
  GBool univariateShadedFill(GfxState *state, SplashUnivariatePattern *pattern, double tMin, double tMax);

  void setupScreenParams(double hDPI, double vDPI);
  SplashPattern *getColor(GfxGray gray);
  SplashPattern *getColor(GfxRGB *rgb);
#if SPLASH_CMYK
  SplashPattern *getColor(GfxCMYK *cmyk);
  SplashPattern *getColor(GfxColor *deviceN);
#endif
  void getMatteColor( SplashColorMode colorMode, GfxImageColorMap *colorMap, GfxColor * matteColor, SplashColor splashMatteColor);
  void setOverprintMask(GfxColorSpace *colorSpace, GBool overprintFlag,
			int overprintMode, GfxColor *singleColor, GBool grayIndexed = gFalse);
  SplashPath *convertPath(GfxState *state, GfxPath *path,
			  GBool dropEmptySubpaths);
  void drawType3Glyph(GfxState *state, T3FontCache *t3Font,
		      T3FontCacheTag *tag, Guchar *data);
#ifdef USE_CMS
  GBool useIccImageSrc(void *data);
  static void iccTransform(void *data, SplashBitmap *bitmap);
  static GBool iccImageSrc(void *data, SplashColorPtr colorLine,
			Guchar *alphaLine);
#endif
  static GBool imageMaskSrc(void *data, SplashColorPtr line);
  static GBool imageSrc(void *data, SplashColorPtr colorLine,
			Guchar *alphaLine);
  static GBool alphaImageSrc(void *data, SplashColorPtr line,
			     Guchar *alphaLine);
  static GBool maskedImageSrc(void *data, SplashColorPtr line,
			      Guchar *alphaLine);
  static GBool tilingBitmapSrc(void *data, SplashColorPtr line,
			     Guchar *alphaLine);

  GBool keepAlphaChannel;	// don't fill with paper color, keep alpha channel

  SplashColorMode colorMode;
  int bitmapRowPad;
  GBool bitmapTopDown;
  GBool bitmapUpsideDown;
  GBool fontAntialias;
  GBool vectorAntialias;
  GBool overprintPreview;
  GBool enableFreeTypeHinting;
  GBool enableSlightHinting;
  GBool reverseVideo;		// reverse video mode
  SplashColor paperColor;	// paper color
  SplashScreenParams screenParams;
  GBool skipHorizText;
  GBool skipRotatedText;

  PDFDoc *doc;			// the current document
  XRef *xref;       // the xref of the current document

  SplashBitmap *bitmap;
  Splash *splash;
  SplashFontEngine *fontEngine;

  T3FontCache *			// Type 3 font cache
    t3FontCache[splashOutT3FontCacheSize];
  int nT3Fonts;			// number of valid entries in t3FontCache
  T3GlyphStack *t3GlyphStack;	// Type 3 glyph context stack

  SplashFont *font;		// current font
  GBool needFontUpdate;		// set when the font needs to be updated
  SplashPath *textClipPath;	// clipping path built with text object

  SplashTransparencyGroup *	// transparency group stack
    transpGroupStack;
  SplashBitmap *maskBitmap; // for image masks in pattern colorspace
  int nestCount;
};

#endif
