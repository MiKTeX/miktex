//========================================================================
//
// SplashBitmap.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2009, 2010, 2012, 2015, 2018, 2019, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010, 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Harry Roberts <harry.roberts@midnight-labs.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2010, 2015, 2019 William Bader <williambader@hotmail.com>
// Copyright (C) 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Anthony Wesley <awesley@smartnetworks.com.au>
// Copyright (C) 2015, 2018 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2016 Kenji Uno <ku@digitaldolphins.jp>
// Copyright (C) 2018 Martin Packman <gzlist@googlemail.com>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include "goo/gfile.h"
#include "goo/gmem.h"
#include "SplashErrorCodes.h"
#include "SplashBitmap.h"
#include "poppler/Error.h"
#include "poppler/GfxState.h"
#include "goo/JpegWriter.h"
#include "goo/PNGWriter.h"
#include "goo/TiffWriter.h"
#include "goo/ImgWriter.h"

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

SplashBitmap::SplashBitmap(int widthA, int heightA, int rowPadA, SplashColorMode modeA, bool alphaA, bool topDown, const std::vector<GfxSeparationColorSpace *> *separationListA)
{
    width = widthA;
    height = heightA;
    mode = modeA;
    rowPad = rowPadA;
    switch (mode) {
    case splashModeMono1:
        if (width > 0) {
            rowSize = (width + 7) >> 3;
        } else {
            rowSize = -1;
        }
        break;
    case splashModeMono8:
        if (width > 0) {
            rowSize = width;
        } else {
            rowSize = -1;
        }
        break;
    case splashModeRGB8:
    case splashModeBGR8:
        if (width > 0 && width <= INT_MAX / 3) {
            rowSize = width * 3;
        } else {
            rowSize = -1;
        }
        break;
    case splashModeXBGR8:
        if (width > 0 && width <= INT_MAX / 4) {
            rowSize = width * 4;
        } else {
            rowSize = -1;
        }
        break;
    case splashModeCMYK8:
        if (width > 0 && width <= INT_MAX / 4) {
            rowSize = width * 4;
        } else {
            rowSize = -1;
        }
        break;
    case splashModeDeviceN8:
        if (width > 0 && width <= static_cast<int>(INT_MAX / splashMaxColorComps)) {
            rowSize = width * splashMaxColorComps;
        } else {
            rowSize = -1;
        }
        break;
    }
    if (rowSize > 0) {
        rowSize += rowPad - 1;
        rowSize -= rowSize % rowPad;
    }
    data = (SplashColorPtr)gmallocn_checkoverflow(rowSize, height);
    if (data != nullptr) {
        if (!topDown) {
            data += (height - 1) * rowSize;
            rowSize = -rowSize;
        }
        if (alphaA) {
            alpha = (unsigned char *)gmallocn_checkoverflow(width, height);
        } else {
            alpha = nullptr;
        }
    } else {
        alpha = nullptr;
    }
    separationList = new std::vector<GfxSeparationColorSpace *>();
    if (separationListA != nullptr) {
        for (const GfxSeparationColorSpace *separation : *separationListA) {
            separationList->push_back((GfxSeparationColorSpace *)separation->copy());
        }
    }
}

SplashBitmap *SplashBitmap::copy(const SplashBitmap *src)
{
    SplashBitmap *result = new SplashBitmap(src->getWidth(), src->getHeight(), src->getRowPad(), src->getMode(), src->getAlphaPtr() != nullptr, src->getRowSize() >= 0, src->getSeparationList());
    SplashColorConstPtr dataSource = src->getDataPtr();
    unsigned char *dataDest = result->getDataPtr();
    int amount = src->getRowSize();
    if (amount < 0) {
        dataSource = dataSource + (src->getHeight() - 1) * amount;
        dataDest = dataDest + (src->getHeight() - 1) * amount;
        amount *= -src->getHeight();
    } else {
        amount *= src->getHeight();
    }
    memcpy(dataDest, dataSource, amount);
    if (src->getAlphaPtr() != nullptr) {
        memcpy(result->getAlphaPtr(), src->getAlphaPtr(), src->getWidth() * src->getHeight());
    }
    return result;
}

