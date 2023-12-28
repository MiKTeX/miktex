//========================================================================
//
// PreScanOutputDev.cc
//
// Copyright 2005 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010, 2011, 2018-2021 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011, 2014 William Bader <williambader@hotmail.com>
// Copyright (C) 2011, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2011 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2022 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cmath>
#include "GlobalParams.h"
#include "Gfx.h"
#include "GfxFont.h"
#include "Link.h"
#include "Catalog.h"
#include "Page.h"
#include "PreScanOutputDev.h"

//------------------------------------------------------------------------
// PreScanOutputDev
//------------------------------------------------------------------------

PreScanOutputDev::PreScanOutputDev(PSLevel levelA) : level(levelA)
{
    clearStats();
}

PreScanOutputDev::~PreScanOutputDev() { }

void PreScanOutputDev::startPage(int /*pageNum*/, GfxState * /*state*/, XRef * /*xref*/) { }

void PreScanOutputDev::endPage() { }

void PreScanOutputDev::stroke(GfxState *state)
{
    double dashStart;

    check(state->getStrokeColorSpace(), state->getStrokeColor(), state->getStrokeOpacity(), state->getBlendMode());
    const std::vector<double> &dash = state->getLineDash(&dashStart);
    if (dash.size() != 0) {
        gdi = false;
    }
}

void PreScanOutputDev::fill(GfxState *state)
{
    check(state->getFillColorSpace(), state->getFillColor(), state->getFillOpacity(), state->getBlendMode());
}

void PreScanOutputDev::eoFill(GfxState *state)
{
    check(state->getFillColorSpace(), state->getFillColor(), state->getFillOpacity(), state->getBlendMode());
}

bool PreScanOutputDev::tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *catalog, GfxTilingPattern *tPat, const double *mat, int x0, int y0, int x1, int y1, double xStep, double yStep)
{
    if (tPat->getPaintType() == 1) {
        bool tilingNeeded = (x1 - x0 != 1 || y1 - y0 != 1);
        if (tilingNeeded) {
            inTilingPatternFill++;
        }
        gfx->drawForm(tPat->getContentStream(), tPat->getResDict(), mat, tPat->getBBox());
        if (tilingNeeded) {
            inTilingPatternFill--;
        }
    } else {
        check(state->getFillColorSpace(), state->getFillColor(), state->getFillOpacity(), state->getBlendMode());
    }
    return true;
}

bool PreScanOutputDev::functionShadedFill(GfxState *state, GfxFunctionShading *shading)
{
    if (shading->getColorSpace()->getMode() != csDeviceGray && shading->getColorSpace()->getMode() != csCalGray) {
        gray = false;
    }
    mono = false;
    if (state->getFillOpacity() != 1 || state->getBlendMode() != gfxBlendNormal) {
        transparency = true;
    }
    return true;
}

bool PreScanOutputDev::axialShadedFill(GfxState *state, GfxAxialShading *shading, double /*tMin*/, double /*tMax*/)
{
    if (shading->getColorSpace()->getMode() != csDeviceGray && shading->getColorSpace()->getMode() != csCalGray) {
        gray = false;
    }
    mono = false;
    if (state->getFillOpacity() != 1 || state->getBlendMode() != gfxBlendNormal) {
        transparency = true;
    }
    return true;
}

bool PreScanOutputDev::radialShadedFill(GfxState *state, GfxRadialShading *shading, double /*sMin*/, double /*sMax*/)
{
    if (shading->getColorSpace()->getMode() != csDeviceGray && shading->getColorSpace()->getMode() != csCalGray) {
        gray = false;
    }
    mono = false;
    if (state->getFillOpacity() != 1 || state->getBlendMode() != gfxBlendNormal) {
        transparency = true;
    }
    return true;
}

void PreScanOutputDev::clip(GfxState * /*state*/)
{
    //~ check for a rectangle "near" the edge of the page;
    //~   else set gdi to false
}

void PreScanOutputDev::eoClip(GfxState * /*state*/)
{
    //~ see clip()
}

void PreScanOutputDev::beginStringOp(GfxState *state)
{
    int render;
    double m11, m12, m21, m22;
    bool simpleTTF;

    render = state->getRender();
    if (!(render & 1)) {
        check(state->getFillColorSpace(), state->getFillColor(), state->getFillOpacity(), state->getBlendMode());
    }
    if ((render & 3) == 1 || (render & 3) == 2) {
        check(state->getStrokeColorSpace(), state->getStrokeColor(), state->getStrokeOpacity(), state->getBlendMode());
    }

    std::shared_ptr<const GfxFont> font = state->getFont();
    state->getFontTransMat(&m11, &m12, &m21, &m22);
    //~ this should check for external fonts that are non-TrueType
    simpleTTF = fabs(m11 + m22) < 0.01 && m11 > 0 && fabs(m12) < 0.01 && fabs(m21) < 0.01 && fabs(state->getHorizScaling() - 1) < 0.001 && (font->getType() == fontTrueType || font->getType() == fontTrueTypeOT);
    if (simpleTTF) {
        //~ need to create a FoFiTrueType object, and check for a Unicode cmap
    }
    if (state->getRender() != 0 || !simpleTTF) {
        gdi = false;
    }
}

void PreScanOutputDev::endStringOp(GfxState * /*state*/) { }

bool PreScanOutputDev::beginType3Char(GfxState * /*state*/, double /*x*/, double /*y*/, double /*dx*/, double /*dy*/, CharCode /*code*/, const Unicode * /*u*/, int /*uLen*/)
{
    // return false so all Type 3 chars get rendered (no caching)
    return false;
}

