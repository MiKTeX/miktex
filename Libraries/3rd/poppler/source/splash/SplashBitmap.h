//========================================================================
//
// SplashBitmap.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009, 2012, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Harry Roberts <harry.roberts@midnight-labs.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2010 William Bader <williambader@hotmail.com>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2015 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2016 Kenji Uno <ku@digitaldolphins.jp>
// Copyright (C) 2018 Martin Packman <gzlist@googlemail.com>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHBITMAP_H
#define SPLASHBITMAP_H

#include "SplashTypes.h"
#include "poppler/GfxState.h"
#include <cstdio>

class ImgWriter;

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

class SplashBitmap
{
public:
    // Create a new bitmap.  It will have <widthA> x <heightA> pixels in
    // color mode <modeA>.  Rows will be padded out to a multiple of
    // <rowPad> bytes.  If <topDown> is false, the bitmap will be stored
    // upside-down, i.e., with the last row first in memory.
    SplashBitmap(int widthA, int heightA, int rowPad, SplashColorMode modeA, bool alphaA, bool topDown = true, std::vector<GfxSeparationColorSpace *> *separationList = nullptr);
    static SplashBitmap *copy(SplashBitmap *src);

    ~SplashBitmap();

    SplashBitmap(const SplashBitmap &) = delete;
    SplashBitmap &operator=(const SplashBitmap &) = delete;

    int getWidth() { return width; }
    int getHeight() { return height; }
    int getRowSize() { return rowSize; }
    int getAlphaRowSize() { return width; }
    int getRowPad() { return rowPad; }
    SplashColorMode getMode() { return mode; }
    SplashColorPtr getDataPtr() { return data; }
    unsigned char *getAlphaPtr() { return alpha; }
    std::vector<GfxSeparationColorSpace *> *getSeparationList() { return separationList; }

    SplashError writePNMFile(char *fileName);
    SplashError writePNMFile(FILE *f);
    SplashError writeAlphaPGMFile(char *fileName);

    struct WriteImgParams
    {
        int jpegQuality = -1;
        bool jpegProgressive = false;
        GooString tiffCompression;
        bool jpegOptimize = false;
    };

    SplashError writeImgFile(SplashImageFileFormat format, const char *fileName, int hDPI, int vDPI, WriteImgParams *params = nullptr);
    SplashError writeImgFile(SplashImageFileFormat format, FILE *f, int hDPI, int vDPI, WriteImgParams *params = nullptr);
    SplashError writeImgFile(ImgWriter *writer, FILE *f, int hDPI, int vDPI, SplashColorMode imageWriterFormat);

    enum ConversionMode
    {
        conversionOpaque,
        conversionAlpha,
        conversionAlphaPremultiplied
    };

    bool convertToXBGR(ConversionMode conversionMode = conversionOpaque);

    void getPixel(int x, int y, SplashColorPtr pixel);
    void getRGBLine(int y, SplashColorPtr line);
    void getXBGRLine(int y, SplashColorPtr line, ConversionMode conversionMode = conversionOpaque);
    void getCMYKLine(int y, SplashColorPtr line);
    unsigned char getAlpha(int x, int y);

    // Caller takes ownership of the bitmap data.  The SplashBitmap
    // object is no longer valid -- the next call should be to the
    // destructor.
    SplashColorPtr takeData();

private:
    int width, height; // size of bitmap
    int rowPad;
    int rowSize; // size of one row of data, in bytes
                 //   - negative for bottom-up bitmaps
    SplashColorMode mode; // color mode
    SplashColorPtr data; // pointer to row zero of the color data
    unsigned char *alpha; // pointer to row zero of the alpha data
                          //   (always top-down)
    std::vector<GfxSeparationColorSpace *> *separationList; // list of spot colorants and their mapping functions

    friend class Splash;

    void setJpegParams(ImgWriter *writer, WriteImgParams *params);
};

#endif