SplashBitmap::~SplashBitmap()
{
    if (data) {
        if (rowSize < 0) {
            gfree(data + (height - 1) * rowSize);
        } else {
            gfree(data);
        }
    }
    gfree(alpha);
    for (auto entry : *separationList) {
        delete entry;
    }
    delete separationList;
}

SplashError SplashBitmap::writePNMFile(char *fileName)
{
    FILE *f;
    SplashError e;

    if (!(f = openFile(fileName, "wb"))) {
        return splashErrOpenFile;
    }

    e = this->writePNMFile(f);

    fclose(f);
    return e;
}

SplashError SplashBitmap::writePNMFile(FILE *f)
{
    SplashColorPtr row, p;
    int x, y;

    switch (mode) {

    case splashModeMono1:
        fprintf(f, "P4\n%d %d\n", width, height);
        row = data;
        for (y = 0; y < height; ++y) {
            p = row;
            for (x = 0; x < width; x += 8) {
                fputc(*p ^ 0xff, f);
                ++p;
            }
            row += rowSize;
        }
        break;

    case splashModeMono8:
        fprintf(f, "P5\n%d %d\n255\n", width, height);
        row = data;
        for (y = 0; y < height; ++y) {
            fwrite(row, 1, width, f);
            row += rowSize;
        }
        break;

    case splashModeRGB8:
        fprintf(f, "P6\n%d %d\n255\n", width, height);
        row = data;
        for (y = 0; y < height; ++y) {
            fwrite(row, 1, 3 * width, f);
            row += rowSize;
        }
        break;

    case splashModeXBGR8:
        fprintf(f, "P6\n%d %d\n255\n", width, height);
        row = data;
        for (y = 0; y < height; ++y) {
            p = row;
            for (x = 0; x < width; ++x) {
                fputc(splashBGR8R(p), f);
                fputc(splashBGR8G(p), f);
                fputc(splashBGR8B(p), f);
                p += 4;
            }
            row += rowSize;
        }
        break;

    case splashModeBGR8:
        fprintf(f, "P6\n%d %d\n255\n", width, height);
        row = data;
        for (y = 0; y < height; ++y) {
            p = row;
            for (x = 0; x < width; ++x) {
                fputc(splashBGR8R(p), f);
                fputc(splashBGR8G(p), f);
                fputc(splashBGR8B(p), f);
                p += 3;
            }
            row += rowSize;
        }
        break;

    case splashModeCMYK8:
    case splashModeDeviceN8:
        // PNM doesn't support CMYK
        error(errInternal, -1, "unsupported SplashBitmap mode");
        return splashErrGeneric;
        break;
    }
    return splashOk;
}

SplashError SplashBitmap::writeAlphaPGMFile(char *fileName)
{
    FILE *f;

    if (!alpha) {
        return splashErrModeMismatch;
    }
    if (!(f = openFile(fileName, "wb"))) {
        return splashErrOpenFile;
    }
    fprintf(f, "P5\n%d %d\n255\n", width, height);
    fwrite(alpha, 1, width * height, f);
    fclose(f);
    return splashOk;
}

void SplashBitmap::getPixel(int x, int y, SplashColorPtr pixel)
{
    SplashColorPtr p;

    if (y < 0 || y >= height || x < 0 || x >= width || !data) {
        return;
    }
    switch (mode) {
    case splashModeMono1:
        p = &data[y * rowSize + (x >> 3)];
        pixel[0] = (p[0] & (0x80 >> (x & 7))) ? 0xff : 0x00;
        break;
    case splashModeMono8:
        p = &data[y * rowSize + x];
        pixel[0] = p[0];
        break;
    case splashModeRGB8:
        p = &data[y * rowSize + 3 * x];
        pixel[0] = p[0];
        pixel[1] = p[1];
        pixel[2] = p[2];
        break;
    case splashModeXBGR8:
        p = &data[y * rowSize + 4 * x];
        pixel[0] = p[2];
        pixel[1] = p[1];
        pixel[2] = p[0];
        pixel[3] = p[3];
        break;
    case splashModeBGR8:
        p = &data[y * rowSize + 3 * x];
        pixel[0] = p[2];
        pixel[1] = p[1];
        pixel[2] = p[0];
        break;
    case splashModeCMYK8:
        p = &data[y * rowSize + 4 * x];
        pixel[0] = p[0];
        pixel[1] = p[1];
        pixel[2] = p[2];
        pixel[3] = p[3];
        break;
    case splashModeDeviceN8:
        p = &data[y * rowSize + (SPOT_NCOMPS + 4) * x];
        for (int cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
            pixel[cp] = p[cp];
        }
        break;
    }
}

