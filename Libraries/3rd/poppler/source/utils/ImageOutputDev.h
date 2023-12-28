//========================================================================
//
// ImageOutputDev.h
//
// Copyright 1998-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Rainer Keller <class321@gmx.de>
// Copyright (C) 2008 Timothy Lee <timothy.lee@siriushk.com>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Jakob Voss <jakob.voss@gbv.de>
// Copyright (C) 2012, 2013, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2018, 2019, 2021 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef IMAGEOUTPUTDEV_H
#define IMAGEOUTPUTDEV_H

#include "poppler/poppler-config.h"

#include <cstdio>
#include "goo/ImgWriter.h"
#include "OutputDev.h"

class GfxState;

//------------------------------------------------------------------------
// ImageOutputDev
//------------------------------------------------------------------------

class ImageOutputDev : public OutputDev
{
public:
    enum ImageType
    {
        imgImage,
        imgStencil,
        imgMask,
        imgSmask
    };
    enum ImageFormat
    {
        imgRGB,
        imgRGB48,
        imgGray,
        imgMonochrome,
        imgCMYK
    };

    // Create an OutputDev which will write images to files named
    // <fileRoot>-NNN.<type> or <fileRoot>-PPP-NNN.<type>, if
    // <pageNames> is set. Normally, all images are written as PBM
    // (.pbm) or PPM (.ppm) files unless PNG or Tiff output is enabled
    // (PNG is used if both are enabled).  If Jpeg is enabled, JPEG images
    // are written as JPEG (.jpg) files.
    ImageOutputDev(char *fileRootA, bool pageNamesA, bool listImagesA);

    // Destructor.
    ~ImageOutputDev() override;

    // Use PNG format for output
    void enablePNG(bool png) { outputPNG = png; }

    // Use TIFF format for output
    void enableTiff(bool tiff) { outputTiff = tiff; }

    // Use Jpeg format for Jpeg files
    void enableJpeg(bool jpeg) { dumpJPEG = jpeg; }

    // Use Jpeg2000 format for Jpeg2000 files
    void enableJpeg2000(bool jp2) { dumpJP2 = jp2; }

    // Use JBIG2 format for JBIG2 files
    void enableJBig2(bool jbig2) { dumpJBIG2 = jbig2; }

    // Use CCITT format for CCITT files
    void enableCCITT(bool ccitt) { dumpCCITT = ccitt; }

    // Check if file was successfully created.
    virtual bool isOk() { return ok; }

    // Does this device use tilingPatternFill()?  If this returns false,
    // tiling pattern fills will be reduced to a series of other drawing
    // operations.
    bool useTilingPatternFill() override { return true; }

    // Does this device use beginType3Char/endType3Char?  Otherwise,
    // text in Type 3 fonts will be drawn with drawChar/drawString.
    bool interpretType3Chars() override { return false; }

    // Does this device need non-text content?
    bool needNonText() override { return true; }

    // Start a page
    void startPage(int pageNumA, GfxState *state, XRef *xref) override { pageNum = pageNumA; }

    //---- get info about output device

    // Does this device use upside-down coordinates?
    // (Upside-down means (0,0) is the top left corner of the page.)
    bool upsideDown() override { return true; }

    // Does this device use drawChar() or drawString()?
    bool useDrawChar() override { return false; }

    //----- path painting
    bool tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *cat, GfxTilingPattern *tPat, const double *mat, int x0, int y0, int x1, int y1, double xStep, double yStep) override;

    //----- image drawing
    void drawImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool interpolate, bool inlineImg) override;
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, const int *maskColors, bool inlineImg) override;
    void drawMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, bool maskInvert, bool maskInterpolate) override;
    void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, GfxImageColorMap *maskColorMap,
                             bool maskInterpolate) override;

private:
    // Sets the output filename with a given file extension
    void setFilename(const char *fileExt);
    void listImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, bool inlineImg, ImageType imageType);
    void writeImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool inlineImg);
    void writeRawImage(Stream *str, const char *ext);
    void writeImageFile(ImgWriter *writer, ImageFormat format, const char *ext, Stream *str, int width, int height, GfxImageColorMap *colorMap);
    long getInlineImageLength(Stream *str, int width, int height, GfxImageColorMap *colorMap);

    char *fileRoot; // root of output file names
    char *fileName; // buffer for output file names
    bool listImages; // list images instead of dumping
    bool dumpJPEG; // set to dump native JPEG files
    bool dumpJP2; // set to dump native JPEG2000 files
    bool dumpJBIG2; // set to dump native JBIG2 files
    bool dumpCCITT; // set to dump native CCITT files
    bool outputPNG; // set to output in PNG format
    bool outputTiff; // set to output in TIFF format
    bool pageNames; // set to include page number in file names
    int pageNum; // current page number
    int imgNum; // current image number
    bool ok; // set up ok?
};

#endif