void PreScanOutputDev::endType3Char(GfxState * /*state*/) { }

void PreScanOutputDev::drawImageMask(GfxState *state, Object * /*ref*/, Stream *str, int width, int height, bool /*invert*/, bool /*interpolate*/, bool inlineImg)
{
    int i, j;

    check(state->getFillColorSpace(), state->getFillColor(), state->getFillOpacity(), state->getBlendMode());
    gdi = false;
    if ((level == psLevel1 || level == psLevel1Sep) && (state->getFillColorSpace()->getMode() == csPattern || inTilingPatternFill > 0)) {
        patternImgMask = true;
    }

    if (inlineImg) {
        str->reset();
        j = height * ((width + 7) / 8);
        for (i = 0; i < j; ++i) {
            str->getChar();
        }
        str->close();
    }
}

void PreScanOutputDev::drawImage(GfxState *state, Object * /*ref*/, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool /*interpolate*/, const int * /*maskColors*/, bool inlineImg)
{
    GfxColorSpace *colorSpace;
    int i, j;

    colorSpace = colorMap->getColorSpace();
    if (colorSpace->getMode() == csIndexed) {
        colorSpace = ((GfxIndexedColorSpace *)colorSpace)->getBase();
    }
    if (colorSpace->getMode() == csDeviceGray || colorSpace->getMode() == csCalGray) {
        if (colorMap->getBits() > 1) {
            mono = false;
        }
    } else {
        gray = false;
        mono = false;
    }
    if (state->getFillOpacity() != 1 || state->getBlendMode() != gfxBlendNormal) {
        transparency = true;
    }
    gdi = false;
    if ((level == psLevel1 || level == psLevel1Sep) && inTilingPatternFill > 0) {
        patternImgMask = true;
    }

    if (inlineImg) {
        str->reset();
        j = height * ((width * colorMap->getNumPixelComps() * colorMap->getBits() + 7) / 8);
        for (i = 0; i < j; ++i) {
            str->getChar();
        }
        str->close();
    }
}

void PreScanOutputDev::drawMaskedImage(GfxState *state, Object * /*ref*/, Stream * /*str*/, int /*width*/, int /*height*/, GfxImageColorMap *colorMap, bool /*interpolate*/, Stream * /*maskStr*/, int /*maskWidth*/, int /*maskHeight*/,
                                       bool /*maskInvert*/, bool /*maskInterpolate*/)
{
    GfxColorSpace *colorSpace;

    colorSpace = colorMap->getColorSpace();
    if (colorSpace->getMode() == csIndexed) {
        colorSpace = ((GfxIndexedColorSpace *)colorSpace)->getBase();
    }
    if (colorSpace->getMode() == csDeviceGray || colorSpace->getMode() == csCalGray) {
        if (colorMap->getBits() > 1) {
            mono = false;
        }
    } else {
        gray = false;
        mono = false;
    }
    if (state->getFillOpacity() != 1 || state->getBlendMode() != gfxBlendNormal) {
        transparency = true;
    }
    gdi = false;
}

void PreScanOutputDev::drawSoftMaskedImage(GfxState * /*state*/, Object * /*ref*/, Stream * /*str*/, int /*width*/, int /*height*/, GfxImageColorMap *colorMap, bool /*interpolate*/, Stream * /*maskStr*/, int /*maskWidth*/,
                                           int /*maskHeight*/, GfxImageColorMap * /*maskColorMap*/, bool /*maskInterpolate*/)
{
    GfxColorSpace *colorSpace;

    colorSpace = colorMap->getColorSpace();
    if (colorSpace->getMode() == csIndexed) {
        colorSpace = ((GfxIndexedColorSpace *)colorSpace)->getBase();
    }
    if (colorSpace->getMode() != csDeviceGray && colorSpace->getMode() != csCalGray) {
        gray = false;
    }
    mono = false;
    transparency = true;
    gdi = false;
}

void PreScanOutputDev::beginTransparencyGroup(GfxState * /*state*/, const double * /*bbox*/, GfxColorSpace * /*blendingColorSpace*/, bool /*isolated*/, bool /*knockout*/, bool /*forSoftMask*/)
{
    gdi = false;
}

void PreScanOutputDev::paintTransparencyGroup(GfxState *state, const double * /*bbox*/)
{
    check(state->getFillColorSpace(), state->getFillColor(), state->getFillOpacity(), state->getBlendMode());
}

void PreScanOutputDev::setSoftMask(GfxState * /*state*/, const double * /*bbox*/, bool /*alpha*/, Function * /*transferFunc*/, GfxColor * /*backdropColor*/)
{
    transparency = true;
}

void PreScanOutputDev::check(GfxColorSpace *colorSpace, const GfxColor *color, double opacity, GfxBlendMode blendMode)
{
    GfxRGB rgb;

    if (colorSpace->getMode() == csPattern) {
        mono = false;
        gray = false;
        gdi = false;
    } else {
        colorSpace->getRGB(color, &rgb);
        if (rgb.r != rgb.g || rgb.g != rgb.b || rgb.b != rgb.r) {
            mono = false;
            gray = false;
        } else if (!((rgb.r == 0 && rgb.g == 0 && rgb.b == 0) || (rgb.r == gfxColorComp1 && rgb.g == gfxColorComp1 && rgb.b == gfxColorComp1))) {
            mono = false;
        }
    }
    if (opacity != 1 || blendMode != gfxBlendNormal) {
        transparency = true;
    }
}

void PreScanOutputDev::clearStats()
{
    mono = true;
    gray = true;
    transparency = false;
    gdi = true;
    patternImgMask = false;
    inTilingPatternFill = 0;
}