unsigned char SplashBitmap::getAlpha(int x, int y)
{
    return alpha[y * width + x];
}

SplashColorPtr SplashBitmap::takeData()
{
    SplashColorPtr data2;

    data2 = data;
    data = nullptr;
    return data2;
}

SplashError SplashBitmap::writeImgFile(SplashImageFileFormat format, const char *fileName, double hDPI, double vDPI, WriteImgParams *params)
{
    FILE *f;
    SplashError e;

    if (!(f = openFile(fileName, "wb"))) {
        return splashErrOpenFile;
    }

    e = writeImgFile(format, f, hDPI, vDPI, params);

    fclose(f);
    return e;
}

void SplashBitmap::setJpegParams(ImgWriter *writer, WriteImgParams *params)
{
#ifdef ENABLE_LIBJPEG
    if (params) {
        static_cast<JpegWriter *>(writer)->setProgressive(params->jpegProgressive);
        static_cast<JpegWriter *>(writer)->setOptimize(params->jpegOptimize);
        if (params->jpegQuality >= 0) {
            static_cast<JpegWriter *>(writer)->setQuality(params->jpegQuality);
        }
    }
#endif
}

SplashError SplashBitmap::writeImgFile(SplashImageFileFormat format, FILE *f, double hDPI, double vDPI, WriteImgParams *params)
{
    ImgWriter *writer;
    SplashError e;

    SplashColorMode imageWriterFormat = splashModeRGB8;

    switch (format) {
#ifdef ENABLE_LIBPNG
    case splashFormatPng:
        writer = new PNGWriter();
        break;
#endif

#ifdef ENABLE_LIBJPEG
    case splashFormatJpegCMYK:
        writer = new JpegWriter(JpegWriter::CMYK);
        setJpegParams(writer, params);
        break;
    case splashFormatJpeg:
        writer = new JpegWriter();
        setJpegParams(writer, params);
        break;
#endif

#ifdef ENABLE_LIBTIFF
    case splashFormatTiff:
        switch (mode) {
        case splashModeMono1:
            writer = new TiffWriter(TiffWriter::MONOCHROME);
            imageWriterFormat = splashModeMono1;
            break;
        case splashModeMono8:
            writer = new TiffWriter(TiffWriter::GRAY);
            imageWriterFormat = splashModeMono8;
            break;
        case splashModeRGB8:
        case splashModeBGR8:
            writer = new TiffWriter(TiffWriter::RGB);
            break;
        case splashModeCMYK8:
        case splashModeDeviceN8:
            writer = new TiffWriter(TiffWriter::CMYK);
            break;
        default:
            fprintf(stderr, "TiffWriter: Mode %d not supported\n", mode);
            writer = new TiffWriter();
        }
        if (writer && params) {
            ((TiffWriter *)writer)->setCompressionString(params->tiffCompression.c_str());
        }
        break;
#endif

    default:
        // Not the greatest error message, but users of this function should
        // have already checked whether their desired format is compiled in.
        error(errInternal, -1, "Support for this image type not compiled in");
        return splashErrGeneric;
    }

    e = writeImgFile(writer, f, hDPI, vDPI, imageWriterFormat);
    delete writer;
    return e;
}

#include "poppler/GfxState_helpers.h"

