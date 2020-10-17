//========================================================================
//
// BBoxOutputDev.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2020 sgerwk <sgerwk@aol.com>
//
//========================================================================

#include <cmath>
#include <BBoxOutputDev.h>
#include <GfxFont.h>

#define writingModeHorizontal 0
#define writingModeVertical 1

BBoxOutputDev::BBoxOutputDev() : BBoxOutputDev(true, true, true) { }

BBoxOutputDev::BBoxOutputDev(bool textA, bool vectorA, bool rasterA) : BBoxOutputDev(textA, vectorA, rasterA, true) { }

BBoxOutputDev::BBoxOutputDev(bool textA, bool vectorA, bool rasterA, bool lwidthA)
{
    hasGraphics = false;
    text = textA;
    vector = vectorA;
    raster = rasterA;
    lwidth = lwidthA;
}

double BBoxOutputDev::getX1() const
{
    return bb.x1;
}

double BBoxOutputDev::getY1() const
{
    return bb.y1;
}

double BBoxOutputDev::getX2() const
{
    return bb.x2;
}

double BBoxOutputDev::getY2() const
{
    return bb.y2;
}

double BBoxOutputDev::getHasGraphics() const
{
    return hasGraphics;
}

void BBoxOutputDev::endPage() { }

void BBoxOutputDev::stroke(GfxState *state)
{
    updatePath(&bb, state->getPath(), state);
}

void BBoxOutputDev::fill(GfxState *state)
{
    updatePath(&bb, state->getPath(), state);
}

void BBoxOutputDev::eoFill(GfxState *state)
{
    updatePath(&bb, state->getPath(), state);
}

void BBoxOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool interpolate, bool inlineImg)
{
    updateImage(&bb, state);
}

void BBoxOutputDev::drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, const int *maskColors, bool inlineImg)
{
    updateImage(&bb, state);
}

void BBoxOutputDev::drawMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, bool maskInvert, bool maskInterpolate)
{
    updateImage(&bb, state);
}

void BBoxOutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, GfxImageColorMap *maskColorMap,
                                        bool maskInterpolate)
{
    updateImage(&bb, state);
}

void BBoxOutputDev::drawChar(GfxState *state, double x, double y, double dx, double dy, double originX, double originY, CharCode code, int nBytes, const Unicode *u, int uLen)
{
    GfxFont *font;
    double leftent, rightent, ascent, descent;
    const double *fm, *fb;
    double fontSize, w, adjust;
    double fx, fy;

    if (!text)
        return;

    font = state->getFont();
    if (!font)
        return;

    if (code == (CharCode)0x20)
        return;

    fontSize = state->getFontSize();

    fb = font->getFontBBox();
    if (font->getWMode() == writingModeHorizontal) {
        leftent = 0;
        rightent = 0;
        ascent = font->getAscent();
        descent = font->getDescent();
    } else {
        if (fb[0] == 0 && fb[1] == 0 && fb[2] == 0 && fb[3] == 0) {
            leftent = -0.5;
            rightent = 0.5;
        } else {
            leftent = fb[1];
            rightent = fb[3];
        }
        ascent = 0;
        descent = 0;
    }

    if (font->getType() != fontType3)
        adjust = 1;
    else {
        // adjust font size for type3 fonts,
        // similar to TextPage::updateFont()
        w = ((Gfx8BitFont *)font)->getWidth(code);
        adjust = w / 0.5;
        fm = font->getFontMatrix();
        if (fm[0] != 0)
            adjust *= fabs(fm[3] / fm[0]);
    }

    ascent *= adjust * fontSize;
    descent *= adjust * fontSize;
    leftent *= adjust * fontSize;
    rightent *= adjust * fontSize;

    state->textTransformDelta(leftent, descent, &fx, &fy);
    updatePoint(&bb, fx + x, fy + y, state);

    state->textTransformDelta(rightent, ascent, &fx, &fy);
    updatePoint(&bb, fx + x, fy + y, state);

    state->textTransformDelta(leftent, descent, &fx, &fy);
    updatePoint(&bb, fx + x + dx, fy + y + dy, state);

    state->textTransformDelta(rightent, ascent, &fx, &fy);
    updatePoint(&bb, fx + x + dx, fy + y + dy, state);
}

/* update the bounding box with a new point */
void BBoxOutputDev::updatePoint(PDFRectangle *bbA, double x, double y, const GfxState *state)
{
    Matrix o = { 1, 0, 0, 1, 0, 0 };
    double tx, ty;
    double xMin, yMin, xMax, yMax;

    state->getClipBBox(&xMin, &yMin, &xMax, &yMax);

    o.scale(1, -1);
    o.translate(0, -state->getPageHeight());

    state->transform(x, y, &tx, &ty);
    tx = tx < xMin ? xMin : tx > xMax ? xMax : tx;
    ty = ty < yMin ? yMin : ty > yMax ? yMax : ty;
    o.transform(tx, ty, &x, &y);

    if (!hasGraphics || bbA->x1 > x)
        bbA->x1 = x;
    if (!hasGraphics || bbA->y1 > y)
        bbA->y1 = y;
    if (!hasGraphics || bbA->x2 < x)
        bbA->x2 = x;
    if (!hasGraphics || bbA->y2 < y)
        bbA->y2 = y;
    hasGraphics = true;
}

/* update the bounding box with a new path */
void BBoxOutputDev::updatePath(PDFRectangle *bbA, const GfxPath *path, const GfxState *state)
{
    int i, j;
    const GfxSubpath *subpath;
    double x, y;
    double w;
    if (!vector)
        return;
    w = lwidth ? state->getLineWidth() : 0;
    for (i = 0; i < path->getNumSubpaths(); i++) {
        subpath = path->getSubpath(i);
        for (j = 0; j < subpath->getNumPoints(); j++) {
            x = subpath->getX(j);
            y = subpath->getY(j);
            updatePoint(bbA, x - w / 2, y - w / 2, state);
            updatePoint(bbA, x + w / 2, y + w / 2, state);
        }
    }
}

/* update the bounding box with a new image */
void BBoxOutputDev::updateImage(PDFRectangle *bbA, const GfxState *state)
{
    if (!raster)
        return;
    updatePoint(bbA, 0, 1, state);
    updatePoint(bbA, 1, 0, state);
}
