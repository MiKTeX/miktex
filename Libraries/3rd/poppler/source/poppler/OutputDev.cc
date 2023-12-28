//========================================================================
//
// OutputDev.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2006 Thorkild Stray <thorkild@ifi.uio.no>
// Copyright (C) 2007, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009, 2012, 2013, 2018, 2019 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstddef>
#include "Object.h"
#include "Stream.h"
#include "GfxState.h"
#include "OutputDev.h"

//------------------------------------------------------------------------
// OutputDev
//------------------------------------------------------------------------

OutputDev::OutputDev()
#ifdef USE_CMS
    : iccColorSpaceCache(5)
#endif
{
}

OutputDev::~OutputDev() = default;

void OutputDev::setDefaultCTM(const double *ctm)
{
    int i;
    double det;

    for (i = 0; i < 6; ++i) {
        defCTM[i] = ctm[i];
    }
    det = 1 / (defCTM[0] * defCTM[3] - defCTM[1] * defCTM[2]);
    defICTM[0] = defCTM[3] * det;
    defICTM[1] = -defCTM[1] * det;
    defICTM[2] = -defCTM[2] * det;
    defICTM[3] = defCTM[0] * det;
    defICTM[4] = (defCTM[2] * defCTM[5] - defCTM[3] * defCTM[4]) * det;
    defICTM[5] = (defCTM[1] * defCTM[4] - defCTM[0] * defCTM[5]) * det;
}

void OutputDev::cvtDevToUser(double dx, double dy, double *ux, double *uy)
{
    *ux = defICTM[0] * dx + defICTM[2] * dy + defICTM[4];
    *uy = defICTM[1] * dx + defICTM[3] * dy + defICTM[5];
}

void OutputDev::cvtUserToDev(double ux, double uy, int *dx, int *dy)
{
    *dx = (int)(defCTM[0] * ux + defCTM[2] * uy + defCTM[4] + 0.5);
    *dy = (int)(defCTM[1] * ux + defCTM[3] * uy + defCTM[5] + 0.5);
}

void OutputDev::updateAll(GfxState *state)
{
    updateLineDash(state);
    updateFlatness(state);
    updateLineJoin(state);
    updateLineCap(state);
    updateMiterLimit(state);
    updateLineWidth(state);
    updateStrokeAdjust(state);
    updateFillColorSpace(state);
    updateFillColor(state);
    updateStrokeColorSpace(state);
    updateStrokeColor(state);
    updateBlendMode(state);
    updateFillOpacity(state);
    updateStrokeOpacity(state);
    updateFillOverprint(state);
    updateStrokeOverprint(state);
    updateTransfer(state);
    updateFont(state);
}

bool OutputDev::beginType3Char(GfxState *state, double x, double y, double dx, double dy, CharCode code, const Unicode *u, int uLen)
{
    return false;
}

void OutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool interpolate, bool inlineImg)
{
    int i, j;

    if (inlineImg) {
        str->reset();
        j = height * ((width + 7) / 8);
        for (i = 0; i < j; ++i) {
            str->getChar();
        }
        str->close();
    }
}

void OutputDev::setSoftMaskFromImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool inlineImg, double *baseMatrix)
{
    drawImageMask(state, ref, str, width, height, invert, false, inlineImg);
}

void OutputDev::unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix)
{
    return;
}

void OutputDev::drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, const int *maskColors, bool inlineImg)
{
    int i, j;

    if (inlineImg) {
        str->reset();
        j = height * ((width * colorMap->getNumPixelComps() * colorMap->getBits() + 7) / 8);
        for (i = 0; i < j; ++i) {
            str->getChar();
        }
        str->close();
    }
}

void OutputDev::drawMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, bool maskInvert, bool maskInterpolate)
{
    drawImage(state, ref, str, width, height, colorMap, interpolate, nullptr, false);
}

void OutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, GfxImageColorMap *maskColorMap,
                                    bool maskInterpolate)
{
    drawImage(state, ref, str, width, height, colorMap, interpolate, nullptr, false);
}

void OutputDev::endMarkedContent(GfxState *state) { }

void OutputDev::beginMarkedContent(const char *name, Dict *properties) { }

void OutputDev::markPoint(const char *name) { }

void OutputDev::markPoint(const char *name, Dict *properties) { }

#ifdef OPI_SUPPORT
void OutputDev::opiBegin(GfxState *state, Dict *opiDict) { }

void OutputDev::opiEnd(GfxState *state, Dict *opiDict) { }
#endif

void OutputDev::startProfile()
{
    profileHash = std::make_unique<std::unordered_map<std::string, ProfileData>>();
}

std::unique_ptr<std::unordered_map<std::string, ProfileData>> OutputDev::endProfile()
{
    return std::move(profileHash);
}