void SplashBitmap::getRGBLine(int yl, SplashColorPtr line)
{
    SplashColor col;
    double c, m, y, k, c1, m1, y1, k1, r, g, b;

    for (int x = 0; x < width; x++) {
        getPixel(x, yl, col);
        c = byteToDbl(col[0]);
        m = byteToDbl(col[1]);
        y = byteToDbl(col[2]);
        k = byteToDbl(col[3]);
        if (separationList->size() > 0) {
            for (std::size_t i = 0; i < separationList->size(); i++) {
                if (col[i + 4] > 0) {
                    GfxCMYK cmyk;
                    GfxColor input;
                    input.c[0] = byteToCol(col[i + 4]);
                    GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)((*separationList)[i]);
                    sepCS->getCMYK(&input, &cmyk);
                    col[0] = colToByte(cmyk.c);
                    col[1] = colToByte(cmyk.m);
                    col[2] = colToByte(cmyk.y);
                    col[3] = colToByte(cmyk.k);
                    c += byteToDbl(col[0]);
                    m += byteToDbl(col[1]);
                    y += byteToDbl(col[2]);
                    k += byteToDbl(col[3]);
                }
            }
            if (c > 1) {
                c = 1;
            }
            if (m > 1) {
                m = 1;
            }
            if (y > 1) {
                y = 1;
            }
            if (k > 1) {
                k = 1;
            }
        }
        c1 = 1 - c;
        m1 = 1 - m;
        y1 = 1 - y;
        k1 = 1 - k;
        cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
        *line++ = dblToByte(clip01(r));
        *line++ = dblToByte(clip01(g));
        *line++ = dblToByte(clip01(b));
    }
}

void SplashBitmap::getXBGRLine(int yl, SplashColorPtr line, ConversionMode conversionMode)
{
    SplashColor col;
    double c, m, y, k, c1, m1, y1, k1, r, g, b;

    for (int x = 0; x < width; x++) {
        getPixel(x, yl, col);
        c = byteToDbl(col[0]);
        m = byteToDbl(col[1]);
        y = byteToDbl(col[2]);
        k = byteToDbl(col[3]);
        if (separationList->size() > 0) {
            for (std::size_t i = 0; i < separationList->size(); i++) {
                if (col[i + 4] > 0) {
                    GfxCMYK cmyk;
                    GfxColor input;
                    input.c[0] = byteToCol(col[i + 4]);
                    GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)((*separationList)[i]);
                    sepCS->getCMYK(&input, &cmyk);
                    col[0] = colToByte(cmyk.c);
                    col[1] = colToByte(cmyk.m);
                    col[2] = colToByte(cmyk.y);
                    col[3] = colToByte(cmyk.k);
                    c += byteToDbl(col[0]);
                    m += byteToDbl(col[1]);
                    y += byteToDbl(col[2]);
                    k += byteToDbl(col[3]);
                }
            }
            if (c > 1) {
                c = 1;
            }
            if (m > 1) {
                m = 1;
            }
            if (y > 1) {
                y = 1;
            }
            if (k > 1) {
                k = 1;
            }
        }
        c1 = 1 - c;
        m1 = 1 - m;
        y1 = 1 - y;
        k1 = 1 - k;
        cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);

        if (conversionMode == conversionAlphaPremultiplied) {
            const double a = getAlpha(x, yl) / 255.0;

            *line++ = dblToByte(clip01(b * a));
            *line++ = dblToByte(clip01(g * a));
            *line++ = dblToByte(clip01(r * a));
        } else {
            *line++ = dblToByte(clip01(b));
            *line++ = dblToByte(clip01(g));
            *line++ = dblToByte(clip01(r));
        }

        if (conversionMode != conversionOpaque) {
            *line++ = getAlpha(x, yl);
        } else {
            *line++ = 255;
        }
    }
}

static inline unsigned char div255(int x)
{
    return (unsigned char)((x + (x >> 8) + 0x80) >> 8);
}

bool SplashBitmap::convertToXBGR(ConversionMode conversionMode)
{
    if (mode == splashModeXBGR8) {
        if (conversionMode != conversionOpaque) {
            // Copy the alpha channel into the fourth component so that XBGR becomes ABGR.
            const SplashColorPtr dbegin = data;
            const SplashColorPtr dend = data + rowSize * height;

            unsigned char *const abegin = alpha;
            unsigned char *const aend = alpha + width * height;

            SplashColorPtr d = dbegin;
            unsigned char *a = abegin;

            if (conversionMode == conversionAlphaPremultiplied) {
                for (; d < dend && a < aend; d += 4, a += 1) {
                    d[0] = div255(d[0] * *a);
                    d[1] = div255(d[1] * *a);
                    d[2] = div255(d[2] * *a);
                    d[3] = *a;
                }
            } else {
                for (d += 3; d < dend && a < aend; d += 4, a += 1) {
                    *d = *a;
                }
            }
        }

        return true;
    }

    int newrowSize = width * 4;
    SplashColorPtr newdata = (SplashColorPtr)gmallocn_checkoverflow(newrowSize, height);
    if (newdata != nullptr) {
        for (int y = 0; y < height; y++) {
            unsigned char *row = newdata + y * newrowSize;
            getXBGRLine(y, row, conversionMode);
        }
        if (rowSize < 0) {
            gfree(data + (height - 1) * rowSize);
        } else {
            gfree(data);
        }
        data = newdata;
        rowSize = newrowSize;
        mode = splashModeXBGR8;
    }
    return newdata != nullptr;
}

void SplashBitmap::getCMYKLine(int yl, SplashColorPtr line)
{
    SplashColor col;

    for (int x = 0; x < width; x++) {
        getPixel(x, yl, col);
        if (separationList->size() > 0) {
            double c, m, y, k;
            c = byteToDbl(col[0]);
            m = byteToDbl(col[1]);
            y = byteToDbl(col[2]);
            k = byteToDbl(col[3]);
            for (std::size_t i = 0; i < separationList->size(); i++) {
                if (col[i + 4] > 0) {
                    GfxCMYK cmyk;
                    GfxColor input;
                    input.c[0] = byteToCol(col[i + 4]);
                    GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)((*separationList)[i]);
                    sepCS->getCMYK(&input, &cmyk);
                    col[0] = colToByte(cmyk.c);
                    col[1] = colToByte(cmyk.m);
                    col[2] = colToByte(cmyk.y);
                    col[3] = colToByte(cmyk.k);
                    c += byteToDbl(col[0]);
                    m += byteToDbl(col[1]);
                    y += byteToDbl(col[2]);
                    k += byteToDbl(col[3]);
                }
            }
            col[0] = dblToByte(clip01(c));
            col[1] = dblToByte(clip01(m));
            col[2] = dblToByte(clip01(y));
            col[3] = dblToByte(clip01(k));
        }
        *line++ = col[0];
        *line++ = col[1];
        *line++ = col[2];
        *line++ = col[3];
    }
}

SplashError SplashBitmap::writeImgFile(ImgWriter *writer, FILE *f, double hDPI, double vDPI, SplashColorMode imageWriterFormat)
{
    if (mode != splashModeRGB8 && mode != splashModeMono8 && mode != splashModeMono1 && mode != splashModeXBGR8 && mode != splashModeBGR8 && mode != splashModeCMYK8 && mode != splashModeDeviceN8) {
        error(errInternal, -1, "unsupported SplashBitmap mode");
        return splashErrGeneric;
    }

    if (!writer->init(f, width, height, hDPI, vDPI)) {
        return splashErrGeneric;
    }

    switch (mode) {
    case splashModeCMYK8:
        if (writer->supportCMYK()) {
            SplashColorPtr row;
            unsigned char **row_pointers = new unsigned char *[height];
            row = data;

            for (int y = 0; y < height; ++y) {
                row_pointers[y] = row;
                row += rowSize;
            }
            if (!writer->writePointers(row_pointers, height)) {
                delete[] row_pointers;
                return splashErrGeneric;
            }
            delete[] row_pointers;
        } else {
            unsigned char *row = new unsigned char[3 * width];
            for (int y = 0; y < height; y++) {
                getRGBLine(y, row);
                if (!writer->writeRow(&row)) {
                    delete[] row;
                    return splashErrGeneric;
                }
            }
            delete[] row;
        }
        break;
    case splashModeDeviceN8:
        if (writer->supportCMYK()) {
            unsigned char *row = new unsigned char[4 * width];
            for (int y = 0; y < height; y++) {
                getCMYKLine(y, row);
                if (!writer->writeRow(&row)) {
                    delete[] row;
                    return splashErrGeneric;
                }
            }
            delete[] row;
        } else {
            unsigned char *row = new unsigned char[3 * width];
            for (int y = 0; y < height; y++) {
                getRGBLine(y, row);
                if (!writer->writeRow(&row)) {
                    delete[] row;
                    return splashErrGeneric;
                }
            }
            delete[] row;
        }
        break;
    case splashModeRGB8: {
        SplashColorPtr row;
        unsigned char **row_pointers = new unsigned char *[height];
        row = data;

        for (int y = 0; y < height; ++y) {
            row_pointers[y] = row;
            row += rowSize;
        }
        if (!writer->writePointers(row_pointers, height)) {
            delete[] row_pointers;
            return splashErrGeneric;
        }
        delete[] row_pointers;
    } break;

    case splashModeBGR8: {
        unsigned char *row = new unsigned char[3 * width];
        for (int y = 0; y < height; y++) {
            // Convert into a PNG row
            for (int x = 0; x < width; x++) {
                row[3 * x] = data[y * rowSize + x * 3 + 2];
                row[3 * x + 1] = data[y * rowSize + x * 3 + 1];
                row[3 * x + 2] = data[y * rowSize + x * 3];
            }

            if (!writer->writeRow(&row)) {
                delete[] row;
                return splashErrGeneric;
            }
        }
        delete[] row;
    } break;

    case splashModeXBGR8: {
        unsigned char *row = new unsigned char[3 * width];
        for (int y = 0; y < height; y++) {
            // Convert into a PNG row
            for (int x = 0; x < width; x++) {
                row[3 * x] = data[y * rowSize + x * 4 + 2];
                row[3 * x + 1] = data[y * rowSize + x * 4 + 1];
                row[3 * x + 2] = data[y * rowSize + x * 4];
            }

            if (!writer->writeRow(&row)) {
                delete[] row;
                return splashErrGeneric;
            }
        }
        delete[] row;
    } break;

    case splashModeMono8: {
        if (imageWriterFormat == splashModeMono8) {
            SplashColorPtr row;
            unsigned char **row_pointers = new unsigned char *[height];
            row = data;

            for (int y = 0; y < height; ++y) {
                row_pointers[y] = row;
                row += rowSize;
            }
            if (!writer->writePointers(row_pointers, height)) {
                delete[] row_pointers;
                return splashErrGeneric;
            }
            delete[] row_pointers;
        } else if (imageWriterFormat == splashModeRGB8) {
            unsigned char *row = new unsigned char[3 * width];
            for (int y = 0; y < height; y++) {
                // Convert into a PNG row
                for (int x = 0; x < width; x++) {
                    row[3 * x] = data[y * rowSize + x];
                    row[3 * x + 1] = data[y * rowSize + x];
                    row[3 * x + 2] = data[y * rowSize + x];
                }

                if (!writer->writeRow(&row)) {
                    delete[] row;
                    return splashErrGeneric;
                }
            }
            delete[] row;
        } else {
            // only splashModeMono8 or splashModeRGB8
            return splashErrGeneric;
        }
    } break;

    case splashModeMono1: {
        if (imageWriterFormat == splashModeMono1) {
            SplashColorPtr row;
            unsigned char **row_pointers = new unsigned char *[height];
            row = data;

            for (int y = 0; y < height; ++y) {
                row_pointers[y] = row;
                row += rowSize;
            }
            if (!writer->writePointers(row_pointers, height)) {
                delete[] row_pointers;
                return splashErrGeneric;
            }
            delete[] row_pointers;
        } else if (imageWriterFormat == splashModeRGB8) {
            unsigned char *row = new unsigned char[3 * width];
            for (int y = 0; y < height; y++) {
                // Convert into a PNG row
                for (int x = 0; x < width; x++) {
                    getPixel(x, y, &row[3 * x]);
                    row[3 * x + 1] = row[3 * x];
                    row[3 * x + 2] = row[3 * x];
                }

                if (!writer->writeRow(&row)) {
                    delete[] row;
                    return splashErrGeneric;
                }
            }
            delete[] row;
        } else {
            // only splashModeMono1 or splashModeRGB8
            return splashErrGeneric;
        }
    } break;

    default:
        // can't happen
        break;
    }

    if (!writer->close()) {
        return splashErrGeneric;
    }

    return splashOk;
}
