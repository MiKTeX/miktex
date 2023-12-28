//========================================================================
//
// Splash.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005-2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Marco Pesenti Gritti <mpg@redhat.com>
// Copyright (C) 2010-2016 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011-2013, 2015 William Bader <williambader@hotmail.com>
// Copyright (C) 2012 Markus Trippelsdorf <markus@trippelsdorf.de>
// Copyright (C) 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Matthias Kramm <kramm@quiss.org>
// Copyright (C) 2018, 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2019 Marek Kasik <mkasik@redhat.com>
// Copyright (C) 2020 Tobias Deiminger <haxtibal@posteo.de>
// Copyright (C) 2021 Even Rouault <even.rouault@spatialys.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdlib>
#include <cstring>
#include <climits>
#include <cassert>
#include <cmath>
#include "goo/gmem.h"
#include "goo/GooLikely.h"
#include "poppler/GfxState.h"
#include "poppler/Error.h"
#include "SplashErrorCodes.h"
#include "SplashMath.h"
#include "SplashBitmap.h"
#include "SplashState.h"
#include "SplashPath.h"
#include "SplashXPath.h"
#include "SplashXPathScanner.h"
#include "SplashPattern.h"
#include "SplashScreen.h"
#include "SplashFont.h"
#include "SplashGlyphBitmap.h"
#include "Splash.h"
#include <algorithm>

// the MSVC math.h doesn't define this
#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------

#define splashAAGamma 1.5

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle ((SplashCoord)0.55228475)
#define bezierCircle2 ((SplashCoord)(0.5 * 0.55228475))

// Divide a 16-bit value (in [0, 255*255]) by 255, returning an 8-bit result.
static inline unsigned char div255(int x)
{
    return (unsigned char)((x + (x >> 8) + 0x80) >> 8);
}

// Clip x to lie in [0, 255].
static inline unsigned char clip255(int x)
{
    return x < 0 ? 0 : x > 255 ? 255 : x;
}

template<typename T>
inline void Guswap(T &a, T &b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

// The PDF spec says that all pixels whose *centers* lie within the
// image target region get painted, so we want to round n+0.5 down to
// n.  But this causes problems, e.g., with PDF files that fill a
// rectangle with black and then draw an image to the exact same
// rectangle, so we instead use the fill scan conversion rule.
// However, the correct rule works better for glyphs, so we also
// provide that option in fillImageMask.
#if 0
static inline int imgCoordMungeLower(SplashCoord x) {
  return splashCeil(x + 0.5) - 1;
}
static inline int imgCoordMungeUpper(SplashCoord x) {
  return splashCeil(x + 0.5) - 1;
}
#else
static inline int imgCoordMungeLower(SplashCoord x)
{
    return splashFloor(x);
}
static inline int imgCoordMungeUpper(SplashCoord x)
{
    return splashFloor(x) + 1;
}
static inline int imgCoordMungeLowerC(SplashCoord x, bool glyphMode)
{
    return glyphMode ? (splashCeil(x + 0.5) - 1) : splashFloor(x);
}
static inline int imgCoordMungeUpperC(SplashCoord x, bool glyphMode)
{
    return glyphMode ? (splashCeil(x + 0.5) - 1) : (splashFloor(x) + 1);
}
#endif

// Used by drawImage and fillImageMask to divide the target
// quadrilateral into sections.
struct ImageSection
{
    int y0, y1; // actual y range
    int ia0, ia1; // vertex indices for edge A
    int ib0, ib1; // vertex indices for edge A
    SplashCoord xa0, ya0, xa1, ya1; // edge A
    SplashCoord dxdya; // slope of edge A
    SplashCoord xb0, yb0, xb1, yb1; // edge B
    SplashCoord dxdyb; // slope of edge B
};

//------------------------------------------------------------------------
// SplashPipe
//------------------------------------------------------------------------

#define splashPipeMaxStages 9

struct SplashPipe
{
    // pixel coordinates
    int x, y;

    // source pattern
    SplashPattern *pattern;

    // source alpha and color
    unsigned char aInput;
    bool usesShape;
    SplashColorPtr cSrc;
    SplashColor cSrcVal = {};

    // non-isolated group alpha0
    unsigned char *alpha0Ptr;

    // knockout groups
    bool knockout;
    unsigned char knockoutOpacity;

    // soft mask
    SplashColorPtr softMaskPtr;

    // destination alpha and color
    SplashColorPtr destColorPtr;
    int destColorMask;
    unsigned char *destAlphaPtr;

    // shape
    unsigned char shape;

    // result alpha and color
    bool noTransparency;
    SplashPipeResultColorCtrl resultColorCtrl;

    // non-isolated group correction
    bool nonIsolatedGroup;

    // the "run" function
    void (Splash::*run)(SplashPipe *pipe);
};

SplashPipeResultColorCtrl Splash::pipeResultColorNoAlphaBlend[] = { splashPipeResultColorNoAlphaBlendMono, splashPipeResultColorNoAlphaBlendMono, splashPipeResultColorNoAlphaBlendRGB,    splashPipeResultColorNoAlphaBlendRGB,
                                                                    splashPipeResultColorNoAlphaBlendRGB,  splashPipeResultColorNoAlphaBlendCMYK, splashPipeResultColorNoAlphaBlendDeviceN };

SplashPipeResultColorCtrl Splash::pipeResultColorAlphaNoBlend[] = { splashPipeResultColorAlphaNoBlendMono, splashPipeResultColorAlphaNoBlendMono, splashPipeResultColorAlphaNoBlendRGB,    splashPipeResultColorAlphaNoBlendRGB,
                                                                    splashPipeResultColorAlphaNoBlendRGB,  splashPipeResultColorAlphaNoBlendCMYK, splashPipeResultColorAlphaNoBlendDeviceN };

SplashPipeResultColorCtrl Splash::pipeResultColorAlphaBlend[] = { splashPipeResultColorAlphaBlendMono, splashPipeResultColorAlphaBlendMono, splashPipeResultColorAlphaBlendRGB,    splashPipeResultColorAlphaBlendRGB,
                                                                  splashPipeResultColorAlphaBlendRGB,  splashPipeResultColorAlphaBlendCMYK, splashPipeResultColorAlphaBlendDeviceN };

//------------------------------------------------------------------------

static void blendXor(SplashColorPtr src, SplashColorPtr dest, SplashColorPtr blend, SplashColorMode cm)
{
    int i;

    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
        blend[i] = src[i] ^ dest[i];
    }
}

//------------------------------------------------------------------------
// pipeline
//------------------------------------------------------------------------

inline void Splash::pipeInit(SplashPipe *pipe, int x, int y, SplashPattern *pattern, SplashColorPtr cSrc, unsigned char aInput, bool usesShape, bool nonIsolatedGroup, bool knockout, unsigned char knockoutOpacity)
{
    pipeSetXY(pipe, x, y);
    pipe->pattern = nullptr;

    // source color
    if (pattern) {
        if (pattern->isStatic()) {
            pattern->getColor(x, y, pipe->cSrcVal);
        } else {
            pipe->pattern = pattern;
        }
        pipe->cSrc = pipe->cSrcVal;
    } else {
        pipe->cSrc = cSrc;
    }

    // source alpha
    pipe->aInput = aInput;
    pipe->usesShape = usesShape;
    pipe->shape = 0;

    // knockout
    pipe->knockout = knockout;
    pipe->knockoutOpacity = knockoutOpacity;

    // result alpha
    if (aInput == 255 && !state->softMask && !usesShape && !state->inNonIsolatedGroup && !nonIsolatedGroup) {
        pipe->noTransparency = true;
    } else {
        pipe->noTransparency = false;
    }

    // result color
    if (pipe->noTransparency) {
        // the !state->blendFunc case is handled separately in pipeRun
        pipe->resultColorCtrl = pipeResultColorNoAlphaBlend[bitmap->mode];
    } else if (!state->blendFunc) {
        pipe->resultColorCtrl = pipeResultColorAlphaNoBlend[bitmap->mode];
    } else {
        pipe->resultColorCtrl = pipeResultColorAlphaBlend[bitmap->mode];
    }

    // non-isolated group correction
    pipe->nonIsolatedGroup = nonIsolatedGroup;

    // select the 'run' function
    pipe->run = &Splash::pipeRun;
    if (!pipe->pattern && pipe->noTransparency && !state->blendFunc) {
        if (bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunSimpleMono1;
        } else if (bitmap->mode == splashModeMono8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunSimpleMono8;
        } else if (bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunSimpleRGB8;
        } else if (bitmap->mode == splashModeXBGR8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunSimpleXBGR8;
        } else if (bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunSimpleBGR8;
        } else if (bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunSimpleCMYK8;
        } else if (bitmap->mode == splashModeDeviceN8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunSimpleDeviceN8;
        }
    } else if (!pipe->pattern && !pipe->noTransparency && !state->softMask && pipe->usesShape && !(state->inNonIsolatedGroup && alpha0Bitmap->alpha) && !state->blendFunc && !pipe->nonIsolatedGroup) {
        if (bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunAAMono1;
        } else if (bitmap->mode == splashModeMono8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunAAMono8;
        } else if (bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunAARGB8;
        } else if (bitmap->mode == splashModeXBGR8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunAAXBGR8;
        } else if (bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunAABGR8;
        } else if (bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunAACMYK8;
        } else if (bitmap->mode == splashModeDeviceN8 && pipe->destAlphaPtr) {
            pipe->run = &Splash::pipeRunAADeviceN8;
        }
    }
}

// general case
void Splash::pipeRun(SplashPipe *pipe)
{
    unsigned char aSrc, aDest, alphaI, alphaIm1, alpha0, aResult;
    SplashColor cSrcNonIso, cDest, cBlend;
    SplashColorPtr cSrc;
    unsigned char cResult0, cResult1, cResult2, cResult3;
    int t;
    int cp, mask;
    unsigned char cResult[SPOT_NCOMPS + 4];

    //----- source color

    // static pattern: handled in pipeInit
    // fixed color: handled in pipeInit

    // dynamic pattern
    if (pipe->pattern) {
        if (!pipe->pattern->getColor(pipe->x, pipe->y, pipe->cSrcVal)) {
            pipeIncX(pipe);
            return;
        }
        if (bitmap->mode == splashModeCMYK8 || bitmap->mode == splashModeDeviceN8) {
            if (state->fillOverprint && state->overprintMode && pipe->pattern->isCMYK()) {
                unsigned int overprintMask = 15;
                if (pipe->cSrcVal[0] == 0) {
                    overprintMask &= ~1;
                }
                if (pipe->cSrcVal[1] == 0) {
                    overprintMask &= ~2;
                }
                if (pipe->cSrcVal[2] == 0) {
                    overprintMask &= ~4;
                }
                if (pipe->cSrcVal[3] == 0) {
                    overprintMask &= ~8;
                }
                state->overprintMask = overprintMask;
            }
        }
    }

    if (pipe->noTransparency && !state->blendFunc) {

        //----- write destination pixel

        switch (bitmap->mode) {
        case splashModeMono1:
            cResult0 = state->grayTransfer[pipe->cSrc[0]];
            if (state->screen->test(pipe->x, pipe->y, cResult0)) {
                *pipe->destColorPtr |= pipe->destColorMask;
            } else {
                *pipe->destColorPtr &= ~pipe->destColorMask;
            }
            if (!(pipe->destColorMask >>= 1)) {
                pipe->destColorMask = 0x80;
                ++pipe->destColorPtr;
            }
            break;
        case splashModeMono8:
            *pipe->destColorPtr++ = state->grayTransfer[pipe->cSrc[0]];
            break;
        case splashModeRGB8:
            *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
            *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
            *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
            break;
        case splashModeXBGR8:
            *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
            *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
            *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
            *pipe->destColorPtr++ = 255;
            break;
        case splashModeBGR8:
            *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
            *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
            *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
            break;
        case splashModeCMYK8:
            if (state->overprintMask & 1) {
                pipe->destColorPtr[0] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[0] + state->cmykTransferC[pipe->cSrc[0]], 255) : state->cmykTransferC[pipe->cSrc[0]];
            }
            if (state->overprintMask & 2) {
                pipe->destColorPtr[1] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[1] + state->cmykTransferM[pipe->cSrc[1]], 255) : state->cmykTransferM[pipe->cSrc[1]];
            }
            if (state->overprintMask & 4) {
                pipe->destColorPtr[2] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[2] + state->cmykTransferY[pipe->cSrc[2]], 255) : state->cmykTransferY[pipe->cSrc[2]];
            }
            if (state->overprintMask & 8) {
                pipe->destColorPtr[3] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[3] + state->cmykTransferK[pipe->cSrc[3]], 255) : state->cmykTransferK[pipe->cSrc[3]];
            }
            pipe->destColorPtr += 4;
            break;
        case splashModeDeviceN8:
            mask = 1;
            for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                if (state->overprintMask & mask) {
                    pipe->destColorPtr[cp] = state->deviceNTransfer[cp][pipe->cSrc[cp]];
                }
                mask <<= 1;
            }
            pipe->destColorPtr += (SPOT_NCOMPS + 4);
            break;
        }
        if (pipe->destAlphaPtr) {
            *pipe->destAlphaPtr++ = 255;
        }

    } else {

        //----- read destination pixel

        unsigned char *destColorPtr;
        if (pipe->shape && state->blendFunc && pipe->knockout && alpha0Bitmap != nullptr) {
            destColorPtr = alpha0Bitmap->data + (alpha0Y + pipe->y) * alpha0Bitmap->rowSize;
            switch (bitmap->mode) {
            case splashModeMono1:
                destColorPtr += (alpha0X + pipe->x) / 8;
                break;
            case splashModeMono8:
                destColorPtr += (alpha0X + pipe->x);
                break;
            case splashModeRGB8:
            case splashModeBGR8:
                destColorPtr += (alpha0X + pipe->x) * 3;
                break;
            case splashModeXBGR8:
            case splashModeCMYK8:
                destColorPtr += (alpha0X + pipe->x) * 4;
                break;
            case splashModeDeviceN8:
                destColorPtr += (alpha0X + pipe->x) * (SPOT_NCOMPS + 4);
                break;
            }
        } else {
            destColorPtr = pipe->destColorPtr;
        }
        switch (bitmap->mode) {
        case splashModeMono1:
            cDest[0] = (*destColorPtr & pipe->destColorMask) ? 0xff : 0x00;
            break;
        case splashModeMono8:
            cDest[0] = *destColorPtr;
            break;
        case splashModeRGB8:
            cDest[0] = destColorPtr[0];
            cDest[1] = destColorPtr[1];
            cDest[2] = destColorPtr[2];
            break;
        case splashModeXBGR8:
            cDest[0] = destColorPtr[2];
            cDest[1] = destColorPtr[1];
            cDest[2] = destColorPtr[0];
            cDest[3] = 255;
            break;
        case splashModeBGR8:
            cDest[0] = destColorPtr[2];
            cDest[1] = destColorPtr[1];
            cDest[2] = destColorPtr[0];
            break;
        case splashModeCMYK8:
            cDest[0] = destColorPtr[0];
            cDest[1] = destColorPtr[1];
            cDest[2] = destColorPtr[2];
            cDest[3] = destColorPtr[3];
            break;
        case splashModeDeviceN8:
            for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                cDest[cp] = destColorPtr[cp];
            }
            break;
        }
        if (pipe->destAlphaPtr) {
            aDest = *pipe->destAlphaPtr;
        } else {
            aDest = 0xff;
        }

        //----- source alpha

        if (state->softMask) {
            if (pipe->usesShape) {
                aSrc = div255(div255(pipe->aInput * *pipe->softMaskPtr++) * pipe->shape);
            } else {
                aSrc = div255(pipe->aInput * *pipe->softMaskPtr++);
            }
        } else if (pipe->usesShape) {
            aSrc = div255(pipe->aInput * pipe->shape);
        } else {
            aSrc = pipe->aInput;
        }

        //----- non-isolated group correction

        if (pipe->nonIsolatedGroup) {
            // This path is only used when Splash::composite() is called to
            // composite a non-isolated group onto the backdrop.  In this
            // case, pipe->shape is the source (group) alpha.
            if (pipe->shape == 0) {
                // this value will be multiplied by zero later, so it doesn't
                // matter what we use
                cSrc = pipe->cSrc;
            } else {
                t = (aDest * 255) / pipe->shape - aDest;
                switch (bitmap->mode) {
                case splashModeDeviceN8:
                    for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                        cSrcNonIso[cp] = clip255(pipe->cSrc[cp] + ((pipe->cSrc[cp] - cDest[cp]) * t) / 255);
                    }
                    break;
                case splashModeCMYK8:
                    for (cp = 0; cp < 4; cp++) {
                        cSrcNonIso[cp] = clip255(pipe->cSrc[cp] + ((pipe->cSrc[cp] - cDest[cp]) * t) / 255);
                    }
                    break;
                case splashModeXBGR8:
                    cSrcNonIso[3] = 255;
                    // fallthrough
                case splashModeRGB8:
                case splashModeBGR8:
                    cSrcNonIso[2] = clip255(pipe->cSrc[2] + ((pipe->cSrc[2] - cDest[2]) * t) / 255);
                    cSrcNonIso[1] = clip255(pipe->cSrc[1] + ((pipe->cSrc[1] - cDest[1]) * t) / 255);
                    // fallthrough
                case splashModeMono1:
                case splashModeMono8:
                    cSrcNonIso[0] = clip255(pipe->cSrc[0] + ((pipe->cSrc[0] - cDest[0]) * t) / 255);
                    break;
                }
                cSrc = cSrcNonIso;
                // knockout: remove backdrop color
                if (pipe->knockout && pipe->shape >= pipe->knockoutOpacity) {
                    aDest = 0;
                }
            }
        } else {
            cSrc = pipe->cSrc;
        }

        //----- blend function

        if (state->blendFunc) {
            if (bitmap->mode == splashModeDeviceN8) {
                for (int k = 4; k < 4 + SPOT_NCOMPS; k++) {
                    cBlend[k] = 0;
                }
            }
            (*state->blendFunc)(cSrc, cDest, cBlend, bitmap->mode);
        }

        //----- result alpha and non-isolated group element correction

        if (pipe->noTransparency) {
            alphaI = alphaIm1 = aResult = 255;
        } else {
            aResult = aSrc + aDest - div255(aSrc * aDest);

            // alphaI = alpha_i
            // alphaIm1 = alpha_(i-1)
            if (pipe->alpha0Ptr) {
                alpha0 = *pipe->alpha0Ptr++;
                alphaI = aResult + alpha0 - div255(aResult * alpha0);
                alphaIm1 = alpha0 + aDest - div255(alpha0 * aDest);
            } else {
                alphaI = aResult;
                alphaIm1 = aDest;
            }
        }

        //----- result color

        cResult0 = cResult1 = cResult2 = cResult3 = 0; // make gcc happy

        switch (pipe->resultColorCtrl) {

        case splashPipeResultColorNoAlphaBlendMono:
            cResult0 = state->grayTransfer[div255((255 - aDest) * cSrc[0] + aDest * cBlend[0])];
            break;
        case splashPipeResultColorNoAlphaBlendRGB:
            cResult0 = state->rgbTransferR[div255((255 - aDest) * cSrc[0] + aDest * cBlend[0])];
            cResult1 = state->rgbTransferG[div255((255 - aDest) * cSrc[1] + aDest * cBlend[1])];
            cResult2 = state->rgbTransferB[div255((255 - aDest) * cSrc[2] + aDest * cBlend[2])];
            break;
        case splashPipeResultColorNoAlphaBlendCMYK:
            cResult0 = state->cmykTransferC[div255((255 - aDest) * cSrc[0] + aDest * cBlend[0])];
            cResult1 = state->cmykTransferM[div255((255 - aDest) * cSrc[1] + aDest * cBlend[1])];
            cResult2 = state->cmykTransferY[div255((255 - aDest) * cSrc[2] + aDest * cBlend[2])];
            cResult3 = state->cmykTransferK[div255((255 - aDest) * cSrc[3] + aDest * cBlend[3])];
            break;
        case splashPipeResultColorNoAlphaBlendDeviceN:
            for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                cResult[cp] = state->deviceNTransfer[cp][div255((255 - aDest) * cSrc[cp] + aDest * cBlend[cp])];
            }
            break;

        case splashPipeResultColorAlphaNoBlendMono:
            if (alphaI == 0) {
                cResult0 = 0;
            } else {
                cResult0 = state->grayTransfer[((alphaI - aSrc) * cDest[0] + aSrc * cSrc[0]) / alphaI];
            }
            break;
        case splashPipeResultColorAlphaNoBlendRGB:
            if (alphaI == 0) {
                cResult0 = 0;
                cResult1 = 0;
                cResult2 = 0;
            } else {
                cResult0 = state->rgbTransferR[((alphaI - aSrc) * cDest[0] + aSrc * cSrc[0]) / alphaI];
                cResult1 = state->rgbTransferG[((alphaI - aSrc) * cDest[1] + aSrc * cSrc[1]) / alphaI];
                cResult2 = state->rgbTransferB[((alphaI - aSrc) * cDest[2] + aSrc * cSrc[2]) / alphaI];
            }
            break;
        case splashPipeResultColorAlphaNoBlendCMYK:
            if (alphaI == 0) {
                cResult0 = 0;
                cResult1 = 0;
                cResult2 = 0;
                cResult3 = 0;
            } else {
                cResult0 = state->cmykTransferC[((alphaI - aSrc) * cDest[0] + aSrc * cSrc[0]) / alphaI];
                cResult1 = state->cmykTransferM[((alphaI - aSrc) * cDest[1] + aSrc * cSrc[1]) / alphaI];
                cResult2 = state->cmykTransferY[((alphaI - aSrc) * cDest[2] + aSrc * cSrc[2]) / alphaI];
                cResult3 = state->cmykTransferK[((alphaI - aSrc) * cDest[3] + aSrc * cSrc[3]) / alphaI];
            }
            break;
        case splashPipeResultColorAlphaNoBlendDeviceN:
            if (alphaI == 0) {
                for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    cResult[cp] = 0;
                }
            } else {
                for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    cResult[cp] = state->deviceNTransfer[cp][((alphaI - aSrc) * cDest[cp] + aSrc * cSrc[cp]) / alphaI];
                }
            }
            break;

        case splashPipeResultColorAlphaBlendMono:
            if (alphaI == 0) {
                cResult0 = 0;
            } else {
                cResult0 = state->grayTransfer[((alphaI - aSrc) * cDest[0] + aSrc * ((255 - alphaIm1) * cSrc[0] + alphaIm1 * cBlend[0]) / 255) / alphaI];
            }
            break;
        case splashPipeResultColorAlphaBlendRGB:
            if (alphaI == 0) {
                cResult0 = 0;
                cResult1 = 0;
                cResult2 = 0;
            } else {
                cResult0 = state->rgbTransferR[((alphaI - aSrc) * cDest[0] + aSrc * ((255 - alphaIm1) * cSrc[0] + alphaIm1 * cBlend[0]) / 255) / alphaI];
                cResult1 = state->rgbTransferG[((alphaI - aSrc) * cDest[1] + aSrc * ((255 - alphaIm1) * cSrc[1] + alphaIm1 * cBlend[1]) / 255) / alphaI];
                cResult2 = state->rgbTransferB[((alphaI - aSrc) * cDest[2] + aSrc * ((255 - alphaIm1) * cSrc[2] + alphaIm1 * cBlend[2]) / 255) / alphaI];
            }
            break;
        case splashPipeResultColorAlphaBlendCMYK:
            if (alphaI == 0) {
                cResult0 = 0;
                cResult1 = 0;
                cResult2 = 0;
                cResult3 = 0;
            } else {
                cResult0 = state->cmykTransferC[((alphaI - aSrc) * cDest[0] + aSrc * ((255 - alphaIm1) * cSrc[0] + alphaIm1 * cBlend[0]) / 255) / alphaI];
                cResult1 = state->cmykTransferM[((alphaI - aSrc) * cDest[1] + aSrc * ((255 - alphaIm1) * cSrc[1] + alphaIm1 * cBlend[1]) / 255) / alphaI];
                cResult2 = state->cmykTransferY[((alphaI - aSrc) * cDest[2] + aSrc * ((255 - alphaIm1) * cSrc[2] + alphaIm1 * cBlend[2]) / 255) / alphaI];
                cResult3 = state->cmykTransferK[((alphaI - aSrc) * cDest[3] + aSrc * ((255 - alphaIm1) * cSrc[3] + alphaIm1 * cBlend[3]) / 255) / alphaI];
            }
            break;
        case splashPipeResultColorAlphaBlendDeviceN:
            if (alphaI == 0) {
                for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    cResult[cp] = 0;
                }
            } else {
                for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    cResult[cp] = state->deviceNTransfer[cp][((alphaI - aSrc) * cDest[cp] + aSrc * ((255 - alphaIm1) * cSrc[cp] + alphaIm1 * cBlend[cp]) / 255) / alphaI];
                }
            }
            break;
        }

        //----- write destination pixel

        switch (bitmap->mode) {
        case splashModeMono1:
            if (state->screen->test(pipe->x, pipe->y, cResult0)) {
                *pipe->destColorPtr |= pipe->destColorMask;
            } else {
                *pipe->destColorPtr &= ~pipe->destColorMask;
            }
            if (!(pipe->destColorMask >>= 1)) {
                pipe->destColorMask = 0x80;
                ++pipe->destColorPtr;
            }
            break;
        case splashModeMono8:
            *pipe->destColorPtr++ = cResult0;
            break;
        case splashModeRGB8:
            *pipe->destColorPtr++ = cResult0;
            *pipe->destColorPtr++ = cResult1;
            *pipe->destColorPtr++ = cResult2;
            break;
        case splashModeXBGR8:
            *pipe->destColorPtr++ = cResult2;
            *pipe->destColorPtr++ = cResult1;
            *pipe->destColorPtr++ = cResult0;
            *pipe->destColorPtr++ = 255;
            break;
        case splashModeBGR8:
            *pipe->destColorPtr++ = cResult2;
            *pipe->destColorPtr++ = cResult1;
            *pipe->destColorPtr++ = cResult0;
            break;
        case splashModeCMYK8:
            if (state->overprintMask & 1) {
                pipe->destColorPtr[0] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[0] + cResult0, 255) : cResult0;
            }
            if (state->overprintMask & 2) {
                pipe->destColorPtr[1] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[1] + cResult1, 255) : cResult1;
            }
            if (state->overprintMask & 4) {
                pipe->destColorPtr[2] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[2] + cResult2, 255) : cResult2;
            }
            if (state->overprintMask & 8) {
                pipe->destColorPtr[3] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[3] + cResult3, 255) : cResult3;
            }
            pipe->destColorPtr += 4;
            break;
        case splashModeDeviceN8:
            mask = 1;
            for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                if (state->overprintMask & mask) {
                    pipe->destColorPtr[cp] = cResult[cp];
                }
                mask <<= 1;
            }
            pipe->destColorPtr += (SPOT_NCOMPS + 4);
            break;
        }
        if (pipe->destAlphaPtr) {
            *pipe->destAlphaPtr++ = aResult;
        }
    }

    ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr) {
void Splash::pipeRunSimpleMono1(SplashPipe *pipe)
{
    unsigned char cResult0;

    //----- write destination pixel
    cResult0 = state->grayTransfer[pipe->cSrc[0]];
    if (state->screen->test(pipe->x, pipe->y, cResult0)) {
        *pipe->destColorPtr |= pipe->destColorMask;
    } else {
        *pipe->destColorPtr &= ~pipe->destColorMask;
    }
    if (!(pipe->destColorMask >>= 1)) {
        pipe->destColorMask = 0x80;
        ++pipe->destColorPtr;
    }

    ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeMono8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleMono8(SplashPipe *pipe)
{
    //----- write destination pixel
    *pipe->destColorPtr++ = state->grayTransfer[pipe->cSrc[0]];
    *pipe->destAlphaPtr++ = 255;

    ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleRGB8(SplashPipe *pipe)
{
    //----- write destination pixel
    *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
    *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
    *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
    *pipe->destAlphaPtr++ = 255;

    ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeXBGR8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleXBGR8(SplashPipe *pipe)
{
    //----- write destination pixel
    *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
    *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
    *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
    *pipe->destColorPtr++ = 255;
    *pipe->destAlphaPtr++ = 255;

    ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleBGR8(SplashPipe *pipe)
{
    //----- write destination pixel
    *pipe->destColorPtr++ = state->rgbTransferB[pipe->cSrc[2]];
    *pipe->destColorPtr++ = state->rgbTransferG[pipe->cSrc[1]];
    *pipe->destColorPtr++ = state->rgbTransferR[pipe->cSrc[0]];
    *pipe->destAlphaPtr++ = 255;

    ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleCMYK8(SplashPipe *pipe)
{
    //----- write destination pixel
    if (state->overprintMask & 1) {
        pipe->destColorPtr[0] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[0] + state->cmykTransferC[pipe->cSrc[0]], 255) : state->cmykTransferC[pipe->cSrc[0]];
    }
    if (state->overprintMask & 2) {
        pipe->destColorPtr[1] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[1] + state->cmykTransferM[pipe->cSrc[1]], 255) : state->cmykTransferM[pipe->cSrc[1]];
    }
    if (state->overprintMask & 4) {
        pipe->destColorPtr[2] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[2] + state->cmykTransferY[pipe->cSrc[2]], 255) : state->cmykTransferY[pipe->cSrc[2]];
    }
    if (state->overprintMask & 8) {
        pipe->destColorPtr[3] = (state->overprintAdditive) ? std::min<int>(pipe->destColorPtr[3] + state->cmykTransferK[pipe->cSrc[3]], 255) : state->cmykTransferK[pipe->cSrc[3]];
    }
    pipe->destColorPtr += 4;
    *pipe->destAlphaPtr++ = 255;

    ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeDeviceN8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleDeviceN8(SplashPipe *pipe)
{
    //----- write destination pixel
    int mask = 1;
    for (int cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
        if (state->overprintMask & mask) {
            pipe->destColorPtr[cp] = state->deviceNTransfer[cp][pipe->cSrc[cp]];
        }
        mask <<= 1;
    }
    pipe->destColorPtr += (SPOT_NCOMPS + 4);
    *pipe->destAlphaPtr++ = 255;

    ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr
void Splash::pipeRunAAMono1(SplashPipe *pipe)
{
    unsigned char aSrc;
    SplashColor cDest;
    unsigned char cResult0;

    //----- read destination pixel
    cDest[0] = (*pipe->destColorPtr & pipe->destColorMask) ? 0xff : 0x00;

    //----- source alpha
    aSrc = div255(pipe->aInput * pipe->shape);

    //----- result color
    // note: aDest = alpha2 = aResult = 0xff
    cResult0 = state->grayTransfer[(unsigned char)div255((0xff - aSrc) * cDest[0] + aSrc * pipe->cSrc[0])];

    //----- write destination pixel
    if (state->screen->test(pipe->x, pipe->y, cResult0)) {
        *pipe->destColorPtr |= pipe->destColorMask;
    } else {
        *pipe->destColorPtr &= ~pipe->destColorMask;
    }
    if (!(pipe->destColorMask >>= 1)) {
        pipe->destColorMask = 0x80;
        ++pipe->destColorPtr;
    }

    ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeMono8 && pipe->destAlphaPtr
void Splash::pipeRunAAMono8(SplashPipe *pipe)
{
    unsigned char aSrc, aDest, alpha2, aResult;
    SplashColor cDest;
    unsigned char cResult0;

    //----- read destination pixel
    cDest[0] = *pipe->destColorPtr;
    aDest = *pipe->destAlphaPtr;

    //----- source alpha
    aSrc = div255(pipe->aInput * pipe->shape);

    //----- result alpha and non-isolated group element correction
    aResult = aSrc + aDest - div255(aSrc * aDest);
    alpha2 = aResult;

    //----- result color
    if (alpha2 == 0) {
        cResult0 = 0;
    } else {
        cResult0 = state->grayTransfer[(unsigned char)(((alpha2 - aSrc) * cDest[0] + aSrc * pipe->cSrc[0]) / alpha2)];
    }

    //----- write destination pixel
    *pipe->destColorPtr++ = cResult0;
    *pipe->destAlphaPtr++ = aResult;

    ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr
void Splash::pipeRunAARGB8(SplashPipe *pipe)
{
    unsigned char aSrc, aDest, alpha2, aResult;
    SplashColor cDest;
    unsigned char cResult0, cResult1, cResult2;

    //----- read destination alpha
    aDest = *pipe->destAlphaPtr;

    //----- source alpha
    aSrc = div255(pipe->aInput * pipe->shape);

    //----- result color
    if (aSrc == 255) {
        cResult0 = state->rgbTransferR[pipe->cSrc[0]];
        cResult1 = state->rgbTransferG[pipe->cSrc[1]];
        cResult2 = state->rgbTransferB[pipe->cSrc[2]];
        aResult = 255;

    } else if (aSrc == 0 && aDest == 0) {
        cResult0 = 0;
        cResult1 = 0;
        cResult2 = 0;
        aResult = 0;

    } else {
        //----- read destination pixel
        cDest[0] = pipe->destColorPtr[0];
        cDest[1] = pipe->destColorPtr[1];
        cDest[2] = pipe->destColorPtr[2];

        //----- result alpha and non-isolated group element correction
        aResult = aSrc + aDest - div255(aSrc * aDest);
        alpha2 = aResult;

        cResult0 = state->rgbTransferR[(unsigned char)(((alpha2 - aSrc) * cDest[0] + aSrc * pipe->cSrc[0]) / alpha2)];
        cResult1 = state->rgbTransferG[(unsigned char)(((alpha2 - aSrc) * cDest[1] + aSrc * pipe->cSrc[1]) / alpha2)];
        cResult2 = state->rgbTransferB[(unsigned char)(((alpha2 - aSrc) * cDest[2] + aSrc * pipe->cSrc[2]) / alpha2)];
    }

    //----- write destination pixel
    *pipe->destColorPtr++ = cResult0;
    *pipe->destColorPtr++ = cResult1;
    *pipe->destColorPtr++ = cResult2;
    *pipe->destAlphaPtr++ = aResult;

    ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeXBGR8 && pipe->destAlphaPtr
void Splash::pipeRunAAXBGR8(SplashPipe *pipe)
{
    unsigned char aSrc, aDest, alpha2, aResult;
    SplashColor cDest;
    unsigned char cResult0, cResult1, cResult2;

    //----- read destination alpha
    aDest = *pipe->destAlphaPtr;

    //----- source alpha
    aSrc = div255(pipe->aInput * pipe->shape);

    //----- result color
    if (aSrc == 255) {
        cResult0 = state->rgbTransferR[pipe->cSrc[0]];
        cResult1 = state->rgbTransferG[pipe->cSrc[1]];
        cResult2 = state->rgbTransferB[pipe->cSrc[2]];
        aResult = 255;

    } else if (aSrc == 0 && aDest == 0) {
        cResult0 = 0;
        cResult1 = 0;
        cResult2 = 0;
        aResult = 0;

    } else {
        //----- read destination color
        cDest[0] = pipe->destColorPtr[2];
        cDest[1] = pipe->destColorPtr[1];
        cDest[2] = pipe->destColorPtr[0];

        //----- result alpha and non-isolated group element correction
        aResult = aSrc + aDest - div255(aSrc * aDest);
        alpha2 = aResult;

        cResult0 = state->rgbTransferR[(unsigned char)(((alpha2 - aSrc) * cDest[0] + aSrc * pipe->cSrc[0]) / alpha2)];
        cResult1 = state->rgbTransferG[(unsigned char)(((alpha2 - aSrc) * cDest[1] + aSrc * pipe->cSrc[1]) / alpha2)];
        cResult2 = state->rgbTransferB[(unsigned char)(((alpha2 - aSrc) * cDest[2] + aSrc * pipe->cSrc[2]) / alpha2)];
    }

    //----- write destination pixel
    *pipe->destColorPtr++ = cResult2;
    *pipe->destColorPtr++ = cResult1;
    *pipe->destColorPtr++ = cResult0;
    *pipe->destColorPtr++ = 255;
    *pipe->destAlphaPtr++ = aResult;

    ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr
void Splash::pipeRunAABGR8(SplashPipe *pipe)
{
    unsigned char aSrc, aDest, alpha2, aResult;
    SplashColor cDest;
    unsigned char cResult0, cResult1, cResult2;

    //----- read destination alpha
    aDest = *pipe->destAlphaPtr;

    //----- source alpha
    aSrc = div255(pipe->aInput * pipe->shape);

    //----- result color
    if (aSrc == 255) {
        cResult0 = state->rgbTransferR[pipe->cSrc[0]];
        cResult1 = state->rgbTransferG[pipe->cSrc[1]];
        cResult2 = state->rgbTransferB[pipe->cSrc[2]];
        aResult = 255;

    } else if (aSrc == 0 && aDest == 0) {
        cResult0 = 0;
        cResult1 = 0;
        cResult2 = 0;
        aResult = 0;

    } else {
        //----- read destination color
        cDest[0] = pipe->destColorPtr[2];
        cDest[1] = pipe->destColorPtr[1];
        cDest[2] = pipe->destColorPtr[0];

        //----- result alpha and non-isolated group element correction
        aResult = aSrc + aDest - div255(aSrc * aDest);
        alpha2 = aResult;

        cResult0 = state->rgbTransferR[(unsigned char)(((alpha2 - aSrc) * cDest[0] + aSrc * pipe->cSrc[0]) / alpha2)];
        cResult1 = state->rgbTransferG[(unsigned char)(((alpha2 - aSrc) * cDest[1] + aSrc * pipe->cSrc[1]) / alpha2)];
        cResult2 = state->rgbTransferB[(unsigned char)(((alpha2 - aSrc) * cDest[2] + aSrc * pipe->cSrc[2]) / alpha2)];
    }

    //----- write destination pixel
    *pipe->destColorPtr++ = cResult2;
    *pipe->destColorPtr++ = cResult1;
    *pipe->destColorPtr++ = cResult0;
    *pipe->destAlphaPtr++ = aResult;

    ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr
void Splash::pipeRunAACMYK8(SplashPipe *pipe)
{
    unsigned char aSrc, aDest, alpha2, aResult;
    SplashColor cDest;
    unsigned char cResult0, cResult1, cResult2, cResult3;

    //----- read destination pixel
    cDest[0] = pipe->destColorPtr[0];
    cDest[1] = pipe->destColorPtr[1];
    cDest[2] = pipe->destColorPtr[2];
    cDest[3] = pipe->destColorPtr[3];
    aDest = *pipe->destAlphaPtr;

    //----- source alpha
    aSrc = div255(pipe->aInput * pipe->shape);

    //----- result alpha and non-isolated group element correction
    aResult = aSrc + aDest - div255(aSrc * aDest);
    alpha2 = aResult;

    //----- result color
    if (alpha2 == 0) {
        cResult0 = 0;
        cResult1 = 0;
        cResult2 = 0;
        cResult3 = 0;
    } else {
        cResult0 = state->cmykTransferC[(unsigned char)(((alpha2 - aSrc) * cDest[0] + aSrc * pipe->cSrc[0]) / alpha2)];
        cResult1 = state->cmykTransferM[(unsigned char)(((alpha2 - aSrc) * cDest[1] + aSrc * pipe->cSrc[1]) / alpha2)];
        cResult2 = state->cmykTransferY[(unsigned char)(((alpha2 - aSrc) * cDest[2] + aSrc * pipe->cSrc[2]) / alpha2)];
        cResult3 = state->cmykTransferK[(unsigned char)(((alpha2 - aSrc) * cDest[3] + aSrc * pipe->cSrc[3]) / alpha2)];
    }

    //----- write destination pixel
    if (state->overprintMask & 1) {
        pipe->destColorPtr[0] = (state->overprintAdditive && pipe->shape != 0) ? std::min<int>(pipe->destColorPtr[0] + cResult0, 255) : cResult0;
    }
    if (state->overprintMask & 2) {
        pipe->destColorPtr[1] = (state->overprintAdditive && pipe->shape != 0) ? std::min<int>(pipe->destColorPtr[1] + cResult1, 255) : cResult1;
    }
    if (state->overprintMask & 4) {
        pipe->destColorPtr[2] = (state->overprintAdditive && pipe->shape != 0) ? std::min<int>(pipe->destColorPtr[2] + cResult2, 255) : cResult2;
    }
    if (state->overprintMask & 8) {
        pipe->destColorPtr[3] = (state->overprintAdditive && pipe->shape != 0) ? std::min<int>(pipe->destColorPtr[3] + cResult3, 255) : cResult3;
    }
    pipe->destColorPtr += 4;
    *pipe->destAlphaPtr++ = aResult;

    ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeDeviceN8 && pipe->destAlphaPtr
void Splash::pipeRunAADeviceN8(SplashPipe *pipe)
{
    unsigned char aSrc, aDest, alpha2, aResult;
    SplashColor cDest;
    unsigned char cResult[SPOT_NCOMPS + 4];
    int cp, mask;

    //----- read destination pixel
    for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
        cDest[cp] = pipe->destColorPtr[cp];
    }
    aDest = *pipe->destAlphaPtr;

    //----- source alpha
    aSrc = div255(pipe->aInput * pipe->shape);

    //----- result alpha and non-isolated group element correction
    aResult = aSrc + aDest - div255(aSrc * aDest);
    alpha2 = aResult;

    //----- result color
    if (alpha2 == 0) {
        for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
            cResult[cp] = 0;
        }
    } else {
        for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
            cResult[cp] = state->deviceNTransfer[cp][(unsigned char)(((alpha2 - aSrc) * cDest[cp] + aSrc * pipe->cSrc[cp]) / alpha2)];
        }
    }

    //----- write destination pixel
    mask = 1;
    for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
        if (state->overprintMask & mask) {
            pipe->destColorPtr[cp] = cResult[cp];
        }
        mask <<= 1;
    }
    pipe->destColorPtr += (SPOT_NCOMPS + 4);
    *pipe->destAlphaPtr++ = aResult;

    ++pipe->x;
}

inline void Splash::pipeSetXY(SplashPipe *pipe, int x, int y)
{
    pipe->x = x;
    pipe->y = y;
    if (state->softMask) {
        pipe->softMaskPtr = &state->softMask->data[y * state->softMask->rowSize + x];
    }
    switch (bitmap->mode) {
    case splashModeMono1:
        pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + (x >> 3)];
        pipe->destColorMask = 0x80 >> (x & 7);
        break;
    case splashModeMono8:
        pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + x];
        break;
    case splashModeRGB8:
    case splashModeBGR8:
        pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x];
        break;
    case splashModeXBGR8:
        pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x];
        break;
    case splashModeCMYK8:
        pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x];
        break;
    case splashModeDeviceN8:
        pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + (SPOT_NCOMPS + 4) * x];
        break;
    }
    if (bitmap->alpha) {
        pipe->destAlphaPtr = &bitmap->alpha[y * bitmap->width + x];
    } else {
        pipe->destAlphaPtr = nullptr;
    }
    if (state->inNonIsolatedGroup && alpha0Bitmap->alpha) {
        pipe->alpha0Ptr = &alpha0Bitmap->alpha[(alpha0Y + y) * alpha0Bitmap->width + (alpha0X + x)];
    } else {
        pipe->alpha0Ptr = nullptr;
    }
}

inline void Splash::pipeIncX(SplashPipe *pipe)
{
    ++pipe->x;
    if (state->softMask) {
        ++pipe->softMaskPtr;
    }
    switch (bitmap->mode) {
    case splashModeMono1:
        if (!(pipe->destColorMask >>= 1)) {
            pipe->destColorMask = 0x80;
            ++pipe->destColorPtr;
        }
        break;
    case splashModeMono8:
        ++pipe->destColorPtr;
        break;
    case splashModeRGB8:
    case splashModeBGR8:
        pipe->destColorPtr += 3;
        break;
    case splashModeXBGR8:
        pipe->destColorPtr += 4;
        break;
    case splashModeCMYK8:
        pipe->destColorPtr += 4;
        break;
    case splashModeDeviceN8:
        pipe->destColorPtr += (SPOT_NCOMPS + 4);
        break;
    }
    if (pipe->destAlphaPtr) {
        ++pipe->destAlphaPtr;
    }
    if (pipe->alpha0Ptr) {
        ++pipe->alpha0Ptr;
    }
}

inline void Splash::drawPixel(SplashPipe *pipe, int x, int y, bool noClip)
{
    if (unlikely(y < 0)) {
        return;
    }

    if (noClip || state->clip->test(x, y)) {
        pipeSetXY(pipe, x, y);
        (this->*pipe->run)(pipe);
    }
}

inline void Splash::drawAAPixelInit()
{
    aaBufY = -1;
}

inline void Splash::drawAAPixel(SplashPipe *pipe, int x, int y)
{
#if splashAASize == 4
    static const int bitCount4[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
    int w;
#else
    int xx, yy;
#endif
    SplashColorPtr p;
    int x0, x1, t;

    if (x < 0 || x >= bitmap->width || y < state->clip->getYMinI() || y > state->clip->getYMaxI()) {
        return;
    }

    // update aaBuf
    if (y != aaBufY) {
        memset(aaBuf->getDataPtr(), 0xff, aaBuf->getRowSize() * aaBuf->getHeight());
        x0 = 0;
        x1 = bitmap->width - 1;
        state->clip->clipAALine(aaBuf, &x0, &x1, y);
        aaBufY = y;
    }

    // compute the shape value
#if splashAASize == 4
    p = aaBuf->getDataPtr() + (x >> 1);
    w = aaBuf->getRowSize();
    if (x & 1) {
        t = bitCount4[*p & 0x0f] + bitCount4[p[w] & 0x0f] + bitCount4[p[2 * w] & 0x0f] + bitCount4[p[3 * w] & 0x0f];
    } else {
        t = bitCount4[*p >> 4] + bitCount4[p[w] >> 4] + bitCount4[p[2 * w] >> 4] + bitCount4[p[3 * w] >> 4];
    }
#else
    t = 0;
    for (yy = 0; yy < splashAASize; ++yy) {
        for (xx = 0; xx < splashAASize; ++xx) {
            p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + ((x * splashAASize + xx) >> 3);
            t += (*p >> (7 - ((x * splashAASize + xx) & 7))) & 1;
        }
    }
#endif

    // draw the pixel
    if (t != 0) {
        pipeSetXY(pipe, x, y);
        pipe->shape = div255(static_cast<int>(aaGamma[t] * pipe->shape));
        (this->*pipe->run)(pipe);
    }
}

inline void Splash::drawSpan(SplashPipe *pipe, int x0, int x1, int y, bool noClip)
{
    int x;

    if (noClip) {
        pipeSetXY(pipe, x0, y);
        for (x = x0; x <= x1; ++x) {
            (this->*pipe->run)(pipe);
        }
    } else {
        if (x0 < state->clip->getXMinI()) {
            x0 = state->clip->getXMinI();
        }
        if (x1 > state->clip->getXMaxI()) {
            x1 = state->clip->getXMaxI();
        }
        pipeSetXY(pipe, x0, y);
        for (x = x0; x <= x1; ++x) {
            if (state->clip->test(x, y)) {
                (this->*pipe->run)(pipe);
            } else {
                pipeIncX(pipe);
            }
        }
    }
}

inline void Splash::drawAALine(SplashPipe *pipe, int x0, int x1, int y, bool adjustLine, unsigned char lineOpacity)
{
#if splashAASize == 4
    static const int bitCount4[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
    SplashColorPtr p0, p1, p2, p3;
    int t;
#else
    SplashColorPtr p;
    int xx, yy, t;
#endif
    int x;

#if splashAASize == 4
    p0 = aaBuf->getDataPtr() + (x0 >> 1);
    p1 = p0 + aaBuf->getRowSize();
    p2 = p1 + aaBuf->getRowSize();
    p3 = p2 + aaBuf->getRowSize();
#endif
    pipeSetXY(pipe, x0, y);
    for (x = x0; x <= x1; ++x) {

        // compute the shape value
#if splashAASize == 4
        if (x & 1) {
            t = bitCount4[*p0 & 0x0f] + bitCount4[*p1 & 0x0f] + bitCount4[*p2 & 0x0f] + bitCount4[*p3 & 0x0f];
            ++p0;
            ++p1;
            ++p2;
            ++p3;
        } else {
            t = bitCount4[*p0 >> 4] + bitCount4[*p1 >> 4] + bitCount4[*p2 >> 4] + bitCount4[*p3 >> 4];
        }
#else
        t = 0;
        for (yy = 0; yy < splashAASize; ++yy) {
            for (xx = 0; xx < splashAASize; ++xx) {
                p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + ((x * splashAASize + xx) >> 3);
                t += (*p >> (7 - ((x * splashAASize + xx) & 7))) & 1;
            }
        }
#endif

        if (t != 0) {
            pipe->shape = (adjustLine) ? div255(static_cast<int>((int)lineOpacity * (double)aaGamma[t])) : (int)aaGamma[t];
            (this->*pipe->run)(pipe);
        } else {
            pipeIncX(pipe);
        }
    }
}

//------------------------------------------------------------------------

// Transform a point from user space to device space.
inline void Splash::transform(const SplashCoord *matrix, SplashCoord xi, SplashCoord yi, SplashCoord *xo, SplashCoord *yo)
{
    //                          [ m[0] m[1] 0 ]
    // [xo yo 1] = [xi yi 1] *  [ m[2] m[3] 0 ]
    //                          [ m[4] m[5] 1 ]
    *xo = xi * matrix[0] + yi * matrix[2] + matrix[4];
    *yo = xi * matrix[1] + yi * matrix[3] + matrix[5];
}

//------------------------------------------------------------------------
// Splash
//------------------------------------------------------------------------

Splash::Splash(SplashBitmap *bitmapA, bool vectorAntialiasA, SplashScreenParams *screenParams)
{
    int i;

    bitmap = bitmapA;
    vectorAntialias = vectorAntialiasA;
    inShading = false;
    state = new SplashState(bitmap->width, bitmap->height, vectorAntialias, screenParams);
    if (vectorAntialias) {
        aaBuf = new SplashBitmap(splashAASize * bitmap->width, splashAASize, 1, splashModeMono1, false);
        for (i = 0; i <= splashAASize * splashAASize; ++i) {
            aaGamma[i] = (unsigned char)splashRound(splashPow((SplashCoord)i / (SplashCoord)(splashAASize * splashAASize), splashAAGamma) * 255);
        }
    } else {
        aaBuf = nullptr;
    }
    minLineWidth = 0;
    thinLineMode = splashThinLineDefault;
    debugMode = false;
    alpha0Bitmap = nullptr;
}

Splash::Splash(SplashBitmap *bitmapA, bool vectorAntialiasA, SplashScreen *screenA)
{
    int i;

    bitmap = bitmapA;
    inShading = false;
    vectorAntialias = vectorAntialiasA;
    state = new SplashState(bitmap->width, bitmap->height, vectorAntialias, screenA);
    if (vectorAntialias) {
        aaBuf = new SplashBitmap(splashAASize * bitmap->width, splashAASize, 1, splashModeMono1, false);
        for (i = 0; i <= splashAASize * splashAASize; ++i) {
            aaGamma[i] = (unsigned char)splashRound(splashPow((SplashCoord)i / (SplashCoord)(splashAASize * splashAASize), splashAAGamma) * 255);
        }
    } else {
        aaBuf = nullptr;
    }
    minLineWidth = 0;
    thinLineMode = splashThinLineDefault;
    debugMode = false;
    alpha0Bitmap = nullptr;
}

Splash::~Splash()
{
    while (state->next) {
        restoreState();
    }
    delete state;
    delete aaBuf;
}

//------------------------------------------------------------------------
// state read
//------------------------------------------------------------------------

SplashCoord *Splash::getMatrix()
{
    return state->matrix;
}

SplashPattern *Splash::getStrokePattern()
{
    return state->strokePattern;
}

SplashPattern *Splash::getFillPattern()
{
    return state->fillPattern;
}

SplashScreen *Splash::getScreen()
{
    return state->screen;
}

SplashBlendFunc Splash::getBlendFunc()
{
    return state->blendFunc;
}

SplashCoord Splash::getStrokeAlpha()
{
    return state->strokeAlpha;
}

SplashCoord Splash::getFillAlpha()
{
    return state->fillAlpha;
}

SplashCoord Splash::getLineWidth()
{
    return state->lineWidth;
}

int Splash::getLineCap()
{
    return state->lineCap;
}

int Splash::getLineJoin()
{
    return state->lineJoin;
}

SplashCoord Splash::getMiterLimit()
{
    return state->miterLimit;
}

SplashCoord Splash::getFlatness()
{
    return state->flatness;
}

SplashCoord Splash::getLineDashPhase()
{
    return state->lineDashPhase;
}

bool Splash::getStrokeAdjust()
{
    return state->strokeAdjust;
}

SplashClip *Splash::getClip()
{
    return state->clip;
}

SplashBitmap *Splash::getSoftMask()
{
    return state->softMask;
}

bool Splash::getInNonIsolatedGroup()
{
    return state->inNonIsolatedGroup;
}

//------------------------------------------------------------------------
// state write
//------------------------------------------------------------------------

void Splash::setMatrix(SplashCoord *matrix)
{
    memcpy(state->matrix, matrix, 6 * sizeof(SplashCoord));
}

void Splash::setStrokePattern(SplashPattern *strokePattern)
{
    state->setStrokePattern(strokePattern);
}

void Splash::setFillPattern(SplashPattern *fillPattern)
{
    state->setFillPattern(fillPattern);
}

void Splash::setScreen(SplashScreen *screen)
{
    state->setScreen(screen);
}

void Splash::setBlendFunc(SplashBlendFunc func)
{
    state->blendFunc = func;
}

void Splash::setStrokeAlpha(SplashCoord alpha)
{
    state->strokeAlpha = (state->multiplyPatternAlpha) ? alpha * state->patternStrokeAlpha : alpha;
}

void Splash::setFillAlpha(SplashCoord alpha)
{
    state->fillAlpha = (state->multiplyPatternAlpha) ? alpha * state->patternFillAlpha : alpha;
}

void Splash::setPatternAlpha(SplashCoord strokeAlpha, SplashCoord fillAlpha)
{
    state->patternStrokeAlpha = strokeAlpha;
    state->patternFillAlpha = fillAlpha;
    state->multiplyPatternAlpha = true;
}

void Splash::clearPatternAlpha()
{
    state->patternStrokeAlpha = 1;
    state->patternFillAlpha = 1;
    state->multiplyPatternAlpha = false;
}

void Splash::setFillOverprint(bool fop)
{
    state->fillOverprint = fop;
}

void Splash::setStrokeOverprint(bool sop)
{
    state->strokeOverprint = sop;
}

void Splash::setOverprintMode(int opm)
{
    state->overprintMode = opm;
}

void Splash::setLineWidth(SplashCoord lineWidth)
{
    state->lineWidth = lineWidth;
}

void Splash::setLineCap(int lineCap)
{
    state->lineCap = lineCap;
}

void Splash::setLineJoin(int lineJoin)
{
    state->lineJoin = lineJoin;
}

void Splash::setMiterLimit(SplashCoord miterLimit)
{
    state->miterLimit = miterLimit;
}

void Splash::setFlatness(SplashCoord flatness)
{
    if (flatness < 1) {
        state->flatness = 1;
    } else {
        state->flatness = flatness;
    }
}

void Splash::setLineDash(std::vector<SplashCoord> &&lineDash, SplashCoord lineDashPhase)
{
    state->setLineDash(std::move(lineDash), lineDashPhase);
}

void Splash::setStrokeAdjust(bool strokeAdjust)
{
    state->strokeAdjust = strokeAdjust;
}

void Splash::clipResetToRect(SplashCoord x0, SplashCoord y0, SplashCoord x1, SplashCoord y1)
{
    state->clip->resetToRect(x0, y0, x1, y1);
}

SplashError Splash::clipToRect(SplashCoord x0, SplashCoord y0, SplashCoord x1, SplashCoord y1)
{
    return state->clip->clipToRect(x0, y0, x1, y1);
}

SplashError Splash::clipToPath(SplashPath *path, bool eo)
{
    return state->clip->clipToPath(path, state->matrix, state->flatness, eo);
}

void Splash::setSoftMask(SplashBitmap *softMask)
{
    state->setSoftMask(softMask);
}

void Splash::setInNonIsolatedGroup(SplashBitmap *alpha0BitmapA, int alpha0XA, int alpha0YA)
{
    alpha0Bitmap = alpha0BitmapA;
    alpha0X = alpha0XA;
    alpha0Y = alpha0YA;
    state->inNonIsolatedGroup = true;
}

void Splash::setTransfer(unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *gray)
{
    state->setTransfer(red, green, blue, gray);
}

void Splash::setOverprintMask(unsigned int overprintMask, bool additive)
{
    state->overprintMask = overprintMask;
    state->overprintAdditive = additive;
}

//------------------------------------------------------------------------
// state save/restore
//------------------------------------------------------------------------

void Splash::saveState()
{
    SplashState *newState;

    newState = state->copy();
    newState->next = state;
    state = newState;
}

SplashError Splash::restoreState()
{
    SplashState *oldState;

    if (!state->next) {
        return splashErrNoSave;
    }
    oldState = state;
    state = state->next;
    delete oldState;
    return splashOk;
}

//------------------------------------------------------------------------
// drawing operations
//------------------------------------------------------------------------

void Splash::clear(SplashColorPtr color, unsigned char alpha)
{
    SplashColorPtr row, p;
    unsigned char mono;
    int x, y;

    switch (bitmap->mode) {
    case splashModeMono1:
        mono = (color[0] & 0x80) ? 0xff : 0x00;
        if (bitmap->rowSize < 0) {
            memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1), mono, -bitmap->rowSize * bitmap->height);
        } else {
            memset(bitmap->data, mono, bitmap->rowSize * bitmap->height);
        }
        break;
    case splashModeMono8:
        if (bitmap->rowSize < 0) {
            memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1), color[0], -bitmap->rowSize * bitmap->height);
        } else {
            memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
        }
        break;
    case splashModeRGB8:
        if (color[0] == color[1] && color[1] == color[2]) {
            if (bitmap->rowSize < 0) {
                memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1), color[0], -bitmap->rowSize * bitmap->height);
            } else {
                memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
            }
        } else {
            row = bitmap->data;
            for (y = 0; y < bitmap->height; ++y) {
                p = row;
                for (x = 0; x < bitmap->width; ++x) {
                    *p++ = color[2];
                    *p++ = color[1];
                    *p++ = color[0];
                }
                row += bitmap->rowSize;
            }
        }
        break;
    case splashModeXBGR8:
        if (color[0] == color[1] && color[1] == color[2]) {
            if (bitmap->rowSize < 0) {
                memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1), color[0], -bitmap->rowSize * bitmap->height);
            } else {
                memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
            }
        } else {
            row = bitmap->data;
            for (y = 0; y < bitmap->height; ++y) {
                p = row;
                for (x = 0; x < bitmap->width; ++x) {
                    *p++ = color[0];
                    *p++ = color[1];
                    *p++ = color[2];
                    *p++ = 255;
                }
                row += bitmap->rowSize;
            }
        }
        break;
    case splashModeBGR8:
        if (color[0] == color[1] && color[1] == color[2]) {
            if (bitmap->rowSize < 0) {
                memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1), color[0], -bitmap->rowSize * bitmap->height);
            } else {
                memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
            }
        } else {
            row = bitmap->data;
            for (y = 0; y < bitmap->height; ++y) {
                p = row;
                for (x = 0; x < bitmap->width; ++x) {
                    *p++ = color[0];
                    *p++ = color[1];
                    *p++ = color[2];
                }
                row += bitmap->rowSize;
            }
        }
        break;
    case splashModeCMYK8:
        if (color[0] == color[1] && color[1] == color[2] && color[2] == color[3]) {
            if (bitmap->rowSize < 0) {
                memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1), color[0], -bitmap->rowSize * bitmap->height);
            } else {
                memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
            }
        } else {
            row = bitmap->data;
            for (y = 0; y < bitmap->height; ++y) {
                p = row;
                for (x = 0; x < bitmap->width; ++x) {
                    *p++ = color[0];
                    *p++ = color[1];
                    *p++ = color[2];
                    *p++ = color[3];
                }
                row += bitmap->rowSize;
            }
        }
        break;
    case splashModeDeviceN8:
        row = bitmap->data;
        for (y = 0; y < bitmap->height; ++y) {
            p = row;
            for (x = 0; x < bitmap->width; ++x) {
                for (int cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    *p++ = color[cp];
                }
            }
            row += bitmap->rowSize;
        }
        break;
    }

    if (bitmap->alpha) {
        memset(bitmap->alpha, alpha, bitmap->width * bitmap->height);
    }
}

SplashError Splash::stroke(SplashPath *path)
{
    SplashPath *path2, *dPath;
    SplashCoord d1, d2, t1, t2, w;

    if (debugMode) {
        printf("stroke [dash:%zu] [width:%.2f]:\n", state->lineDash.size(), (double)state->lineWidth);
        dumpPath(path);
    }
    opClipRes = splashClipAllOutside;
    if (path->length == 0) {
        return splashErrEmptyPath;
    }
    path2 = flattenPath(path, state->matrix, state->flatness);
    if (!state->lineDash.empty()) {
        dPath = makeDashedPath(path2);
        delete path2;
        path2 = dPath;
        if (path2->length == 0) {
            delete path2;
            return splashErrEmptyPath;
        }
    }

    // transform a unit square, and take the half the max of the two
    // diagonals; the product of this number and the line width is the
    // (approximate) transformed line width
    t1 = state->matrix[0] + state->matrix[2];
    t2 = state->matrix[1] + state->matrix[3];
    d1 = t1 * t1 + t2 * t2;
    t1 = state->matrix[0] - state->matrix[2];
    t2 = state->matrix[1] - state->matrix[3];
    d2 = t1 * t1 + t2 * t2;
    if (d2 > d1) {
        d1 = d2;
    }
    d1 *= 0.5;
    if (d1 > 0 && d1 * state->lineWidth * state->lineWidth < minLineWidth * minLineWidth) {
        w = minLineWidth / splashSqrt(d1);
        strokeWide(path2, w);
    } else if (bitmap->mode == splashModeMono1) {
        // this gets close to Adobe's behavior in mono mode
        if (d1 * state->lineWidth <= 2) {
            strokeNarrow(path2);
        } else {
            strokeWide(path2, state->lineWidth);
        }
    } else {
        if (state->lineWidth == 0) {
            strokeNarrow(path2);
        } else {
            strokeWide(path2, state->lineWidth);
        }
    }

    delete path2;
    return splashOk;
}

void Splash::strokeNarrow(SplashPath *path)
{
    SplashPipe pipe;
    SplashXPathSeg *seg;
    int x0, x1, y0, y1, xa, xb, y;
    SplashCoord dxdy;
    SplashClipResult clipRes;
    int nClipRes[3];
    int i;

    nClipRes[0] = nClipRes[1] = nClipRes[2] = 0;

    SplashXPath xPath(path, state->matrix, state->flatness, false);

    pipeInit(&pipe, 0, 0, state->strokePattern, nullptr, (unsigned char)splashRound(state->strokeAlpha * 255), false, false);

    for (i = 0, seg = xPath.segs; i < xPath.length; ++i, ++seg) {
        if (seg->y0 <= seg->y1) {
            y0 = splashFloor(seg->y0);
            y1 = splashFloor(seg->y1);
            x0 = splashFloor(seg->x0);
            x1 = splashFloor(seg->x1);
        } else {
            y0 = splashFloor(seg->y1);
            y1 = splashFloor(seg->y0);
            x0 = splashFloor(seg->x1);
            x1 = splashFloor(seg->x0);
        }
        if ((clipRes = state->clip->testRect(x0 <= x1 ? x0 : x1, y0, x0 <= x1 ? x1 : x0, y1)) != splashClipAllOutside) {
            if (y0 == y1) {
                if (x0 <= x1) {
                    drawSpan(&pipe, x0, x1, y0, clipRes == splashClipAllInside);
                } else {
                    drawSpan(&pipe, x1, x0, y0, clipRes == splashClipAllInside);
                }
            } else {
                dxdy = seg->dxdy;
                if (y0 < state->clip->getYMinI()) {
                    y0 = state->clip->getYMinI();
                    x0 = splashFloor(seg->x0 + (state->clip->getYMin() - seg->y0) * dxdy);
                }
                if (y1 > state->clip->getYMaxI()) {
                    y1 = state->clip->getYMaxI();
                    x1 = splashFloor(seg->x0 + (state->clip->getYMax() - seg->y0) * dxdy);
                }
                if (x0 <= x1) {
                    xa = x0;
                    for (y = y0; y <= y1; ++y) {
                        if (y < y1) {
                            xb = splashFloor(seg->x0 + ((SplashCoord)y + 1 - seg->y0) * dxdy);
                        } else {
                            xb = x1 + 1;
                        }
                        if (xa == xb) {
                            drawPixel(&pipe, xa, y, clipRes == splashClipAllInside);
                        } else {
                            drawSpan(&pipe, xa, xb - 1, y, clipRes == splashClipAllInside);
                        }
                        xa = xb;
                    }
                } else {
                    xa = x0;
                    for (y = y0; y <= y1; ++y) {
                        if (y < y1) {
                            xb = splashFloor(seg->x0 + ((SplashCoord)y + 1 - seg->y0) * dxdy);
                        } else {
                            xb = x1 - 1;
                        }
                        if (xa == xb) {
                            drawPixel(&pipe, xa, y, clipRes == splashClipAllInside);
                        } else {
                            drawSpan(&pipe, xb + 1, xa, y, clipRes == splashClipAllInside);
                        }
                        xa = xb;
                    }
                }
            }
        }
        ++nClipRes[clipRes];
    }
    if (nClipRes[splashClipPartial] || (nClipRes[splashClipAllInside] && nClipRes[splashClipAllOutside])) {
        opClipRes = splashClipPartial;
    } else if (nClipRes[splashClipAllInside]) {
        opClipRes = splashClipAllInside;
    } else {
        opClipRes = splashClipAllOutside;
    }
}

void Splash::strokeWide(SplashPath *path, SplashCoord w)
{
    SplashPath *path2;

    path2 = makeStrokePath(path, w, false);
    fillWithPattern(path2, false, state->strokePattern, state->strokeAlpha);
    delete path2;
}

SplashPath *Splash::flattenPath(SplashPath *path, SplashCoord *matrix, SplashCoord flatness)
{
    SplashPath *fPath;
    SplashCoord flatness2;
    unsigned char flag;
    int i;

    fPath = new SplashPath();
    flatness2 = flatness * flatness;
    i = 0;
    while (i < path->length) {
        flag = path->flags[i];
        if (flag & splashPathFirst) {
            fPath->moveTo(path->pts[i].x, path->pts[i].y);
            ++i;
        } else {
            if (flag & splashPathCurve) {
                flattenCurve(path->pts[i - 1].x, path->pts[i - 1].y, path->pts[i].x, path->pts[i].y, path->pts[i + 1].x, path->pts[i + 1].y, path->pts[i + 2].x, path->pts[i + 2].y, matrix, flatness2, fPath);
                i += 3;
            } else {
                fPath->lineTo(path->pts[i].x, path->pts[i].y);
                ++i;
            }
            if (path->flags[i - 1] & splashPathClosed) {
                fPath->close();
            }
        }
    }
    return fPath;
}

void Splash::flattenCurve(SplashCoord x0, SplashCoord y0, SplashCoord x1, SplashCoord y1, SplashCoord x2, SplashCoord y2, SplashCoord x3, SplashCoord y3, SplashCoord *matrix, SplashCoord flatness2, SplashPath *fPath)
{
    SplashCoord cx[splashMaxCurveSplits + 1][3];
    SplashCoord cy[splashMaxCurveSplits + 1][3];
    int cNext[splashMaxCurveSplits + 1];
    SplashCoord xl0, xl1, xl2, xr0, xr1, xr2, xr3, xx1, xx2, xh;
    SplashCoord yl0, yl1, yl2, yr0, yr1, yr2, yr3, yy1, yy2, yh;
    SplashCoord dx, dy, mx, my, tx, ty, d1, d2;
    int p1, p2, p3;

    // initial segment
    p1 = 0;
    p2 = splashMaxCurveSplits;
    cx[p1][0] = x0;
    cy[p1][0] = y0;
    cx[p1][1] = x1;
    cy[p1][1] = y1;
    cx[p1][2] = x2;
    cy[p1][2] = y2;
    cx[p2][0] = x3;
    cy[p2][0] = y3;
    cNext[p1] = p2;

    while (p1 < splashMaxCurveSplits) {

        // get the next segment
        xl0 = cx[p1][0];
        yl0 = cy[p1][0];
        xx1 = cx[p1][1];
        yy1 = cy[p1][1];
        xx2 = cx[p1][2];
        yy2 = cy[p1][2];
        p2 = cNext[p1];
        xr3 = cx[p2][0];
        yr3 = cy[p2][0];

        // compute the distances (in device space) from the control points
        // to the midpoint of the straight line (this is a bit of a hack,
        // but it's much faster than computing the actual distances to the
        // line)
        transform(matrix, (xl0 + xr3) * 0.5, (yl0 + yr3) * 0.5, &mx, &my);
        transform(matrix, xx1, yy1, &tx, &ty);
        dx = tx - mx;
        dy = ty - my;
        d1 = dx * dx + dy * dy;
        transform(matrix, xx2, yy2, &tx, &ty);
        dx = tx - mx;
        dy = ty - my;
        d2 = dx * dx + dy * dy;

        // if the curve is flat enough, or no more subdivisions are
        // allowed, add the straight line segment
        if (p2 - p1 == 1 || (d1 <= flatness2 && d2 <= flatness2)) {
            fPath->lineTo(xr3, yr3);
            p1 = p2;

            // otherwise, subdivide the curve
        } else {
            xl1 = splashAvg(xl0, xx1);
            yl1 = splashAvg(yl0, yy1);
            xh = splashAvg(xx1, xx2);
            yh = splashAvg(yy1, yy2);
            xl2 = splashAvg(xl1, xh);
            yl2 = splashAvg(yl1, yh);
            xr2 = splashAvg(xx2, xr3);
            yr2 = splashAvg(yy2, yr3);
            xr1 = splashAvg(xh, xr2);
            yr1 = splashAvg(yh, yr2);
            xr0 = splashAvg(xl2, xr1);
            yr0 = splashAvg(yl2, yr1);
            // add the new subdivision points
            p3 = (p1 + p2) / 2;
            cx[p1][1] = xl1;
            cy[p1][1] = yl1;
            cx[p1][2] = xl2;
            cy[p1][2] = yl2;
            cNext[p1] = p3;
            cx[p3][0] = xr0;
            cy[p3][0] = yr0;
            cx[p3][1] = xr1;
            cy[p3][1] = yr1;
            cx[p3][2] = xr2;
            cy[p3][2] = yr2;
            cNext[p3] = p2;
        }
    }
}

SplashPath *Splash::makeDashedPath(SplashPath *path)
{
    SplashPath *dPath;
    SplashCoord lineDashTotal;
    SplashCoord lineDashStartPhase, lineDashDist, segLen;
    SplashCoord x0, y0, x1, y1, xa, ya;
    bool lineDashStartOn, lineDashOn, newPath;
    int i, j, k;

    lineDashTotal = 0;
    for (SplashCoord dash : state->lineDash) {
        lineDashTotal += dash;
    }
    // Acrobat simply draws nothing if the dash array is [0]
    if (lineDashTotal == 0) {
        return new SplashPath();
    }
    lineDashStartPhase = state->lineDashPhase;
    i = splashFloor(lineDashStartPhase / lineDashTotal);
    lineDashStartPhase -= (SplashCoord)i * lineDashTotal;
    lineDashStartOn = true;
    size_t lineDashStartIdx = 0;
    if (lineDashStartPhase > 0) {
        while (lineDashStartIdx < state->lineDash.size() && lineDashStartPhase >= state->lineDash[lineDashStartIdx]) {
            lineDashStartOn = !lineDashStartOn;
            lineDashStartPhase -= state->lineDash[lineDashStartIdx];
            ++lineDashStartIdx;
        }
        if (unlikely(lineDashStartIdx == state->lineDash.size())) {
            return new SplashPath();
        }
    }

    dPath = new SplashPath();

    // process each subpath
    i = 0;
    while (i < path->length) {

        // find the end of the subpath
        for (j = i; j < path->length - 1 && !(path->flags[j] & splashPathLast); ++j) {
            ;
        }

        // initialize the dash parameters
        lineDashOn = lineDashStartOn;
        size_t lineDashIdx = lineDashStartIdx;
        lineDashDist = state->lineDash[lineDashIdx] - lineDashStartPhase;

        // process each segment of the subpath
        newPath = true;
        for (k = i; k < j; ++k) {

            // grab the segment
            x0 = path->pts[k].x;
            y0 = path->pts[k].y;
            x1 = path->pts[k + 1].x;
            y1 = path->pts[k + 1].y;
            segLen = splashDist(x0, y0, x1, y1);

            // process the segment
            while (segLen > 0) {

                if (lineDashDist >= segLen) {
                    if (lineDashOn) {
                        if (newPath) {
                            dPath->moveTo(x0, y0);
                            newPath = false;
                        }
                        dPath->lineTo(x1, y1);
                    }
                    lineDashDist -= segLen;
                    segLen = 0;

                } else {
                    xa = x0 + (lineDashDist / segLen) * (x1 - x0);
                    ya = y0 + (lineDashDist / segLen) * (y1 - y0);
                    if (lineDashOn) {
                        if (newPath) {
                            dPath->moveTo(x0, y0);
                            newPath = false;
                        }
                        dPath->lineTo(xa, ya);
                    }
                    x0 = xa;
                    y0 = ya;
                    segLen -= lineDashDist;
                    lineDashDist = 0;
                }

                // get the next entry in the dash array
                if (lineDashDist <= 0) {
                    lineDashOn = !lineDashOn;
                    if (++lineDashIdx == state->lineDash.size()) {
                        lineDashIdx = 0;
                    }
                    lineDashDist = state->lineDash[lineDashIdx];
                    newPath = true;
                }
            }
        }
        i = j + 1;
    }

    if (dPath->length == 0) {
        bool allSame = true;
        for (i = 0; allSame && i < path->length - 1; ++i) {
            allSame = path->pts[i].x == path->pts[i + 1].x && path->pts[i].y == path->pts[i + 1].y;
        }
        if (allSame) {
            x0 = path->pts[0].x;
            y0 = path->pts[0].y;
            dPath->moveTo(x0, y0);
            dPath->lineTo(x0, y0);
        }
    }

    return dPath;
}

SplashError Splash::fill(SplashPath *path, bool eo)
{
    if (debugMode) {
        printf("fill [eo:%d]:\n", eo);
        dumpPath(path);
    }
    return fillWithPattern(path, eo, state->fillPattern, state->fillAlpha);
}

inline void Splash::getBBoxFP(SplashPath *path, SplashCoord *xMinA, SplashCoord *yMinA, SplashCoord *xMaxA, SplashCoord *yMaxA)
{
    SplashCoord xMinFP, yMinFP, xMaxFP, yMaxFP, tx, ty;

    // make compiler happy:
    xMinFP = xMaxFP = yMinFP = yMaxFP = 0;
    for (int i = 0; i < path->length; ++i) {
        transform(state->matrix, path->pts[i].x, path->pts[i].y, &tx, &ty);
        if (i == 0) {
            xMinFP = xMaxFP = tx;
            yMinFP = yMaxFP = ty;
        } else {
            if (tx < xMinFP) {
                xMinFP = tx;
            }
            if (tx > xMaxFP) {
                xMaxFP = tx;
            }
            if (ty < yMinFP) {
                yMinFP = ty;
            }
            if (ty > yMaxFP) {
                yMaxFP = ty;
            }
        }
    }

    *xMinA = xMinFP;
    *yMinA = yMinFP;
    *xMaxA = xMaxFP;
    *yMaxA = yMaxFP;
}

SplashError Splash::fillWithPattern(SplashPath *path, bool eo, SplashPattern *pattern, SplashCoord alpha)
{
    SplashPipe pipe = {};
    int xMinI, yMinI, xMaxI, yMaxI, x0, x1, y;
    SplashClipResult clipRes, clipRes2;
    bool adjustLine = false;
    int linePosI = 0;

    if (path->length == 0) {
        return splashErrEmptyPath;
    }
    if (pathAllOutside(path)) {
        opClipRes = splashClipAllOutside;
        return splashOk;
    }

    // add stroke adjustment hints for filled rectangles -- this only
    // applies to paths that consist of a single subpath
    // (this appears to match Acrobat's behavior)
    if (state->strokeAdjust && !path->hints) {
        int n;
        n = path->getLength();
        if (n == 4 && !(path->flags[0] & splashPathClosed) && !(path->flags[1] & splashPathLast) && !(path->flags[2] & splashPathLast)) {
            path->close(true);
            path->addStrokeAdjustHint(0, 2, 0, 4);
            path->addStrokeAdjustHint(1, 3, 0, 4);
        } else if (n == 5 && (path->flags[0] & splashPathClosed) && !(path->flags[1] & splashPathLast) && !(path->flags[2] & splashPathLast) && !(path->flags[3] & splashPathLast)) {
            path->addStrokeAdjustHint(0, 2, 0, 4);
            path->addStrokeAdjustHint(1, 3, 0, 4);
        }
    }

    if (thinLineMode != splashThinLineDefault) {
        if (state->clip->getXMinI() == state->clip->getXMaxI()) {
            linePosI = state->clip->getXMinI();
            adjustLine = true;
        } else if (state->clip->getXMinI() == state->clip->getXMaxI() - 1) {
            adjustLine = true;
            linePosI = splashFloor(state->clip->getXMin() + state->lineWidth);
        } else if (state->clip->getYMinI() == state->clip->getYMaxI()) {
            linePosI = state->clip->getYMinI();
            adjustLine = true;
        } else if (state->clip->getYMinI() == state->clip->getYMaxI() - 1) {
            adjustLine = true;
            linePosI = splashFloor(state->clip->getYMin() + state->lineWidth);
        }
    }

    SplashXPath xPath(path, state->matrix, state->flatness, true, adjustLine, linePosI);
    if (vectorAntialias && !inShading) {
        xPath.aaScale();
    }
    xPath.sort();
    yMinI = state->clip->getYMinI();
    yMaxI = state->clip->getYMaxI();
    if (vectorAntialias && !inShading) {
        yMinI = yMinI * splashAASize;
        yMaxI = (yMaxI + 1) * splashAASize - 1;
    }
    SplashXPathScanner scanner(xPath, eo, yMinI, yMaxI);

    // get the min and max x and y values
    if (vectorAntialias && !inShading) {
        scanner.getBBoxAA(&xMinI, &yMinI, &xMaxI, &yMaxI);
    } else {
        scanner.getBBox(&xMinI, &yMinI, &xMaxI, &yMaxI);
    }

    if (eo && (yMinI == yMaxI || xMinI == xMaxI) && thinLineMode != splashThinLineDefault) {
        SplashCoord delta, xMinFP, yMinFP, xMaxFP, yMaxFP;
        getBBoxFP(path, &xMinFP, &yMinFP, &xMaxFP, &yMaxFP);
        delta = (yMinI == yMaxI) ? yMaxFP - yMinFP : xMaxFP - xMinFP;
        if (delta < 0.2) {
            opClipRes = splashClipAllOutside;
            return splashOk;
        }
    }

    // check clipping
    if ((clipRes = state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI)) != splashClipAllOutside) {
        if (scanner.hasPartialClip()) {
            clipRes = splashClipPartial;
        }

        pipeInit(&pipe, 0, yMinI, pattern, nullptr, (unsigned char)splashRound(alpha * 255), vectorAntialias && !inShading, false);

        // draw the spans
        if (vectorAntialias && !inShading) {
            for (y = yMinI; y <= yMaxI; ++y) {
                scanner.renderAALine(aaBuf, &x0, &x1, y, thinLineMode != splashThinLineDefault && xMinI == xMaxI);
                if (clipRes != splashClipAllInside) {
                    state->clip->clipAALine(aaBuf, &x0, &x1, y, thinLineMode != splashThinLineDefault && xMinI == xMaxI);
                }
                unsigned char lineShape = 255;
                bool doAdjustLine = false;
                if (thinLineMode == splashThinLineShape && (xMinI == xMaxI || yMinI == yMaxI)) {
                    // compute line shape for thin lines:
                    SplashCoord mx, my, delta;
                    transform(state->matrix, 0, 0, &mx, &my);
                    transform(state->matrix, state->lineWidth, 0, &delta, &my);
                    doAdjustLine = true;
                    lineShape = clip255(static_cast<int>((delta - mx) * 255));
                }
                drawAALine(&pipe, x0, x1, y, doAdjustLine, lineShape);
            }
        } else {
            for (y = yMinI; y <= yMaxI; ++y) {
                SplashXPathScanIterator iterator(scanner, y);
                while (iterator.getNextSpan(&x0, &x1)) {
                    if (clipRes == splashClipAllInside) {
                        drawSpan(&pipe, x0, x1, y, true);
                    } else {
                        // limit the x range
                        if (x0 < state->clip->getXMinI()) {
                            x0 = state->clip->getXMinI();
                        }
                        if (x1 > state->clip->getXMaxI()) {
                            x1 = state->clip->getXMaxI();
                        }
                        clipRes2 = state->clip->testSpan(x0, x1, y);
                        drawSpan(&pipe, x0, x1, y, clipRes2 == splashClipAllInside);
                    }
                }
            }
        }
    }
    opClipRes = clipRes;

    return splashOk;
}

bool Splash::pathAllOutside(SplashPath *path)
{
    SplashCoord xMin1, yMin1, xMax1, yMax1;
    SplashCoord xMin2, yMin2, xMax2, yMax2;
    SplashCoord x, y;
    int xMinI, yMinI, xMaxI, yMaxI;
    int i;

    xMin1 = xMax1 = path->pts[0].x;
    yMin1 = yMax1 = path->pts[0].y;
    for (i = 1; i < path->length; ++i) {
        if (path->pts[i].x < xMin1) {
            xMin1 = path->pts[i].x;
        } else if (path->pts[i].x > xMax1) {
            xMax1 = path->pts[i].x;
        }
        if (path->pts[i].y < yMin1) {
            yMin1 = path->pts[i].y;
        } else if (path->pts[i].y > yMax1) {
            yMax1 = path->pts[i].y;
        }
    }

    transform(state->matrix, xMin1, yMin1, &x, &y);
    xMin2 = xMax2 = x;
    yMin2 = yMax2 = y;
    transform(state->matrix, xMin1, yMax1, &x, &y);
    if (x < xMin2) {
        xMin2 = x;
    } else if (x > xMax2) {
        xMax2 = x;
    }
    if (y < yMin2) {
        yMin2 = y;
    } else if (y > yMax2) {
        yMax2 = y;
    }
    transform(state->matrix, xMax1, yMin1, &x, &y);
    if (x < xMin2) {
        xMin2 = x;
    } else if (x > xMax2) {
        xMax2 = x;
    }
    if (y < yMin2) {
        yMin2 = y;
    } else if (y > yMax2) {
        yMax2 = y;
    }
    transform(state->matrix, xMax1, yMax1, &x, &y);
    if (x < xMin2) {
        xMin2 = x;
    } else if (x > xMax2) {
        xMax2 = x;
    }
    if (y < yMin2) {
        yMin2 = y;
    } else if (y > yMax2) {
        yMax2 = y;
    }
    xMinI = splashFloor(xMin2);
    yMinI = splashFloor(yMin2);
    xMaxI = splashFloor(xMax2);
    yMaxI = splashFloor(yMax2);

    return state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI) == splashClipAllOutside;
}

SplashError Splash::xorFill(SplashPath *path, bool eo)
{
    SplashPipe pipe;
    int xMinI, yMinI, xMaxI, yMaxI, x0, x1, y;
    SplashClipResult clipRes, clipRes2;
    SplashBlendFunc origBlendFunc;

    if (path->length == 0) {
        return splashErrEmptyPath;
    }
    SplashXPath xPath(path, state->matrix, state->flatness, true);
    xPath.sort();
    SplashXPathScanner scanner(xPath, eo, state->clip->getYMinI(), state->clip->getYMaxI());

    // get the min and max x and y values
    scanner.getBBox(&xMinI, &yMinI, &xMaxI, &yMaxI);

    // check clipping
    if ((clipRes = state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI)) != splashClipAllOutside) {
        if (scanner.hasPartialClip()) {
            clipRes = splashClipPartial;
        }

        origBlendFunc = state->blendFunc;
        state->blendFunc = &blendXor;
        pipeInit(&pipe, 0, yMinI, state->fillPattern, nullptr, 255, false, false);

        // draw the spans
        for (y = yMinI; y <= yMaxI; ++y) {
            SplashXPathScanIterator iterator(scanner, y);
            while (iterator.getNextSpan(&x0, &x1)) {
                if (clipRes == splashClipAllInside) {
                    drawSpan(&pipe, x0, x1, y, true);
                } else {
                    // limit the x range
                    if (x0 < state->clip->getXMinI()) {
                        x0 = state->clip->getXMinI();
                    }
                    if (x1 > state->clip->getXMaxI()) {
                        x1 = state->clip->getXMaxI();
                    }
                    clipRes2 = state->clip->testSpan(x0, x1, y);
                    drawSpan(&pipe, x0, x1, y, clipRes2 == splashClipAllInside);
                }
            }
        }
        state->blendFunc = origBlendFunc;
    }
    opClipRes = clipRes;

    return splashOk;
}

SplashError Splash::fillChar(SplashCoord x, SplashCoord y, int c, SplashFont *font)
{
    SplashGlyphBitmap glyph;
    SplashCoord xt, yt;
    int x0, y0, xFrac, yFrac;
    SplashClipResult clipRes;

    if (debugMode) {
        printf("fillChar: x=%.2f y=%.2f c=%3d=0x%02x='%c'\n", (double)x, (double)y, c, c, c);
    }
    transform(state->matrix, x, y, &xt, &yt);
    x0 = splashFloor(xt);
    xFrac = splashFloor((xt - x0) * splashFontFraction);
    y0 = splashFloor(yt);
    yFrac = splashFloor((yt - y0) * splashFontFraction);
    if (!font->getGlyph(c, xFrac, yFrac, &glyph, x0, y0, state->clip, &clipRes)) {
        return splashErrNoGlyph;
    }
    if (clipRes != splashClipAllOutside) {
        fillGlyph2(x0, y0, &glyph, clipRes == splashClipAllInside);
    }
    opClipRes = clipRes;
    if (glyph.freeData) {
        gfree(glyph.data);
    }
    return splashOk;
}

void Splash::fillGlyph(SplashCoord x, SplashCoord y, SplashGlyphBitmap *glyph)
{
    SplashCoord xt, yt;
    int x0, y0;

    transform(state->matrix, x, y, &xt, &yt);
    x0 = splashFloor(xt);
    y0 = splashFloor(yt);
    SplashClipResult clipRes = state->clip->testRect(x0 - glyph->x, y0 - glyph->y, x0 - glyph->x + glyph->w - 1, y0 - glyph->y + glyph->h - 1);
    if (clipRes != splashClipAllOutside) {
        fillGlyph2(x0, y0, glyph, clipRes == splashClipAllInside);
    }
    opClipRes = clipRes;
}

void Splash::fillGlyph2(int x0, int y0, SplashGlyphBitmap *glyph, bool noClip)
{
    SplashPipe pipe;
    int alpha0;
    unsigned char alpha;
    unsigned char *p;
    int x1, y1, xx, xx1, yy;

    p = glyph->data;
    int xStart = x0 - glyph->x;
    int yStart = y0 - glyph->y;
    int xxLimit = glyph->w;
    int yyLimit = glyph->h;
    int xShift = 0;

    if (yStart < 0) {
        p += (glyph->aa ? glyph->w : splashCeil(glyph->w / 8.0)) * -yStart; // move p to the beginning of the first painted row
        yyLimit += yStart;
        yStart = 0;
    }

    if (xStart < 0) {
        if (glyph->aa) {
            p += -xStart;
        } else {
            p += (-xStart) / 8;
            xShift = (-xStart) % 8;
        }
        xxLimit += xStart;
        xStart = 0;
    }

    if (xxLimit + xStart >= bitmap->width) {
        xxLimit = bitmap->width - xStart;
    }
    if (yyLimit + yStart >= bitmap->height) {
        yyLimit = bitmap->height - yStart;
    }

    if (noClip) {
        if (glyph->aa) {
            pipeInit(&pipe, xStart, yStart, state->fillPattern, nullptr, (unsigned char)splashRound(state->fillAlpha * 255), true, false);
            for (yy = 0, y1 = yStart; yy < yyLimit; ++yy, ++y1) {
                pipeSetXY(&pipe, xStart, y1);
                for (xx = 0, x1 = xStart; xx < xxLimit; ++xx, ++x1) {
                    alpha = p[xx];
                    if (alpha != 0) {
                        pipe.shape = alpha;
                        (this->*pipe.run)(&pipe);
                    } else {
                        pipeIncX(&pipe);
                    }
                }
                p += glyph->w;
            }
        } else {
            const int widthEight = splashCeil(glyph->w / 8.0);

            pipeInit(&pipe, xStart, yStart, state->fillPattern, nullptr, (unsigned char)splashRound(state->fillAlpha * 255), false, false);
            for (yy = 0, y1 = yStart; yy < yyLimit; ++yy, ++y1) {
                pipeSetXY(&pipe, xStart, y1);
                for (xx = 0, x1 = xStart; xx < xxLimit; xx += 8) {
                    alpha0 = (xShift > 0 && xx < xxLimit - 8 ? (p[xx / 8] << xShift) | (p[xx / 8 + 1] >> (8 - xShift)) : p[xx / 8]);
                    for (xx1 = 0; xx1 < 8 && xx + xx1 < xxLimit; ++xx1, ++x1) {
                        if (alpha0 & 0x80) {
                            (this->*pipe.run)(&pipe);
                        } else {
                            pipeIncX(&pipe);
                        }
                        alpha0 <<= 1;
                    }
                }
                p += widthEight;
            }
        }
    } else {
        if (glyph->aa) {
            pipeInit(&pipe, xStart, yStart, state->fillPattern, nullptr, (unsigned char)splashRound(state->fillAlpha * 255), true, false);
            for (yy = 0, y1 = yStart; yy < yyLimit; ++yy, ++y1) {
                pipeSetXY(&pipe, xStart, y1);
                for (xx = 0, x1 = xStart; xx < xxLimit; ++xx, ++x1) {
                    if (state->clip->test(x1, y1)) {
                        alpha = p[xx];
                        if (alpha != 0) {
                            pipe.shape = alpha;
                            (this->*pipe.run)(&pipe);
                        } else {
                            pipeIncX(&pipe);
                        }
                    } else {
                        pipeIncX(&pipe);
                    }
                }
                p += glyph->w;
            }
        } else {
            const int widthEight = splashCeil(glyph->w / 8.0);

            pipeInit(&pipe, xStart, yStart, state->fillPattern, nullptr, (unsigned char)splashRound(state->fillAlpha * 255), false, false);
            for (yy = 0, y1 = yStart; yy < yyLimit; ++yy, ++y1) {
                pipeSetXY(&pipe, xStart, y1);
                for (xx = 0, x1 = xStart; xx < xxLimit; xx += 8) {
                    alpha0 = (xShift > 0 && xx < xxLimit - 8 ? (p[xx / 8] << xShift) | (p[xx / 8 + 1] >> (8 - xShift)) : p[xx / 8]);
                    for (xx1 = 0; xx1 < 8 && xx + xx1 < xxLimit; ++xx1, ++x1) {
                        if (state->clip->test(x1, y1)) {
                            if (alpha0 & 0x80) {
                                (this->*pipe.run)(&pipe);
                            } else {
                                pipeIncX(&pipe);
                            }
                        } else {
                            pipeIncX(&pipe);
                        }
                        alpha0 <<= 1;
                    }
                }
                p += widthEight;
            }
        }
    }
}

SplashError Splash::fillImageMask(SplashImageMaskSource src, void *srcData, int w, int h, SplashCoord *mat, bool glyphMode)
{
    SplashBitmap *scaledMask;
    SplashClipResult clipRes;
    bool minorAxisZero;
    int x0, y0, x1, y1, scaledWidth, scaledHeight;
    int yp;

    if (debugMode) {
        printf("fillImageMask: w=%d h=%d mat=[%.2f %.2f %.2f %.2f %.2f %.2f]\n", w, h, (double)mat[0], (double)mat[1], (double)mat[2], (double)mat[3], (double)mat[4], (double)mat[5]);
    }

    if (w == 0 && h == 0) {
        return splashErrZeroImage;
    }

    // check for singular matrix
    if (!splashCheckDet(mat[0], mat[1], mat[2], mat[3], 0.000001)) {
        return splashErrSingularMatrix;
    }

    minorAxisZero = mat[1] == 0 && mat[2] == 0;

    // scaling only
    if (mat[0] > 0 && minorAxisZero && mat[3] > 0) {
        x0 = imgCoordMungeLowerC(mat[4], glyphMode);
        y0 = imgCoordMungeLowerC(mat[5], glyphMode);
        x1 = imgCoordMungeUpperC(mat[0] + mat[4], glyphMode);
        y1 = imgCoordMungeUpperC(mat[3] + mat[5], glyphMode);
        // make sure narrow images cover at least one pixel
        if (x0 == x1) {
            ++x1;
        }
        if (y0 == y1) {
            ++y1;
        }
        clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
        opClipRes = clipRes;
        if (clipRes != splashClipAllOutside) {
            scaledWidth = x1 - x0;
            scaledHeight = y1 - y0;
            yp = h / scaledHeight;
            if (yp < 0 || yp > INT_MAX - 1) {
                return splashErrBadArg;
            }
            scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight);
            blitMask(scaledMask, x0, y0, clipRes);
            delete scaledMask;
        }

        // scaling plus vertical flip
    } else if (mat[0] > 0 && minorAxisZero && mat[3] < 0) {
        x0 = imgCoordMungeLowerC(mat[4], glyphMode);
        y0 = imgCoordMungeLowerC(mat[3] + mat[5], glyphMode);
        x1 = imgCoordMungeUpperC(mat[0] + mat[4], glyphMode);
        y1 = imgCoordMungeUpperC(mat[5], glyphMode);
        // make sure narrow images cover at least one pixel
        if (x0 == x1) {
            ++x1;
        }
        if (y0 == y1) {
            ++y1;
        }
        clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
        opClipRes = clipRes;
        if (clipRes != splashClipAllOutside) {
            scaledWidth = x1 - x0;
            scaledHeight = y1 - y0;
            yp = h / scaledHeight;
            if (yp < 0 || yp > INT_MAX - 1) {
                return splashErrBadArg;
            }
            scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight);
            vertFlipImage(scaledMask, scaledWidth, scaledHeight, 1);
            blitMask(scaledMask, x0, y0, clipRes);
            delete scaledMask;
        }

        // all other cases
    } else {
        arbitraryTransformMask(src, srcData, w, h, mat, glyphMode);
    }

    return splashOk;
}

void Splash::arbitraryTransformMask(SplashImageMaskSource src, void *srcData, int srcWidth, int srcHeight, SplashCoord *mat, bool glyphMode)
{
    SplashBitmap *scaledMask;
    SplashClipResult clipRes, clipRes2;
    SplashPipe pipe;
    int scaledWidth, scaledHeight, t0, t1;
    SplashCoord r00, r01, r10, r11, det, ir00, ir01, ir10, ir11;
    SplashCoord vx[4], vy[4];
    int xMin, yMin, xMax, yMax;
    ImageSection section[3];
    int nSections;
    int y, xa, xb, x, i, xx, yy;

    // compute the four vertices of the target quadrilateral
    vx[0] = mat[4];
    vy[0] = mat[5];
    vx[1] = mat[2] + mat[4];
    vy[1] = mat[3] + mat[5];
    vx[2] = mat[0] + mat[2] + mat[4];
    vy[2] = mat[1] + mat[3] + mat[5];
    vx[3] = mat[0] + mat[4];
    vy[3] = mat[1] + mat[5];

    // make sure vx/vy fit in integers since we're transforming them to in the next lines
    for (i = 0; i < 4; ++i) {
        if (unlikely(vx[i] < INT_MIN || vx[i] > INT_MAX || vy[i] < INT_MIN || vy[i] > INT_MAX)) {
            error(errInternal, -1, "arbitraryTransformMask vertices values don't fit in an integer");
            return;
        }
    }

    // clipping
    xMin = imgCoordMungeLowerC(vx[0], glyphMode);
    xMax = imgCoordMungeUpperC(vx[0], glyphMode);
    yMin = imgCoordMungeLowerC(vy[0], glyphMode);
    yMax = imgCoordMungeUpperC(vy[0], glyphMode);
    for (i = 1; i < 4; ++i) {
        t0 = imgCoordMungeLowerC(vx[i], glyphMode);
        if (t0 < xMin) {
            xMin = t0;
        }
        t0 = imgCoordMungeUpperC(vx[i], glyphMode);
        if (t0 > xMax) {
            xMax = t0;
        }
        t1 = imgCoordMungeLowerC(vy[i], glyphMode);
        if (t1 < yMin) {
            yMin = t1;
        }
        t1 = imgCoordMungeUpperC(vy[i], glyphMode);
        if (t1 > yMax) {
            yMax = t1;
        }
    }
    clipRes = state->clip->testRect(xMin, yMin, xMax - 1, yMax - 1);
    opClipRes = clipRes;
    if (clipRes == splashClipAllOutside) {
        return;
    }

    // compute the scale factors
    if (mat[0] >= 0) {
        t0 = imgCoordMungeUpperC(mat[0] + mat[4], glyphMode) - imgCoordMungeLowerC(mat[4], glyphMode);
    } else {
        t0 = imgCoordMungeUpperC(mat[4], glyphMode) - imgCoordMungeLowerC(mat[0] + mat[4], glyphMode);
    }
    if (mat[1] >= 0) {
        t1 = imgCoordMungeUpperC(mat[1] + mat[5], glyphMode) - imgCoordMungeLowerC(mat[5], glyphMode);
    } else {
        t1 = imgCoordMungeUpperC(mat[5], glyphMode) - imgCoordMungeLowerC(mat[1] + mat[5], glyphMode);
    }
    scaledWidth = t0 > t1 ? t0 : t1;
    if (mat[2] >= 0) {
        t0 = imgCoordMungeUpperC(mat[2] + mat[4], glyphMode) - imgCoordMungeLowerC(mat[4], glyphMode);
    } else {
        t0 = imgCoordMungeUpperC(mat[4], glyphMode) - imgCoordMungeLowerC(mat[2] + mat[4], glyphMode);
    }
    if (mat[3] >= 0) {
        t1 = imgCoordMungeUpperC(mat[3] + mat[5], glyphMode) - imgCoordMungeLowerC(mat[5], glyphMode);
    } else {
        t1 = imgCoordMungeUpperC(mat[5], glyphMode) - imgCoordMungeLowerC(mat[3] + mat[5], glyphMode);
    }
    scaledHeight = t0 > t1 ? t0 : t1;
    if (scaledWidth == 0) {
        scaledWidth = 1;
    }
    if (scaledHeight == 0) {
        scaledHeight = 1;
    }

    // compute the inverse transform (after scaling) matrix
    r00 = mat[0] / scaledWidth;
    r01 = mat[1] / scaledWidth;
    r10 = mat[2] / scaledHeight;
    r11 = mat[3] / scaledHeight;
    det = r00 * r11 - r01 * r10;
    if (splashAbs(det) < 1e-6) {
        // this should be caught by the singular matrix check in fillImageMask
        return;
    }
    ir00 = r11 / det;
    ir01 = -r01 / det;
    ir10 = -r10 / det;
    ir11 = r00 / det;

    // scale the input image
    scaledMask = scaleMask(src, srcData, srcWidth, srcHeight, scaledWidth, scaledHeight);
    if (scaledMask->data == nullptr) {
        error(errInternal, -1, "scaledMask->data is NULL in Splash::arbitraryTransformMask");
        delete scaledMask;
        return;
    }

    // construct the three sections
    i = (vy[2] <= vy[3]) ? 2 : 3;
    if (vy[1] <= vy[i]) {
        i = 1;
    }
    if (vy[0] < vy[i] || (i != 3 && vy[0] == vy[i])) {
        i = 0;
    }
    if (vy[i] == vy[(i + 1) & 3]) {
        section[0].y0 = imgCoordMungeLowerC(vy[i], glyphMode);
        section[0].y1 = imgCoordMungeUpperC(vy[(i + 2) & 3], glyphMode) - 1;
        if (vx[i] < vx[(i + 1) & 3]) {
            section[0].ia0 = i;
            section[0].ia1 = (i + 3) & 3;
            section[0].ib0 = (i + 1) & 3;
            section[0].ib1 = (i + 2) & 3;
        } else {
            section[0].ia0 = (i + 1) & 3;
            section[0].ia1 = (i + 2) & 3;
            section[0].ib0 = i;
            section[0].ib1 = (i + 3) & 3;
        }
        nSections = 1;
    } else {
        section[0].y0 = imgCoordMungeLowerC(vy[i], glyphMode);
        section[2].y1 = imgCoordMungeUpperC(vy[(i + 2) & 3], glyphMode) - 1;
        section[0].ia0 = section[0].ib0 = i;
        section[2].ia1 = section[2].ib1 = (i + 2) & 3;
        if (vx[(i + 1) & 3] < vx[(i + 3) & 3]) {
            section[0].ia1 = section[2].ia0 = (i + 1) & 3;
            section[0].ib1 = section[2].ib0 = (i + 3) & 3;
        } else {
            section[0].ia1 = section[2].ia0 = (i + 3) & 3;
            section[0].ib1 = section[2].ib0 = (i + 1) & 3;
        }
        if (vy[(i + 1) & 3] < vy[(i + 3) & 3]) {
            section[1].y0 = imgCoordMungeLowerC(vy[(i + 1) & 3], glyphMode);
            section[2].y0 = imgCoordMungeUpperC(vy[(i + 3) & 3], glyphMode);
            if (vx[(i + 1) & 3] < vx[(i + 3) & 3]) {
                section[1].ia0 = (i + 1) & 3;
                section[1].ia1 = (i + 2) & 3;
                section[1].ib0 = i;
                section[1].ib1 = (i + 3) & 3;
            } else {
                section[1].ia0 = i;
                section[1].ia1 = (i + 3) & 3;
                section[1].ib0 = (i + 1) & 3;
                section[1].ib1 = (i + 2) & 3;
            }
        } else {
            section[1].y0 = imgCoordMungeLowerC(vy[(i + 3) & 3], glyphMode);
            section[2].y0 = imgCoordMungeUpperC(vy[(i + 1) & 3], glyphMode);
            if (vx[(i + 1) & 3] < vx[(i + 3) & 3]) {
                section[1].ia0 = i;
                section[1].ia1 = (i + 1) & 3;
                section[1].ib0 = (i + 3) & 3;
                section[1].ib1 = (i + 2) & 3;
            } else {
                section[1].ia0 = (i + 3) & 3;
                section[1].ia1 = (i + 2) & 3;
                section[1].ib0 = i;
                section[1].ib1 = (i + 1) & 3;
            }
        }
        section[0].y1 = section[1].y0 - 1;
        section[1].y1 = section[2].y0 - 1;
        nSections = 3;
    }
    for (i = 0; i < nSections; ++i) {
        section[i].xa0 = vx[section[i].ia0];
        section[i].ya0 = vy[section[i].ia0];
        section[i].xa1 = vx[section[i].ia1];
        section[i].ya1 = vy[section[i].ia1];
        section[i].xb0 = vx[section[i].ib0];
        section[i].yb0 = vy[section[i].ib0];
        section[i].xb1 = vx[section[i].ib1];
        section[i].yb1 = vy[section[i].ib1];
        section[i].dxdya = (section[i].xa1 - section[i].xa0) / (section[i].ya1 - section[i].ya0);
        section[i].dxdyb = (section[i].xb1 - section[i].xb0) / (section[i].yb1 - section[i].yb0);
    }

    // initialize the pixel pipe
    pipeInit(&pipe, 0, 0, state->fillPattern, nullptr, (unsigned char)splashRound(state->fillAlpha * 255), true, false);
    if (vectorAntialias) {
        drawAAPixelInit();
    }

    // make sure narrow images cover at least one pixel
    if (nSections == 1) {
        if (section[0].y0 == section[0].y1) {
            ++section[0].y1;
            clipRes = opClipRes = splashClipPartial;
        }
    } else {
        if (section[0].y0 == section[2].y1) {
            ++section[1].y1;
            clipRes = opClipRes = splashClipPartial;
        }
    }

    // scan all pixels inside the target region
    for (i = 0; i < nSections; ++i) {
        for (y = section[i].y0; y <= section[i].y1; ++y) {
            xa = imgCoordMungeLowerC(section[i].xa0 + ((SplashCoord)y + 0.5 - section[i].ya0) * section[i].dxdya, glyphMode);
            xb = imgCoordMungeUpperC(section[i].xb0 + ((SplashCoord)y + 0.5 - section[i].yb0) * section[i].dxdyb, glyphMode);
            if (unlikely(xa < 0)) {
                xa = 0;
            }
            // make sure narrow images cover at least one pixel
            if (xa == xb) {
                ++xb;
            }
            if (clipRes != splashClipAllInside) {
                clipRes2 = state->clip->testSpan(xa, xb - 1, y);
            } else {
                clipRes2 = clipRes;
            }
            for (x = xa; x < xb; ++x) {
                // map (x+0.5, y+0.5) back to the scaled image
                xx = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir00 + ((SplashCoord)y + 0.5 - mat[5]) * ir10);
                yy = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir01 + ((SplashCoord)y + 0.5 - mat[5]) * ir11);
                // xx should always be within bounds, but floating point
                // inaccuracy can cause problems
                if (unlikely(xx < 0)) {
                    xx = 0;
                    clipRes2 = splashClipPartial;
                } else if (unlikely(xx >= scaledWidth)) {
                    xx = scaledWidth - 1;
                    clipRes2 = splashClipPartial;
                }
                if (unlikely(yy < 0)) {
                    yy = 0;
                    clipRes2 = splashClipPartial;
                } else if (unlikely(yy >= scaledHeight)) {
                    yy = scaledHeight - 1;
                    clipRes2 = splashClipPartial;
                }
                pipe.shape = scaledMask->data[yy * scaledWidth + xx];
                if (vectorAntialias && clipRes2 != splashClipAllInside) {
                    drawAAPixel(&pipe, x, y);
                } else {
                    drawPixel(&pipe, x, y, clipRes2 == splashClipAllInside);
                }
            }
        }
    }

    delete scaledMask;
}

// Scale an image mask into a SplashBitmap.
SplashBitmap *Splash::scaleMask(SplashImageMaskSource src, void *srcData, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight)
{
    SplashBitmap *dest;

    dest = new SplashBitmap(scaledWidth, scaledHeight, 1, splashModeMono8, false);
    if (scaledHeight < srcHeight) {
        if (scaledWidth < srcWidth) {
            scaleMaskYdownXdown(src, srcData, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
        } else {
            scaleMaskYdownXup(src, srcData, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
        }
    } else {
        if (scaledWidth < srcWidth) {
            scaleMaskYupXdown(src, srcData, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
        } else {
            scaleMaskYupXup(src, srcData, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
        }
    }
    return dest;
}

void Splash::scaleMaskYdownXdown(SplashImageMaskSource src, void *srcData, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf;
    unsigned int *pixBuf;
    unsigned int pix;
    unsigned char *destPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, d, d0, d1;
    int i, j;

    // Bresenham parameters for y scale
    yp = srcHeight / scaledHeight;
    yq = srcHeight % scaledHeight;

    // Bresenham parameters for x scale
    xp = srcWidth / scaledWidth;
    xq = srcWidth % scaledWidth;

    // allocate buffers
    lineBuf = (unsigned char *)gmalloc(srcWidth);
    pixBuf = (unsigned int *)gmallocn_checkoverflow(srcWidth, sizeof(int));
    if (unlikely(!pixBuf)) {
        error(errInternal, -1, "Couldn't allocate memory for pixBux in Splash::scaleMaskYdownXdown");
        gfree(lineBuf);
        return;
    }

    // init y scale Bresenham
    yt = 0;

    destPtr = dest->data;
    for (y = 0; y < scaledHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= scaledHeight) {
            yt -= scaledHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read rows from image
        memset(pixBuf, 0, srcWidth * sizeof(int));
        for (i = 0; i < yStep; ++i) {
            (*src)(srcData, lineBuf);
            for (j = 0; j < srcWidth; ++j) {
                pixBuf[j] += lineBuf[j];
            }
        }

        // init x scale Bresenham
        xt = 0;
        d0 = (255 << 23) / (yStep * xp);
        d1 = (255 << 23) / (yStep * (xp + 1));

        xx = 0;
        for (x = 0; x < scaledWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= scaledWidth) {
                xt -= scaledWidth;
                xStep = xp + 1;
                d = d1;
            } else {
                xStep = xp;
                d = d0;
            }

            // compute the final pixel
            pix = 0;
            for (i = 0; i < xStep; ++i) {
                pix += pixBuf[xx++];
            }
            // (255 * pix) / xStep * yStep
            pix = (pix * d) >> 23;

            // store the pixel
            *destPtr++ = (unsigned char)pix;
        }
    }

    gfree(pixBuf);
    gfree(lineBuf);
}

void Splash::scaleMaskYdownXup(SplashImageMaskSource src, void *srcData, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf;
    unsigned int *pixBuf;
    unsigned int pix;
    unsigned char *destPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, d;
    int i, j;

    destPtr = dest->data;
    if (destPtr == nullptr) {
        error(errInternal, -1, "dest->data is NULL in Splash::scaleMaskYdownXup");
        return;
    }

    // Bresenham parameters for y scale
    yp = srcHeight / scaledHeight;
    yq = srcHeight % scaledHeight;

    // Bresenham parameters for x scale
    xp = scaledWidth / srcWidth;
    xq = scaledWidth % srcWidth;

    // allocate buffers
    lineBuf = (unsigned char *)gmalloc(srcWidth);
    pixBuf = (unsigned int *)gmallocn(srcWidth, sizeof(int));

    // init y scale Bresenham
    yt = 0;

    for (y = 0; y < scaledHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= scaledHeight) {
            yt -= scaledHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read rows from image
        memset(pixBuf, 0, srcWidth * sizeof(int));
        for (i = 0; i < yStep; ++i) {
            (*src)(srcData, lineBuf);
            for (j = 0; j < srcWidth; ++j) {
                pixBuf[j] += lineBuf[j];
            }
        }

        // init x scale Bresenham
        xt = 0;
        d = (255 << 23) / yStep;

        for (x = 0; x < srcWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= srcWidth) {
                xt -= srcWidth;
                xStep = xp + 1;
            } else {
                xStep = xp;
            }

            // compute the final pixel
            pix = pixBuf[x];
            // (255 * pix) / yStep
            pix = (pix * d) >> 23;

            // store the pixel
            for (i = 0; i < xStep; ++i) {
                *destPtr++ = (unsigned char)pix;
            }
        }
    }

    gfree(pixBuf);
    gfree(lineBuf);
}

void Splash::scaleMaskYupXdown(SplashImageMaskSource src, void *srcData, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf;
    unsigned int pix;
    unsigned char *destPtr0, *destPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, d, d0, d1;
    int i;

    destPtr0 = dest->data;
    if (destPtr0 == nullptr) {
        error(errInternal, -1, "dest->data is NULL in Splash::scaleMaskYupXdown");
        return;
    }

    // Bresenham parameters for y scale
    yp = scaledHeight / srcHeight;
    yq = scaledHeight % srcHeight;

    // Bresenham parameters for x scale
    xp = srcWidth / scaledWidth;
    xq = srcWidth % scaledWidth;

    // allocate buffers
    lineBuf = (unsigned char *)gmalloc(srcWidth);

    // init y scale Bresenham
    yt = 0;

    for (y = 0; y < srcHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= srcHeight) {
            yt -= srcHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read row from image
        (*src)(srcData, lineBuf);

        // init x scale Bresenham
        xt = 0;
        d0 = (255 << 23) / xp;
        d1 = (255 << 23) / (xp + 1);

        xx = 0;
        for (x = 0; x < scaledWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= scaledWidth) {
                xt -= scaledWidth;
                xStep = xp + 1;
                d = d1;
            } else {
                xStep = xp;
                d = d0;
            }

            // compute the final pixel
            pix = 0;
            for (i = 0; i < xStep; ++i) {
                pix += lineBuf[xx++];
            }
            // (255 * pix) / xStep
            pix = (pix * d) >> 23;

            // store the pixel
            for (i = 0; i < yStep; ++i) {
                destPtr = destPtr0 + i * scaledWidth + x;
                *destPtr = (unsigned char)pix;
            }
        }

        destPtr0 += yStep * scaledWidth;
    }

    gfree(lineBuf);
}

void Splash::scaleMaskYupXup(SplashImageMaskSource src, void *srcData, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf;
    unsigned int pix;
    unsigned char *destPtr0, *destPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx;
    int i, j;

    destPtr0 = dest->data;
    if (destPtr0 == nullptr) {
        error(errInternal, -1, "dest->data is NULL in Splash::scaleMaskYupXup");
        return;
    }

    if (unlikely(srcWidth <= 0 || srcHeight <= 0)) {
        error(errSyntaxError, -1, "srcWidth <= 0 || srcHeight <= 0 in Splash::scaleMaskYupXup");
        gfree(dest->takeData());
        return;
    }

    // Bresenham parameters for y scale
    yp = scaledHeight / srcHeight;
    yq = scaledHeight % srcHeight;

    // Bresenham parameters for x scale
    xp = scaledWidth / srcWidth;
    xq = scaledWidth % srcWidth;

    // allocate buffers
    lineBuf = (unsigned char *)gmalloc(srcWidth);

    // init y scale Bresenham
    yt = 0;

    for (y = 0; y < srcHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= srcHeight) {
            yt -= srcHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read row from image
        (*src)(srcData, lineBuf);

        // init x scale Bresenham
        xt = 0;

        xx = 0;
        for (x = 0; x < srcWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= srcWidth) {
                xt -= srcWidth;
                xStep = xp + 1;
            } else {
                xStep = xp;
            }

            // compute the final pixel
            pix = lineBuf[x] ? 255 : 0;

            // store the pixel
            for (i = 0; i < yStep; ++i) {
                for (j = 0; j < xStep; ++j) {
                    destPtr = destPtr0 + i * scaledWidth + xx + j;
                    *destPtr++ = (unsigned char)pix;
                }
            }

            xx += xStep;
        }

        destPtr0 += yStep * scaledWidth;
    }

    gfree(lineBuf);
}

void Splash::blitMask(SplashBitmap *src, int xDest, int yDest, SplashClipResult clipRes)
{
    SplashPipe pipe;
    unsigned char *p;
    int w, h, x, y;

    w = src->getWidth();
    h = src->getHeight();
    p = src->getDataPtr();
    if (p == nullptr) {
        error(errInternal, -1, "src->getDataPtr() is NULL in Splash::blitMask");
        return;
    }
    if (vectorAntialias && clipRes != splashClipAllInside) {
        pipeInit(&pipe, xDest, yDest, state->fillPattern, nullptr, (unsigned char)splashRound(state->fillAlpha * 255), true, false);
        drawAAPixelInit();
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                pipe.shape = *p++;
                drawAAPixel(&pipe, xDest + x, yDest + y);
            }
        }
    } else {
        pipeInit(&pipe, xDest, yDest, state->fillPattern, nullptr, (unsigned char)splashRound(state->fillAlpha * 255), true, false);
        if (clipRes == splashClipAllInside) {
            for (y = 0; y < h; ++y) {
                pipeSetXY(&pipe, xDest, yDest + y);
                for (x = 0; x < w; ++x) {
                    if (*p) {
                        pipe.shape = *p;
                        (this->*pipe.run)(&pipe);
                    } else {
                        pipeIncX(&pipe);
                    }
                    ++p;
                }
            }
        } else {
            for (y = 0; y < h; ++y) {
                pipeSetXY(&pipe, xDest, yDest + y);
                for (x = 0; x < w; ++x) {
                    if (*p && state->clip->test(xDest + x, yDest + y)) {
                        pipe.shape = *p;
                        (this->*pipe.run)(&pipe);
                    } else {
                        pipeIncX(&pipe);
                    }
                    ++p;
                }
            }
        }
    }
}

SplashError Splash::drawImage(SplashImageSource src, SplashICCTransform tf, void *srcData, SplashColorMode srcMode, bool srcAlpha, int w, int h, SplashCoord *mat, bool interpolate, bool tilingPattern)
{
    bool ok;
    SplashBitmap *scaledImg;
    SplashClipResult clipRes;
    bool minorAxisZero;
    int x0, y0, x1, y1, scaledWidth, scaledHeight;
    int nComps;
    int yp;

    if (debugMode) {
        printf("drawImage: srcMode=%d srcAlpha=%d w=%d h=%d mat=[%.2f %.2f %.2f %.2f %.2f %.2f]\n", srcMode, srcAlpha, w, h, (double)mat[0], (double)mat[1], (double)mat[2], (double)mat[3], (double)mat[4], (double)mat[5]);
    }

    // check color modes
    ok = false; // make gcc happy
    nComps = 0; // make gcc happy
    switch (bitmap->mode) {
    case splashModeMono1:
    case splashModeMono8:
        ok = srcMode == splashModeMono8;
        nComps = 1;
        break;
    case splashModeRGB8:
        ok = srcMode == splashModeRGB8;
        nComps = 3;
        break;
    case splashModeXBGR8:
        ok = srcMode == splashModeXBGR8;
        nComps = 4;
        break;
    case splashModeBGR8:
        ok = srcMode == splashModeBGR8;
        nComps = 3;
        break;
    case splashModeCMYK8:
        ok = srcMode == splashModeCMYK8;
        nComps = 4;
        break;
    case splashModeDeviceN8:
        ok = srcMode == splashModeDeviceN8;
        nComps = SPOT_NCOMPS + 4;
        break;
    default:
        ok = false;
        break;
    }
    if (!ok) {
        return splashErrModeMismatch;
    }

    // check for singular matrix
    if (!splashCheckDet(mat[0], mat[1], mat[2], mat[3], 0.000001)) {
        return splashErrSingularMatrix;
    }

    minorAxisZero = mat[1] == 0 && mat[2] == 0;

    // scaling only
    if (mat[0] > 0 && minorAxisZero && mat[3] > 0) {
        x0 = imgCoordMungeLower(mat[4]);
        y0 = imgCoordMungeLower(mat[5]);
        x1 = imgCoordMungeUpper(mat[0] + mat[4]);
        y1 = imgCoordMungeUpper(mat[3] + mat[5]);
        // make sure narrow images cover at least one pixel
        if (x0 == x1) {
            ++x1;
        }
        if (y0 == y1) {
            ++y1;
        }
        clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
        opClipRes = clipRes;
        if (clipRes != splashClipAllOutside) {
            scaledWidth = x1 - x0;
            scaledHeight = y1 - y0;
            yp = h / scaledHeight;
            if (yp < 0 || yp > INT_MAX - 1) {
                return splashErrBadArg;
            }
            scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h, scaledWidth, scaledHeight, interpolate, tilingPattern);
            if (scaledImg == nullptr) {
                return splashErrBadArg;
            }
            if (tf != nullptr) {
                (*tf)(srcData, scaledImg);
            }
            blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
            delete scaledImg;
        }

        // scaling plus vertical flip
    } else if (mat[0] > 0 && minorAxisZero && mat[3] < 0) {
        x0 = imgCoordMungeLower(mat[4]);
        y0 = imgCoordMungeLower(mat[3] + mat[5]);
        x1 = imgCoordMungeUpper(mat[0] + mat[4]);
        y1 = imgCoordMungeUpper(mat[5]);
        if (x0 == x1) {
            if (mat[4] + mat[0] * 0.5 < x0) {
                --x0;
            } else {
                ++x1;
            }
        }
        if (y0 == y1) {
            if (mat[5] + mat[1] * 0.5 < y0) {
                --y0;
            } else {
                ++y1;
            }
        }
        clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
        opClipRes = clipRes;
        if (clipRes != splashClipAllOutside) {
            scaledWidth = x1 - x0;
            scaledHeight = y1 - y0;
            yp = h / scaledHeight;
            if (yp < 0 || yp > INT_MAX - 1) {
                return splashErrBadArg;
            }
            scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h, scaledWidth, scaledHeight, interpolate, tilingPattern);
            if (scaledImg == nullptr) {
                return splashErrBadArg;
            }
            if (tf != nullptr) {
                (*tf)(srcData, scaledImg);
            }
            vertFlipImage(scaledImg, scaledWidth, scaledHeight, nComps);
            blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
            delete scaledImg;
        }

        // all other cases
    } else {
        return arbitraryTransformImage(src, tf, srcData, srcMode, nComps, srcAlpha, w, h, mat, interpolate, tilingPattern);
    }

    return splashOk;
}

SplashError Splash::arbitraryTransformImage(SplashImageSource src, SplashICCTransform tf, void *srcData, SplashColorMode srcMode, int nComps, bool srcAlpha, int srcWidth, int srcHeight, SplashCoord *mat, bool interpolate,
                                            bool tilingPattern)
{
    SplashBitmap *scaledImg;
    SplashClipResult clipRes, clipRes2;
    SplashPipe pipe;
    SplashColor pixel = {};
    int scaledWidth, scaledHeight, t0, t1, th;
    SplashCoord r00, r01, r10, r11, det, ir00, ir01, ir10, ir11;
    SplashCoord vx[4], vy[4];
    int xMin, yMin, xMax, yMax;
    ImageSection section[3];
    int nSections;
    int y, xa, xb, x, i, xx, yy, yp;

    // compute the four vertices of the target quadrilateral
    vx[0] = mat[4];
    vy[0] = mat[5];
    vx[1] = mat[2] + mat[4];
    vy[1] = mat[3] + mat[5];
    vx[2] = mat[0] + mat[2] + mat[4];
    vy[2] = mat[1] + mat[3] + mat[5];
    vx[3] = mat[0] + mat[4];
    vy[3] = mat[1] + mat[5];

    // clipping
    xMin = imgCoordMungeLower(vx[0]);
    xMax = imgCoordMungeUpper(vx[0]);
    yMin = imgCoordMungeLower(vy[0]);
    yMax = imgCoordMungeUpper(vy[0]);
    for (i = 1; i < 4; ++i) {
        t0 = imgCoordMungeLower(vx[i]);
        if (t0 < xMin) {
            xMin = t0;
        }
        t0 = imgCoordMungeUpper(vx[i]);
        if (t0 > xMax) {
            xMax = t0;
        }
        t1 = imgCoordMungeLower(vy[i]);
        if (t1 < yMin) {
            yMin = t1;
        }
        t1 = imgCoordMungeUpper(vy[i]);
        if (t1 > yMax) {
            yMax = t1;
        }
    }
    clipRes = state->clip->testRect(xMin, yMin, xMax, yMax);
    opClipRes = clipRes;
    if (clipRes == splashClipAllOutside) {
        return splashOk;
    }

    // compute the scale factors
    if (splashAbs(mat[0]) >= splashAbs(mat[1])) {
        scaledWidth = xMax - xMin;
        scaledHeight = yMax - yMin;
    } else {
        scaledWidth = yMax - yMin;
        scaledHeight = xMax - xMin;
    }
    if (scaledHeight <= 1 || scaledWidth <= 1 || tilingPattern) {
        if (mat[0] >= 0) {
            t0 = imgCoordMungeUpper(mat[0] + mat[4]) - imgCoordMungeLower(mat[4]);
        } else {
            t0 = imgCoordMungeUpper(mat[4]) - imgCoordMungeLower(mat[0] + mat[4]);
        }
        if (mat[1] >= 0) {
            t1 = imgCoordMungeUpper(mat[1] + mat[5]) - imgCoordMungeLower(mat[5]);
        } else {
            t1 = imgCoordMungeUpper(mat[5]) - imgCoordMungeLower(mat[1] + mat[5]);
        }
        scaledWidth = t0 > t1 ? t0 : t1;
        if (mat[2] >= 0) {
            t0 = imgCoordMungeUpper(mat[2] + mat[4]) - imgCoordMungeLower(mat[4]);
            if (splashAbs(mat[1]) >= 1) {
                th = imgCoordMungeUpper(mat[2]) - imgCoordMungeLower(mat[0] * mat[3] / mat[1]);
                if (th > t0) {
                    t0 = th;
                }
            }
        } else {
            t0 = imgCoordMungeUpper(mat[4]) - imgCoordMungeLower(mat[2] + mat[4]);
            if (splashAbs(mat[1]) >= 1) {
                th = imgCoordMungeUpper(mat[0] * mat[3] / mat[1]) - imgCoordMungeLower(mat[2]);
                if (th > t0) {
                    t0 = th;
                }
            }
        }
        if (mat[3] >= 0) {
            t1 = imgCoordMungeUpper(mat[3] + mat[5]) - imgCoordMungeLower(mat[5]);
            if (splashAbs(mat[0]) >= 1) {
                th = imgCoordMungeUpper(mat[3]) - imgCoordMungeLower(mat[1] * mat[2] / mat[0]);
                if (th > t1) {
                    t1 = th;
                }
            }
        } else {
            t1 = imgCoordMungeUpper(mat[5]) - imgCoordMungeLower(mat[3] + mat[5]);
            if (splashAbs(mat[0]) >= 1) {
                th = imgCoordMungeUpper(mat[1] * mat[2] / mat[0]) - imgCoordMungeLower(mat[3]);
                if (th > t1) {
                    t1 = th;
                }
            }
        }
        scaledHeight = t0 > t1 ? t0 : t1;
    }
    if (scaledWidth == 0) {
        scaledWidth = 1;
    }
    if (scaledHeight == 0) {
        scaledHeight = 1;
    }

    // compute the inverse transform (after scaling) matrix
    r00 = mat[0] / scaledWidth;
    r01 = mat[1] / scaledWidth;
    r10 = mat[2] / scaledHeight;
    r11 = mat[3] / scaledHeight;
    det = r00 * r11 - r01 * r10;
    if (splashAbs(det) < 1e-6) {
        // this should be caught by the singular matrix check in drawImage
        return splashErrBadArg;
    }
    ir00 = r11 / det;
    ir01 = -r01 / det;
    ir10 = -r10 / det;
    ir11 = r00 / det;

    // scale the input image
    yp = srcHeight / scaledHeight;
    if (yp < 0 || yp > INT_MAX - 1) {
        return splashErrBadArg;
    }
    scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, srcWidth, srcHeight, scaledWidth, scaledHeight, interpolate);

    if (scaledImg == nullptr) {
        return splashErrBadArg;
    }

    if (tf != nullptr) {
        (*tf)(srcData, scaledImg);
    }
    // construct the three sections
    i = 0;
    if (vy[1] < vy[i]) {
        i = 1;
    }
    if (vy[2] < vy[i]) {
        i = 2;
    }
    if (vy[3] < vy[i]) {
        i = 3;
    }
    // NB: if using fixed point, 0.000001 will be truncated to zero,
    // so these two comparisons must be <=, not <
    if (splashAbs(vy[i] - vy[(i - 1) & 3]) <= 0.000001 && vy[(i - 1) & 3] < vy[(i + 1) & 3]) {
        i = (i - 1) & 3;
    }
    if (splashAbs(vy[i] - vy[(i + 1) & 3]) <= 0.000001) {
        section[0].y0 = imgCoordMungeLower(vy[i]);
        section[0].y1 = imgCoordMungeUpper(vy[(i + 2) & 3]) - 1;
        if (vx[i] < vx[(i + 1) & 3]) {
            section[0].ia0 = i;
            section[0].ia1 = (i + 3) & 3;
            section[0].ib0 = (i + 1) & 3;
            section[0].ib1 = (i + 2) & 3;
        } else {
            section[0].ia0 = (i + 1) & 3;
            section[0].ia1 = (i + 2) & 3;
            section[0].ib0 = i;
            section[0].ib1 = (i + 3) & 3;
        }
        nSections = 1;
    } else {
        section[0].y0 = imgCoordMungeLower(vy[i]);
        section[2].y1 = imgCoordMungeUpper(vy[(i + 2) & 3]) - 1;
        section[0].ia0 = section[0].ib0 = i;
        section[2].ia1 = section[2].ib1 = (i + 2) & 3;
        if (vx[(i + 1) & 3] < vx[(i + 3) & 3]) {
            section[0].ia1 = section[2].ia0 = (i + 1) & 3;
            section[0].ib1 = section[2].ib0 = (i + 3) & 3;
        } else {
            section[0].ia1 = section[2].ia0 = (i + 3) & 3;
            section[0].ib1 = section[2].ib0 = (i + 1) & 3;
        }
        if (vy[(i + 1) & 3] < vy[(i + 3) & 3]) {
            section[1].y0 = imgCoordMungeLower(vy[(i + 1) & 3]);
            section[2].y0 = imgCoordMungeUpper(vy[(i + 3) & 3]);
            if (vx[(i + 1) & 3] < vx[(i + 3) & 3]) {
                section[1].ia0 = (i + 1) & 3;
                section[1].ia1 = (i + 2) & 3;
                section[1].ib0 = i;
                section[1].ib1 = (i + 3) & 3;
            } else {
                section[1].ia0 = i;
                section[1].ia1 = (i + 3) & 3;
                section[1].ib0 = (i + 1) & 3;
                section[1].ib1 = (i + 2) & 3;
            }
        } else {
            section[1].y0 = imgCoordMungeLower(vy[(i + 3) & 3]);
            section[2].y0 = imgCoordMungeUpper(vy[(i + 1) & 3]);
            if (vx[(i + 1) & 3] < vx[(i + 3) & 3]) {
                section[1].ia0 = i;
                section[1].ia1 = (i + 1) & 3;
                section[1].ib0 = (i + 3) & 3;
                section[1].ib1 = (i + 2) & 3;
            } else {
                section[1].ia0 = (i + 3) & 3;
                section[1].ia1 = (i + 2) & 3;
                section[1].ib0 = i;
                section[1].ib1 = (i + 1) & 3;
            }
        }
        section[0].y1 = section[1].y0 - 1;
        section[1].y1 = section[2].y0 - 1;
        nSections = 3;
    }
    for (i = 0; i < nSections; ++i) {
        section[i].xa0 = vx[section[i].ia0];
        section[i].ya0 = vy[section[i].ia0];
        section[i].xa1 = vx[section[i].ia1];
        section[i].ya1 = vy[section[i].ia1];
        section[i].xb0 = vx[section[i].ib0];
        section[i].yb0 = vy[section[i].ib0];
        section[i].xb1 = vx[section[i].ib1];
        section[i].yb1 = vy[section[i].ib1];
        section[i].dxdya = (section[i].xa1 - section[i].xa0) / (section[i].ya1 - section[i].ya0);
        section[i].dxdyb = (section[i].xb1 - section[i].xb0) / (section[i].yb1 - section[i].yb0);
    }

    // initialize the pixel pipe
    pipeInit(&pipe, 0, 0, nullptr, pixel, (unsigned char)splashRound(state->fillAlpha * 255), srcAlpha || (vectorAntialias && clipRes != splashClipAllInside), false);
    if (vectorAntialias) {
        drawAAPixelInit();
    }

    // make sure narrow images cover at least one pixel
    if (nSections == 1) {
        if (section[0].y0 == section[0].y1) {
            ++section[0].y1;
            clipRes = opClipRes = splashClipPartial;
        }
    } else {
        if (section[0].y0 == section[2].y1) {
            ++section[1].y1;
            clipRes = opClipRes = splashClipPartial;
        }
    }

    // scan all pixels inside the target region
    for (i = 0; i < nSections; ++i) {
        for (y = section[i].y0; y <= section[i].y1; ++y) {
            xa = imgCoordMungeLower(section[i].xa0 + ((SplashCoord)y + 0.5 - section[i].ya0) * section[i].dxdya);
            if (unlikely(xa < 0)) {
                xa = 0;
            }
            xb = imgCoordMungeUpper(section[i].xb0 + ((SplashCoord)y + 0.5 - section[i].yb0) * section[i].dxdyb);
            // make sure narrow images cover at least one pixel
            if (xa == xb) {
                ++xb;
            }
            if (unlikely(clipRes == splashClipAllInside && xb > bitmap->getWidth())) {
                xb = bitmap->getWidth();
            }
            if (clipRes != splashClipAllInside) {
                clipRes2 = state->clip->testSpan(xa, xb - 1, y);
            } else {
                clipRes2 = clipRes;
            }
            for (x = xa; x < xb; ++x) {
                // map (x+0.5, y+0.5) back to the scaled image
                xx = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir00 + ((SplashCoord)y + 0.5 - mat[5]) * ir10);
                yy = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir01 + ((SplashCoord)y + 0.5 - mat[5]) * ir11);
                // xx should always be within bounds, but floating point
                // inaccuracy can cause problems
                if (xx < 0) {
                    xx = 0;
                } else if (xx >= scaledWidth) {
                    xx = scaledWidth - 1;
                }
                if (yy < 0) {
                    yy = 0;
                } else if (yy >= scaledHeight) {
                    yy = scaledHeight - 1;
                }
                scaledImg->getPixel(xx, yy, pixel);
                if (srcAlpha) {
                    pipe.shape = scaledImg->alpha[yy * scaledWidth + xx];
                } else {
                    pipe.shape = 255;
                }
                if (vectorAntialias && clipRes2 != splashClipAllInside) {
                    drawAAPixel(&pipe, x, y);
                } else {
                    drawPixel(&pipe, x, y, clipRes2 == splashClipAllInside);
                }
            }
        }
    }

    delete scaledImg;
    return splashOk;
}

// determine if a scaled image requires interpolation based on the scale and
// the interpolate flag from the image dictionary
static bool isImageInterpolationRequired(int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, bool interpolate)
{
    if (interpolate || srcWidth == 0 || srcHeight == 0) {
        return true;
    }

    /* When scale factor is >= 400% we don't interpolate. See bugs #25268, #9860 */
    if (scaledWidth / srcWidth >= 4 || scaledHeight / srcHeight >= 4) {
        return false;
    }

    return true;
}

// Scale an image into a SplashBitmap.
SplashBitmap *Splash::scaleImage(SplashImageSource src, void *srcData, SplashColorMode srcMode, int nComps, bool srcAlpha, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, bool interpolate, bool tilingPattern)
{
    SplashBitmap *dest;

    dest = new SplashBitmap(scaledWidth, scaledHeight, 1, srcMode, srcAlpha, true, bitmap->getSeparationList());
    if (dest->getDataPtr() != nullptr && srcHeight > 0 && srcWidth > 0) {
        bool success = true;
        if (scaledHeight < srcHeight) {
            if (scaledWidth < srcWidth) {
                success = scaleImageYdownXdown(src, srcData, srcMode, nComps, srcAlpha, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
            } else {
                success = scaleImageYdownXup(src, srcData, srcMode, nComps, srcAlpha, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
            }
        } else {
            if (scaledWidth < srcWidth) {
                success = scaleImageYupXdown(src, srcData, srcMode, nComps, srcAlpha, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
            } else {
                if (!tilingPattern && isImageInterpolationRequired(srcWidth, srcHeight, scaledWidth, scaledHeight, interpolate)) {
                    success = scaleImageYupXupBilinear(src, srcData, srcMode, nComps, srcAlpha, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
                } else {
                    success = scaleImageYupXup(src, srcData, srcMode, nComps, srcAlpha, srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
                }
            }
        }
        if (unlikely(!success)) {
            delete dest;
            dest = nullptr;
        }
    } else {
        delete dest;
        dest = nullptr;
    }
    return dest;
}

bool Splash::scaleImageYdownXdown(SplashImageSource src, void *srcData, SplashColorMode srcMode, int nComps, bool srcAlpha, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf, *alphaLineBuf;
    unsigned int *pixBuf, *alphaPixBuf;
    unsigned int pix0, pix1, pix2;
    unsigned int pix3;
    unsigned int pix[SPOT_NCOMPS + 4], cp;
    unsigned int alpha;
    unsigned char *destPtr, *destAlphaPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, xxa, d, d0, d1;
    int i, j;

    // Bresenham parameters for y scale
    yp = srcHeight / scaledHeight;
    yq = srcHeight % scaledHeight;

    // Bresenham parameters for x scale
    xp = srcWidth / scaledWidth;
    xq = srcWidth % scaledWidth;

    // allocate buffers
    lineBuf = (unsigned char *)gmallocn_checkoverflow(srcWidth, nComps);
    if (unlikely(!lineBuf)) {
        return false;
    }
    pixBuf = (unsigned int *)gmallocn_checkoverflow(srcWidth, nComps * sizeof(int));
    if (unlikely(!pixBuf)) {
        gfree(lineBuf);
        return false;
    }
    if (srcAlpha) {
        alphaLineBuf = (unsigned char *)gmalloc(srcWidth);
        alphaPixBuf = (unsigned int *)gmallocn(srcWidth, sizeof(int));
    } else {
        alphaLineBuf = nullptr;
        alphaPixBuf = nullptr;
    }

    // init y scale Bresenham
    yt = 0;

    destPtr = dest->data;
    destAlphaPtr = dest->alpha;
    for (y = 0; y < scaledHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= scaledHeight) {
            yt -= scaledHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read rows from image
        memset(pixBuf, 0, srcWidth * nComps * sizeof(int));
        if (srcAlpha) {
            memset(alphaPixBuf, 0, srcWidth * sizeof(int));
        }
        for (i = 0; i < yStep; ++i) {
            (*src)(srcData, lineBuf, alphaLineBuf);
            for (j = 0; j < srcWidth * nComps; ++j) {
                pixBuf[j] += lineBuf[j];
            }
            if (srcAlpha) {
                for (j = 0; j < srcWidth; ++j) {
                    alphaPixBuf[j] += alphaLineBuf[j];
                }
            }
        }

        // init x scale Bresenham
        xt = 0;
        d0 = (1 << 23) / (yStep * xp);
        d1 = (1 << 23) / (yStep * (xp + 1));

        xx = xxa = 0;
        for (x = 0; x < scaledWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= scaledWidth) {
                xt -= scaledWidth;
                xStep = xp + 1;
                d = d1;
            } else {
                xStep = xp;
                d = d0;
            }

            switch (srcMode) {

            case splashModeMono8:

                // compute the final pixel
                pix0 = 0;
                for (i = 0; i < xStep; ++i) {
                    pix0 += pixBuf[xx++];
                }
                // pix / xStep * yStep
                pix0 = (pix0 * d) >> 23;

                // store the pixel
                *destPtr++ = (unsigned char)pix0;
                break;

            case splashModeRGB8:

                // compute the final pixel
                pix0 = pix1 = pix2 = 0;
                for (i = 0; i < xStep; ++i) {
                    pix0 += pixBuf[xx];
                    pix1 += pixBuf[xx + 1];
                    pix2 += pixBuf[xx + 2];
                    xx += 3;
                }
                // pix / xStep * yStep
                pix0 = (pix0 * d) >> 23;
                pix1 = (pix1 * d) >> 23;
                pix2 = (pix2 * d) >> 23;

                // store the pixel
                *destPtr++ = (unsigned char)pix0;
                *destPtr++ = (unsigned char)pix1;
                *destPtr++ = (unsigned char)pix2;
                break;

            case splashModeXBGR8:

                // compute the final pixel
                pix0 = pix1 = pix2 = 0;
                for (i = 0; i < xStep; ++i) {
                    pix0 += pixBuf[xx];
                    pix1 += pixBuf[xx + 1];
                    pix2 += pixBuf[xx + 2];
                    xx += 4;
                }
                // pix / xStep * yStep
                pix0 = (pix0 * d) >> 23;
                pix1 = (pix1 * d) >> 23;
                pix2 = (pix2 * d) >> 23;

                // store the pixel
                *destPtr++ = (unsigned char)pix2;
                *destPtr++ = (unsigned char)pix1;
                *destPtr++ = (unsigned char)pix0;
                *destPtr++ = (unsigned char)255;
                break;

            case splashModeBGR8:

                // compute the final pixel
                pix0 = pix1 = pix2 = 0;
                for (i = 0; i < xStep; ++i) {
                    pix0 += pixBuf[xx];
                    pix1 += pixBuf[xx + 1];
                    pix2 += pixBuf[xx + 2];
                    xx += 3;
                }
                // pix / xStep * yStep
                pix0 = (pix0 * d) >> 23;
                pix1 = (pix1 * d) >> 23;
                pix2 = (pix2 * d) >> 23;

                // store the pixel
                *destPtr++ = (unsigned char)pix2;
                *destPtr++ = (unsigned char)pix1;
                *destPtr++ = (unsigned char)pix0;
                break;

            case splashModeCMYK8:

                // compute the final pixel
                pix0 = pix1 = pix2 = pix3 = 0;
                for (i = 0; i < xStep; ++i) {
                    pix0 += pixBuf[xx];
                    pix1 += pixBuf[xx + 1];
                    pix2 += pixBuf[xx + 2];
                    pix3 += pixBuf[xx + 3];
                    xx += 4;
                }
                // pix / xStep * yStep
                pix0 = (pix0 * d) >> 23;
                pix1 = (pix1 * d) >> 23;
                pix2 = (pix2 * d) >> 23;
                pix3 = (pix3 * d) >> 23;

                // store the pixel
                *destPtr++ = (unsigned char)pix0;
                *destPtr++ = (unsigned char)pix1;
                *destPtr++ = (unsigned char)pix2;
                *destPtr++ = (unsigned char)pix3;
                break;
            case splashModeDeviceN8:

                // compute the final pixel
                for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    pix[cp] = 0;
                }
                for (i = 0; i < xStep; ++i) {
                    for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                        pix[cp] += pixBuf[xx + cp];
                    }
                    xx += (SPOT_NCOMPS + 4);
                }
                // pix / xStep * yStep
                for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    pix[cp] = (pix[cp] * d) >> 23;
                }

                // store the pixel
                for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    *destPtr++ = (unsigned char)pix[cp];
                }
                break;

            case splashModeMono1: // mono1 is not allowed
            default:
                break;
            }

            // process alpha
            if (srcAlpha) {
                alpha = 0;
                for (i = 0; i < xStep; ++i, ++xxa) {
                    alpha += alphaPixBuf[xxa];
                }
                // alpha / xStep * yStep
                alpha = (alpha * d) >> 23;
                *destAlphaPtr++ = (unsigned char)alpha;
            }
        }
    }

    gfree(alphaPixBuf);
    gfree(alphaLineBuf);
    gfree(pixBuf);
    gfree(lineBuf);

    return true;
}

bool Splash::scaleImageYdownXup(SplashImageSource src, void *srcData, SplashColorMode srcMode, int nComps, bool srcAlpha, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf, *alphaLineBuf;
    unsigned int *pixBuf, *alphaPixBuf;
    unsigned int pix[splashMaxColorComps];
    unsigned int alpha;
    unsigned char *destPtr, *destAlphaPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, d;
    int i, j;

    // Bresenham parameters for y scale
    yp = srcHeight / scaledHeight;
    yq = srcHeight % scaledHeight;

    // Bresenham parameters for x scale
    xp = scaledWidth / srcWidth;
    xq = scaledWidth % srcWidth;

    // allocate buffers
    pixBuf = (unsigned int *)gmallocn_checkoverflow(srcWidth, nComps * sizeof(int));
    if (unlikely(!pixBuf)) {
        error(errInternal, -1, "Splash::scaleImageYdownXup. Couldn't allocate pixBuf memory");
        return false;
    }
    lineBuf = (unsigned char *)gmallocn(srcWidth, nComps);
    if (srcAlpha) {
        alphaLineBuf = (unsigned char *)gmalloc(srcWidth);
        alphaPixBuf = (unsigned int *)gmallocn(srcWidth, sizeof(int));
    } else {
        alphaLineBuf = nullptr;
        alphaPixBuf = nullptr;
    }

    // init y scale Bresenham
    yt = 0;

    destPtr = dest->data;
    destAlphaPtr = dest->alpha;
    for (y = 0; y < scaledHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= scaledHeight) {
            yt -= scaledHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read rows from image
        memset(pixBuf, 0, srcWidth * nComps * sizeof(int));
        if (srcAlpha) {
            memset(alphaPixBuf, 0, srcWidth * sizeof(int));
        }
        for (i = 0; i < yStep; ++i) {
            (*src)(srcData, lineBuf, alphaLineBuf);
            for (j = 0; j < srcWidth * nComps; ++j) {
                pixBuf[j] += lineBuf[j];
            }
            if (srcAlpha) {
                for (j = 0; j < srcWidth; ++j) {
                    alphaPixBuf[j] += alphaLineBuf[j];
                }
            }
        }

        // init x scale Bresenham
        xt = 0;
        d = (1 << 23) / yStep;

        for (x = 0; x < srcWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= srcWidth) {
                xt -= srcWidth;
                xStep = xp + 1;
            } else {
                xStep = xp;
            }

            // compute the final pixel
            for (i = 0; i < nComps; ++i) {
                // pixBuf[] / yStep
                pix[i] = (pixBuf[x * nComps + i] * d) >> 23;
            }

            // store the pixel
            switch (srcMode) {
            case splashModeMono1: // mono1 is not allowed
                break;
            case splashModeMono8:
                for (i = 0; i < xStep; ++i) {
                    *destPtr++ = (unsigned char)pix[0];
                }
                break;
            case splashModeRGB8:
                for (i = 0; i < xStep; ++i) {
                    *destPtr++ = (unsigned char)pix[0];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[2];
                }
                break;
            case splashModeXBGR8:
                for (i = 0; i < xStep; ++i) {
                    *destPtr++ = (unsigned char)pix[2];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[0];
                    *destPtr++ = (unsigned char)255;
                }
                break;
            case splashModeBGR8:
                for (i = 0; i < xStep; ++i) {
                    *destPtr++ = (unsigned char)pix[2];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[0];
                }
                break;
            case splashModeCMYK8:
                for (i = 0; i < xStep; ++i) {
                    *destPtr++ = (unsigned char)pix[0];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[2];
                    *destPtr++ = (unsigned char)pix[3];
                }
                break;
            case splashModeDeviceN8:
                for (i = 0; i < xStep; ++i) {
                    for (unsigned int cp : pix) {
                        *destPtr++ = (unsigned char)cp;
                    }
                }
                break;
            }

            // process alpha
            if (srcAlpha) {
                // alphaPixBuf[] / yStep
                alpha = (alphaPixBuf[x] * d) >> 23;
                for (i = 0; i < xStep; ++i) {
                    *destAlphaPtr++ = (unsigned char)alpha;
                }
            }
        }
    }

    gfree(alphaPixBuf);
    gfree(alphaLineBuf);
    gfree(pixBuf);
    gfree(lineBuf);

    return true;
}

bool Splash::scaleImageYupXdown(SplashImageSource src, void *srcData, SplashColorMode srcMode, int nComps, bool srcAlpha, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf, *alphaLineBuf;
    unsigned int pix[splashMaxColorComps];
    unsigned int alpha;
    unsigned char *destPtr0, *destPtr, *destAlphaPtr0, *destAlphaPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, xxa, d, d0, d1;
    int i, j;

    // Bresenham parameters for y scale
    yp = scaledHeight / srcHeight;
    yq = scaledHeight % srcHeight;

    // Bresenham parameters for x scale
    xp = srcWidth / scaledWidth;
    xq = srcWidth % scaledWidth;

    // allocate buffers
    lineBuf = (unsigned char *)gmallocn_checkoverflow(srcWidth, nComps);
    if (unlikely(!lineBuf)) {
        gfree(dest->takeData());
        return false;
    }
    if (srcAlpha) {
        alphaLineBuf = (unsigned char *)gmalloc(srcWidth);
    } else {
        alphaLineBuf = nullptr;
    }

    // init y scale Bresenham
    yt = 0;

    destPtr0 = dest->data;
    destAlphaPtr0 = dest->alpha;
    for (y = 0; y < srcHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= srcHeight) {
            yt -= srcHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read row from image
        (*src)(srcData, lineBuf, alphaLineBuf);

        // init x scale Bresenham
        xt = 0;
        d0 = (1 << 23) / xp;
        d1 = (1 << 23) / (xp + 1);

        xx = xxa = 0;
        for (x = 0; x < scaledWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= scaledWidth) {
                xt -= scaledWidth;
                xStep = xp + 1;
                d = d1;
            } else {
                xStep = xp;
                d = d0;
            }

            // compute the final pixel
            for (i = 0; i < nComps; ++i) {
                pix[i] = 0;
            }
            for (i = 0; i < xStep; ++i) {
                for (j = 0; j < nComps; ++j, ++xx) {
                    pix[j] += lineBuf[xx];
                }
            }
            for (i = 0; i < nComps; ++i) {
                // pix[] / xStep
                pix[i] = (pix[i] * d) >> 23;
            }

            // store the pixel
            switch (srcMode) {
            case splashModeMono1: // mono1 is not allowed
                break;
            case splashModeMono8:
                for (i = 0; i < yStep; ++i) {
                    destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
                    *destPtr++ = (unsigned char)pix[0];
                }
                break;
            case splashModeRGB8:
                for (i = 0; i < yStep; ++i) {
                    destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
                    *destPtr++ = (unsigned char)pix[0];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[2];
                }
                break;
            case splashModeXBGR8:
                for (i = 0; i < yStep; ++i) {
                    destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
                    *destPtr++ = (unsigned char)pix[2];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[0];
                    *destPtr++ = (unsigned char)255;
                }
                break;
            case splashModeBGR8:
                for (i = 0; i < yStep; ++i) {
                    destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
                    *destPtr++ = (unsigned char)pix[2];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[0];
                }
                break;
            case splashModeCMYK8:
                for (i = 0; i < yStep; ++i) {
                    destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
                    *destPtr++ = (unsigned char)pix[0];
                    *destPtr++ = (unsigned char)pix[1];
                    *destPtr++ = (unsigned char)pix[2];
                    *destPtr++ = (unsigned char)pix[3];
                }
                break;
            case splashModeDeviceN8:
                for (i = 0; i < yStep; ++i) {
                    destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
                    for (unsigned int cp : pix) {
                        *destPtr++ = (unsigned char)cp;
                    }
                }
                break;
            }

            // process alpha
            if (srcAlpha) {
                alpha = 0;
                for (i = 0; i < xStep; ++i, ++xxa) {
                    alpha += alphaLineBuf[xxa];
                }
                // alpha / xStep
                alpha = (alpha * d) >> 23;
                for (i = 0; i < yStep; ++i) {
                    destAlphaPtr = destAlphaPtr0 + i * scaledWidth + x;
                    *destAlphaPtr = (unsigned char)alpha;
                }
            }
        }

        destPtr0 += yStep * scaledWidth * nComps;
        if (srcAlpha) {
            destAlphaPtr0 += yStep * scaledWidth;
        }
    }

    gfree(alphaLineBuf);
    gfree(lineBuf);

    return true;
}

bool Splash::scaleImageYupXup(SplashImageSource src, void *srcData, SplashColorMode srcMode, int nComps, bool srcAlpha, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *lineBuf, *alphaLineBuf;
    unsigned int pix[splashMaxColorComps];
    unsigned int alpha;
    unsigned char *destPtr0, *destPtr, *destAlphaPtr0, *destAlphaPtr;
    int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx;
    int i, j;

    // Bresenham parameters for y scale
    yp = scaledHeight / srcHeight;
    yq = scaledHeight % srcHeight;

    // Bresenham parameters for x scale
    xp = scaledWidth / srcWidth;
    xq = scaledWidth % srcWidth;

    // allocate buffers
    lineBuf = (unsigned char *)gmallocn(srcWidth, nComps);
    if (srcAlpha) {
        alphaLineBuf = (unsigned char *)gmalloc(srcWidth);
    } else {
        alphaLineBuf = nullptr;
    }

    // init y scale Bresenham
    yt = 0;

    destPtr0 = dest->data;
    destAlphaPtr0 = dest->alpha;
    for (y = 0; y < srcHeight; ++y) {

        // y scale Bresenham
        if ((yt += yq) >= srcHeight) {
            yt -= srcHeight;
            yStep = yp + 1;
        } else {
            yStep = yp;
        }

        // read row from image
        (*src)(srcData, lineBuf, alphaLineBuf);

        // init x scale Bresenham
        xt = 0;

        xx = 0;
        for (x = 0; x < srcWidth; ++x) {

            // x scale Bresenham
            if ((xt += xq) >= srcWidth) {
                xt -= srcWidth;
                xStep = xp + 1;
            } else {
                xStep = xp;
            }

            // compute the final pixel
            for (i = 0; i < nComps; ++i) {
                pix[i] = lineBuf[x * nComps + i];
            }

            // store the pixel
            switch (srcMode) {
            case splashModeMono1: // mono1 is not allowed
                break;
            case splashModeMono8:
                for (i = 0; i < yStep; ++i) {
                    for (j = 0; j < xStep; ++j) {
                        destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
                        *destPtr++ = (unsigned char)pix[0];
                    }
                }
                break;
            case splashModeRGB8:
                for (i = 0; i < yStep; ++i) {
                    for (j = 0; j < xStep; ++j) {
                        destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
                        *destPtr++ = (unsigned char)pix[0];
                        *destPtr++ = (unsigned char)pix[1];
                        *destPtr++ = (unsigned char)pix[2];
                    }
                }
                break;
            case splashModeXBGR8:
                for (i = 0; i < yStep; ++i) {
                    for (j = 0; j < xStep; ++j) {
                        destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
                        *destPtr++ = (unsigned char)pix[2];
                        *destPtr++ = (unsigned char)pix[1];
                        *destPtr++ = (unsigned char)pix[0];
                        *destPtr++ = (unsigned char)255;
                    }
                }
                break;
            case splashModeBGR8:
                for (i = 0; i < yStep; ++i) {
                    for (j = 0; j < xStep; ++j) {
                        destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
                        *destPtr++ = (unsigned char)pix[2];
                        *destPtr++ = (unsigned char)pix[1];
                        *destPtr++ = (unsigned char)pix[0];
                    }
                }
                break;
            case splashModeCMYK8:
                for (i = 0; i < yStep; ++i) {
                    for (j = 0; j < xStep; ++j) {
                        destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
                        *destPtr++ = (unsigned char)pix[0];
                        *destPtr++ = (unsigned char)pix[1];
                        *destPtr++ = (unsigned char)pix[2];
                        *destPtr++ = (unsigned char)pix[3];
                    }
                }
                break;
            case splashModeDeviceN8:
                for (i = 0; i < yStep; ++i) {
                    for (j = 0; j < xStep; ++j) {
                        destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
                        for (unsigned int cp : pix) {
                            *destPtr++ = (unsigned char)cp;
                        }
                    }
                }
                break;
            }

            // process alpha
            if (srcAlpha) {
                alpha = alphaLineBuf[x];
                for (i = 0; i < yStep; ++i) {
                    for (j = 0; j < xStep; ++j) {
                        destAlphaPtr = destAlphaPtr0 + i * scaledWidth + xx + j;
                        *destAlphaPtr = (unsigned char)alpha;
                    }
                }
            }

            xx += xStep;
        }

        destPtr0 += yStep * scaledWidth * nComps;
        if (srcAlpha) {
            destAlphaPtr0 += yStep * scaledWidth;
        }
    }

    gfree(alphaLineBuf);
    gfree(lineBuf);

    return true;
}

// expand source row to scaledWidth using linear interpolation
static void expandRow(unsigned char *srcBuf, unsigned char *dstBuf, int srcWidth, int scaledWidth, int nComps)
{
    double xStep = (double)srcWidth / scaledWidth;
    double xSrc = 0.0;
    double xFrac, xInt;
    int p;

    // pad the source with an extra pixel equal to the last pixel
    // so that when xStep is inside the last pixel we still have two
    // pixels to interpolate between.
    for (int i = 0; i < nComps; i++) {
        srcBuf[srcWidth * nComps + i] = srcBuf[(srcWidth - 1) * nComps + i];
    }

    for (int x = 0; x < scaledWidth; x++) {
        xFrac = modf(xSrc, &xInt);
        p = (int)xInt;
        for (int c = 0; c < nComps; c++) {
            dstBuf[nComps * x + c] = static_cast<unsigned char>(srcBuf[nComps * p + c] * (1.0 - xFrac) + srcBuf[nComps * (p + 1) + c] * xFrac);
        }
        xSrc += xStep;
    }
}

// Scale up image using bilinear interpolation
bool Splash::scaleImageYupXupBilinear(SplashImageSource src, void *srcData, SplashColorMode srcMode, int nComps, bool srcAlpha, int srcWidth, int srcHeight, int scaledWidth, int scaledHeight, SplashBitmap *dest)
{
    unsigned char *srcBuf, *lineBuf1, *lineBuf2, *alphaSrcBuf, *alphaLineBuf1, *alphaLineBuf2;
    unsigned int pix[splashMaxColorComps];
    unsigned char *destPtr0, *destPtr, *destAlphaPtr0, *destAlphaPtr;
    int i;

    if (srcWidth < 1 || srcHeight < 1) {
        return false;
    }

    // allocate buffers
    srcBuf = (unsigned char *)gmallocn(srcWidth + 1, nComps); // + 1 pixel of padding
    lineBuf1 = (unsigned char *)gmallocn(scaledWidth, nComps);
    lineBuf2 = (unsigned char *)gmallocn(scaledWidth, nComps);
    if (srcAlpha) {
        alphaSrcBuf = (unsigned char *)gmalloc(srcWidth + 1); // + 1 pixel of padding
        alphaLineBuf1 = (unsigned char *)gmalloc(scaledWidth);
        alphaLineBuf2 = (unsigned char *)gmalloc(scaledWidth);
    } else {
        alphaSrcBuf = nullptr;
        alphaLineBuf1 = nullptr;
        alphaLineBuf2 = nullptr;
    }

    double ySrc = 0.0;
    double yStep = (double)srcHeight / scaledHeight;
    double yFrac, yInt;
    int currentSrcRow = -1;
    (*src)(srcData, srcBuf, alphaSrcBuf);
    expandRow(srcBuf, lineBuf2, srcWidth, scaledWidth, nComps);
    if (srcAlpha) {
        expandRow(alphaSrcBuf, alphaLineBuf2, srcWidth, scaledWidth, 1);
    }

    destPtr0 = dest->data;
    destAlphaPtr0 = dest->alpha;
    for (int y = 0; y < scaledHeight; y++) {
        yFrac = modf(ySrc, &yInt);
        if ((int)yInt > currentSrcRow) {
            currentSrcRow++;
            // Copy line2 data to line1 and get next line2 data.
            // If line2 already contains the last source row we don't touch it.
            // This effectively adds an extra row of padding for interpolating the
            // last source row with.
            memcpy(lineBuf1, lineBuf2, scaledWidth * nComps);
            if (srcAlpha) {
                memcpy(alphaLineBuf1, alphaLineBuf2, scaledWidth);
            }
            if (currentSrcRow < srcHeight - 1) {
                (*src)(srcData, srcBuf, alphaSrcBuf);
                expandRow(srcBuf, lineBuf2, srcWidth, scaledWidth, nComps);
                if (srcAlpha) {
                    expandRow(alphaSrcBuf, alphaLineBuf2, srcWidth, scaledWidth, 1);
                }
            }
        }

        // write row y using linear interpolation on lineBuf1 and lineBuf2
        for (int x = 0; x < scaledWidth; ++x) {
            // compute the final pixel
            for (i = 0; i < nComps; ++i) {
                pix[i] = static_cast<unsigned char>(lineBuf1[x * nComps + i] * (1.0 - yFrac) + lineBuf2[x * nComps + i] * yFrac);
            }

            // store the pixel
            destPtr = destPtr0 + (y * scaledWidth + x) * nComps;
            switch (srcMode) {
            case splashModeMono1: // mono1 is not allowed
                break;
            case splashModeMono8:
                *destPtr++ = (unsigned char)pix[0];
                break;
            case splashModeRGB8:
                *destPtr++ = (unsigned char)pix[0];
                *destPtr++ = (unsigned char)pix[1];
                *destPtr++ = (unsigned char)pix[2];
                break;
            case splashModeXBGR8:
                *destPtr++ = (unsigned char)pix[2];
                *destPtr++ = (unsigned char)pix[1];
                *destPtr++ = (unsigned char)pix[0];
                *destPtr++ = (unsigned char)255;
                break;
            case splashModeBGR8:
                *destPtr++ = (unsigned char)pix[2];
                *destPtr++ = (unsigned char)pix[1];
                *destPtr++ = (unsigned char)pix[0];
                break;
            case splashModeCMYK8:
                *destPtr++ = (unsigned char)pix[0];
                *destPtr++ = (unsigned char)pix[1];
                *destPtr++ = (unsigned char)pix[2];
                *destPtr++ = (unsigned char)pix[3];
                break;
            case splashModeDeviceN8:
                for (unsigned int cp : pix) {
                    *destPtr++ = (unsigned char)cp;
                }
                break;
            }

            // process alpha
            if (srcAlpha) {
                destAlphaPtr = destAlphaPtr0 + y * scaledWidth + x;
                *destAlphaPtr = static_cast<unsigned char>(alphaLineBuf1[x] * (1.0 - yFrac) + alphaLineBuf2[x] * yFrac);
            }
        }

        ySrc += yStep;
    }

    gfree(alphaSrcBuf);
    gfree(alphaLineBuf1);
    gfree(alphaLineBuf2);
    gfree(srcBuf);
    gfree(lineBuf1);
    gfree(lineBuf2);

    return true;
}

void Splash::vertFlipImage(SplashBitmap *img, int width, int height, int nComps)
{
    unsigned char *lineBuf;
    unsigned char *p0, *p1;
    int w;

    if (unlikely(img->data == nullptr)) {
        error(errInternal, -1, "img->data is NULL in Splash::vertFlipImage");
        return;
    }

    w = width * nComps;
    lineBuf = (unsigned char *)gmalloc(w);
    for (p0 = img->data, p1 = img->data + (height - 1) * w; p0 < p1; p0 += w, p1 -= w) {
        memcpy(lineBuf, p0, w);
        memcpy(p0, p1, w);
        memcpy(p1, lineBuf, w);
    }
    if (img->alpha) {
        for (p0 = img->alpha, p1 = img->alpha + (height - 1) * width; p0 < p1; p0 += width, p1 -= width) {
            memcpy(lineBuf, p0, width);
            memcpy(p0, p1, width);
            memcpy(p1, lineBuf, width);
        }
    }
    gfree(lineBuf);
}

void Splash::blitImage(SplashBitmap *src, bool srcAlpha, int xDest, int yDest)
{
    SplashClipResult clipRes = state->clip->testRect(xDest, yDest, xDest + src->getWidth() - 1, yDest + src->getHeight() - 1);
    if (clipRes != splashClipAllOutside) {
        blitImage(src, srcAlpha, xDest, yDest, clipRes);
    }
}

void Splash::blitImage(SplashBitmap *src, bool srcAlpha, int xDest, int yDest, SplashClipResult clipRes)
{
    SplashPipe pipe;
    SplashColor pixel = {};
    unsigned char *ap;
    int w, h, x0, y0, x1, y1, x, y;

    // split the image into clipped and unclipped regions
    w = src->getWidth();
    h = src->getHeight();
    if (clipRes == splashClipAllInside) {
        x0 = 0;
        y0 = 0;
        x1 = w;
        y1 = h;
    } else {
        if (state->clip->getNumPaths()) {
            x0 = x1 = w;
            y0 = y1 = h;
        } else {
            if ((x0 = splashCeil(state->clip->getXMin()) - xDest) < 0) {
                x0 = 0;
            }
            if ((y0 = splashCeil(state->clip->getYMin()) - yDest) < 0) {
                y0 = 0;
            }
            if ((x1 = splashFloor(state->clip->getXMax()) - xDest) > w) {
                x1 = w;
            }
            if (x1 < x0) {
                x1 = x0;
            }
            if ((y1 = splashFloor(state->clip->getYMax()) - yDest) > h) {
                y1 = h;
            }
            if (y1 < y0) {
                y1 = y0;
            }
        }
    }

    // draw the unclipped region
    if (x0 < w && y0 < h && x0 < x1 && y0 < y1) {
        pipeInit(&pipe, xDest + x0, yDest + y0, nullptr, pixel, (unsigned char)splashRound(state->fillAlpha * 255), srcAlpha, false);
        if (srcAlpha) {
            for (y = y0; y < y1; ++y) {
                pipeSetXY(&pipe, xDest + x0, yDest + y);
                ap = src->getAlphaPtr() + y * w + x0;
                for (x = x0; x < x1; ++x) {
                    src->getPixel(x, y, pixel);
                    pipe.shape = *ap++;
                    (this->*pipe.run)(&pipe);
                }
            }
        } else {
            for (y = y0; y < y1; ++y) {
                pipeSetXY(&pipe, xDest + x0, yDest + y);
                for (x = x0; x < x1; ++x) {
                    src->getPixel(x, y, pixel);
                    (this->*pipe.run)(&pipe);
                }
            }
        }
    }

    // draw the clipped regions
    if (y0 > 0) {
        blitImageClipped(src, srcAlpha, 0, 0, xDest, yDest, w, y0);
    }
    if (y1 < h) {
        blitImageClipped(src, srcAlpha, 0, y1, xDest, yDest + y1, w, h - y1);
    }
    if (x0 > 0 && y0 < y1) {
        blitImageClipped(src, srcAlpha, 0, y0, xDest, yDest + y0, x0, y1 - y0);
    }
    if (x1 < w && y0 < y1) {
        blitImageClipped(src, srcAlpha, x1, y0, xDest + x1, yDest + y0, w - x1, y1 - y0);
    }
}

void Splash::blitImageClipped(SplashBitmap *src, bool srcAlpha, int xSrc, int ySrc, int xDest, int yDest, int w, int h)
{
    SplashPipe pipe;
    SplashColor pixel = {};
    unsigned char *ap;
    int x, y;

    if (vectorAntialias) {
        pipeInit(&pipe, xDest, yDest, nullptr, pixel, (unsigned char)splashRound(state->fillAlpha * 255), true, false);
        drawAAPixelInit();
        if (srcAlpha) {
            for (y = 0; y < h; ++y) {
                ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
                for (x = 0; x < w; ++x) {
                    src->getPixel(xSrc + x, ySrc + y, pixel);
                    pipe.shape = *ap++;
                    drawAAPixel(&pipe, xDest + x, yDest + y);
                }
            }
        } else {
            for (y = 0; y < h; ++y) {
                for (x = 0; x < w; ++x) {
                    src->getPixel(xSrc + x, ySrc + y, pixel);
                    pipe.shape = 255;
                    drawAAPixel(&pipe, xDest + x, yDest + y);
                }
            }
        }
    } else {
        pipeInit(&pipe, xDest, yDest, nullptr, pixel, (unsigned char)splashRound(state->fillAlpha * 255), srcAlpha, false);
        if (srcAlpha) {
            for (y = 0; y < h; ++y) {
                ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
                pipeSetXY(&pipe, xDest, yDest + y);
                for (x = 0; x < w; ++x) {
                    if (state->clip->test(xDest + x, yDest + y)) {
                        src->getPixel(xSrc + x, ySrc + y, pixel);
                        pipe.shape = *ap++;
                        (this->*pipe.run)(&pipe);
                    } else {
                        pipeIncX(&pipe);
                        ++ap;
                    }
                }
            }
        } else {
            for (y = 0; y < h; ++y) {
                pipeSetXY(&pipe, xDest, yDest + y);
                for (x = 0; x < w; ++x) {
                    if (state->clip->test(xDest + x, yDest + y)) {
                        src->getPixel(xSrc + x, ySrc + y, pixel);
                        (this->*pipe.run)(&pipe);
                    } else {
                        pipeIncX(&pipe);
                    }
                }
            }
        }
    }
}

SplashError Splash::composite(SplashBitmap *src, int xSrc, int ySrc, int xDest, int yDest, int w, int h, bool noClip, bool nonIsolated, bool knockout, SplashCoord knockoutOpacity)
{
    SplashPipe pipe;
    SplashColor pixel;
    unsigned char alpha;
    unsigned char *ap;
    int x, y;

    if (src->mode != bitmap->mode) {
        return splashErrModeMismatch;
    }

    if (unlikely(!bitmap->data)) {
        return splashErrZeroImage;
    }

    if (src->getSeparationList()->size() > bitmap->getSeparationList()->size()) {
        for (x = bitmap->getSeparationList()->size(); x < (int)src->getSeparationList()->size(); x++) {
            bitmap->getSeparationList()->push_back((GfxSeparationColorSpace *)((*src->getSeparationList())[x])->copy());
        }
    }
    if (src->alpha) {
        pipeInit(&pipe, xDest, yDest, nullptr, pixel, (unsigned char)splashRound(state->fillAlpha * 255), true, nonIsolated, knockout, (unsigned char)splashRound(knockoutOpacity * 255));
        if (noClip) {
            for (y = 0; y < h; ++y) {
                pipeSetXY(&pipe, xDest, yDest + y);
                ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
                for (x = 0; x < w; ++x) {
                    src->getPixel(xSrc + x, ySrc + y, pixel);
                    alpha = *ap++;
                    // this uses shape instead of alpha, which isn't technically
                    // correct, but works out the same
                    pipe.shape = alpha;
                    (this->*pipe.run)(&pipe);
                }
            }
        } else {
            for (y = 0; y < h; ++y) {
                pipeSetXY(&pipe, xDest, yDest + y);
                ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
                for (x = 0; x < w; ++x) {
                    src->getPixel(xSrc + x, ySrc + y, pixel);
                    alpha = *ap++;
                    if (state->clip->test(xDest + x, yDest + y)) {
                        // this uses shape instead of alpha, which isn't technically
                        // correct, but works out the same
                        pipe.shape = alpha;
                        (this->*pipe.run)(&pipe);
                    } else {
                        pipeIncX(&pipe);
                    }
                }
            }
        }
    } else {
        pipeInit(&pipe, xDest, yDest, nullptr, pixel, (unsigned char)splashRound(state->fillAlpha * 255), false, nonIsolated);
        if (noClip) {
            for (y = 0; y < h; ++y) {
                pipeSetXY(&pipe, xDest, yDest + y);
                for (x = 0; x < w; ++x) {
                    src->getPixel(xSrc + x, ySrc + y, pixel);
                    (this->*pipe.run)(&pipe);
                }
            }
        } else {
            for (y = 0; y < h; ++y) {
                pipeSetXY(&pipe, xDest, yDest + y);
                for (x = 0; x < w; ++x) {
                    src->getPixel(xSrc + x, ySrc + y, pixel);
                    if (state->clip->test(xDest + x, yDest + y)) {
                        (this->*pipe.run)(&pipe);
                    } else {
                        pipeIncX(&pipe);
                    }
                }
            }
        }
    }

    return splashOk;
}

void Splash::compositeBackground(SplashColorConstPtr color)
{
    SplashColorPtr p;
    unsigned char *q;
    unsigned char alpha, alpha1, c, color0, color1, color2;
    unsigned char color3;
    unsigned char colorsp[SPOT_NCOMPS + 4], cp;
    int x, y, mask;

    if (unlikely(bitmap->alpha == nullptr)) {
        error(errInternal, -1, "bitmap->alpha is NULL in Splash::compositeBackground");
        return;
    }

    switch (bitmap->mode) {
    case splashModeMono1:
        color0 = color[0];
        for (y = 0; y < bitmap->height; ++y) {
            p = &bitmap->data[y * bitmap->rowSize];
            q = &bitmap->alpha[y * bitmap->width];
            mask = 0x80;
            for (x = 0; x < bitmap->width; ++x) {
                alpha = *q++;
                alpha1 = 255 - alpha;
                c = (*p & mask) ? 0xff : 0x00;
                c = div255(alpha1 * color0 + alpha * c);
                if (c & 0x80) {
                    *p |= mask;
                } else {
                    *p &= ~mask;
                }
                if (!(mask >>= 1)) {
                    mask = 0x80;
                    ++p;
                }
            }
        }
        break;
    case splashModeMono8:
        color0 = color[0];
        for (y = 0; y < bitmap->height; ++y) {
            p = &bitmap->data[y * bitmap->rowSize];
            q = &bitmap->alpha[y * bitmap->width];
            for (x = 0; x < bitmap->width; ++x) {
                alpha = *q++;
                alpha1 = 255 - alpha;
                p[0] = div255(alpha1 * color0 + alpha * p[0]);
                ++p;
            }
        }
        break;
    case splashModeRGB8:
    case splashModeBGR8:
        color0 = color[0];
        color1 = color[1];
        color2 = color[2];
        for (y = 0; y < bitmap->height; ++y) {
            p = &bitmap->data[y * bitmap->rowSize];
            q = &bitmap->alpha[y * bitmap->width];
            for (x = 0; x < bitmap->width; ++x) {
                alpha = *q++;
                if (alpha == 0) {
                    p[0] = color0;
                    p[1] = color1;
                    p[2] = color2;
                } else if (alpha != 255) {
                    alpha1 = 255 - alpha;
                    p[0] = div255(alpha1 * color0 + alpha * p[0]);
                    p[1] = div255(alpha1 * color1 + alpha * p[1]);
                    p[2] = div255(alpha1 * color2 + alpha * p[2]);
                }
                p += 3;
            }
        }
        break;
    case splashModeXBGR8:
        color0 = color[0];
        color1 = color[1];
        color2 = color[2];
        for (y = 0; y < bitmap->height; ++y) {
            p = &bitmap->data[y * bitmap->rowSize];
            q = &bitmap->alpha[y * bitmap->width];
            for (x = 0; x < bitmap->width; ++x) {
                alpha = *q++;
                if (alpha == 0) {
                    p[0] = color0;
                    p[1] = color1;
                    p[2] = color2;
                } else if (alpha != 255) {
                    alpha1 = 255 - alpha;
                    p[0] = div255(alpha1 * color0 + alpha * p[0]);
                    p[1] = div255(alpha1 * color1 + alpha * p[1]);
                    p[2] = div255(alpha1 * color2 + alpha * p[2]);
                }
                p[3] = 255;
                p += 4;
            }
        }
        break;
    case splashModeCMYK8:
        color0 = color[0];
        color1 = color[1];
        color2 = color[2];
        color3 = color[3];
        for (y = 0; y < bitmap->height; ++y) {
            p = &bitmap->data[y * bitmap->rowSize];
            q = &bitmap->alpha[y * bitmap->width];
            for (x = 0; x < bitmap->width; ++x) {
                alpha = *q++;
                if (alpha == 0) {
                    p[0] = color0;
                    p[1] = color1;
                    p[2] = color2;
                    p[3] = color3;
                } else if (alpha != 255) {
                    alpha1 = 255 - alpha;
                    p[0] = div255(alpha1 * color0 + alpha * p[0]);
                    p[1] = div255(alpha1 * color1 + alpha * p[1]);
                    p[2] = div255(alpha1 * color2 + alpha * p[2]);
                    p[3] = div255(alpha1 * color3 + alpha * p[3]);
                }
                p += 4;
            }
        }
        break;
    case splashModeDeviceN8:
        for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
            colorsp[cp] = color[cp];
        }
        for (y = 0; y < bitmap->height; ++y) {
            p = &bitmap->data[y * bitmap->rowSize];
            q = &bitmap->alpha[y * bitmap->width];
            for (x = 0; x < bitmap->width; ++x) {
                alpha = *q++;
                if (alpha == 0) {
                    for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                        p[cp] = colorsp[cp];
                    }
                } else if (alpha != 255) {
                    alpha1 = 255 - alpha;
                    for (cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                        p[cp] = div255(alpha1 * colorsp[cp] + alpha * p[cp]);
                    }
                }
                p += (SPOT_NCOMPS + 4);
            }
        }
        break;
    }
    memset(bitmap->alpha, 255, bitmap->width * bitmap->height);
}

bool Splash::gouraudTriangleShadedFill(SplashGouraudColor *shading)
{
    double xdbl[3] = { 0., 0., 0. };
    double ydbl[3] = { 0., 0., 0. };
    int x[3] = { 0, 0, 0 };
    int y[3] = { 0, 0, 0 };
    double xt = 0., xa = 0., yt = 0.;

    const int bitmapWidth = bitmap->getWidth();
    SplashClip *clip = getClip();
    SplashBitmap *blitTarget = bitmap;
    SplashColorPtr bitmapData = bitmap->getDataPtr();
    const int bitmapOffLimit = bitmap->getHeight() * bitmap->getRowSize();
    SplashColorPtr bitmapAlpha = bitmap->getAlphaPtr();
    SplashCoord *userToCanvasMatrix = getMatrix();
    const SplashColorMode bitmapMode = bitmap->getMode();
    bool hasAlpha = (bitmapAlpha != nullptr);
    const int rowSize = bitmap->getRowSize();
    const int colorComps = splashColorModeNComps[bitmapMode];

    SplashPipe pipe;
    SplashColor cSrcVal;

    pipeInit(&pipe, 0, 0, nullptr, cSrcVal, (unsigned char)splashRound(state->fillAlpha * 255), false, false);

    if (vectorAntialias) {
        if (aaBuf == nullptr) {
            return false; // fall back to old behaviour
        }
        drawAAPixelInit();
    }

    // idea:
    // 1. If pipe->noTransparency && !state->blendFunc
    //  -> blit directly into the drawing surface!
    //  -> disable alpha manually.
    // 2. Otherwise:
    // - blit also directly, but into an intermediate surface.
    // Afterwards, blit the intermediate surface using the drawing pipeline.
    // This is necessary because triangle elements can be on top of each
    // other, so the complete shading needs to be drawn before opacity is
    // applied.
    // - the final step, is performed using a SplashPipe:
    // - assign the actual color into cSrcVal: pipe uses cSrcVal by reference
    // - invoke drawPixel(&pipe,X,Y,bNoClip);
    const bool bDirectBlit = vectorAntialias ? false : pipe.noTransparency && !state->blendFunc && !shading->isParameterized();
    if (!bDirectBlit) {
        blitTarget = new SplashBitmap(bitmap->getWidth(), bitmap->getHeight(), bitmap->getRowPad(), bitmap->getMode(), true, bitmap->getRowSize() >= 0);
        bitmapData = blitTarget->getDataPtr();
        bitmapAlpha = blitTarget->getAlphaPtr();

        // initialisation seems to be necessary:
        const int S = bitmap->getWidth() * bitmap->getHeight();
        for (int i = 0; i < S; ++i) {
            bitmapAlpha[i] = 0;
        }
        hasAlpha = true;
    }

    if (shading->isParameterized()) {
        double color[3];
        double scanLimitMapL[2] = { 0., 0. };
        double scanLimitMapR[2] = { 0., 0. };
        double scanColorMapL[2] = { 0., 0. };
        double scanColorMapR[2] = { 0., 0. };
        int scanEdgeL[2] = { 0, 0 };
        int scanEdgeR[2] = { 0, 0 };

        for (int i = 0; i < shading->getNTriangles(); ++i) {
            shading->getParametrizedTriangle(i, xdbl + 0, ydbl + 0, color + 0, xdbl + 1, ydbl + 1, color + 1, xdbl + 2, ydbl + 2, color + 2);
            for (int m = 0; m < 3; ++m) {
                xt = xdbl[m] * (double)userToCanvasMatrix[0] + ydbl[m] * (double)userToCanvasMatrix[2] + (double)userToCanvasMatrix[4];
                yt = xdbl[m] * (double)userToCanvasMatrix[1] + ydbl[m] * (double)userToCanvasMatrix[3] + (double)userToCanvasMatrix[5];
                xdbl[m] = xt;
                ydbl[m] = yt;
                // we operate on scanlines which are integer offsets into the
                // raster image. The double offsets are of no use here.
                x[m] = splashRound(xt);
                y[m] = splashRound(yt);
            }
            // sort according to y coordinate to simplify sweep through scanlines:
            // INSERTION SORT.
            if (y[0] > y[1]) {
                Guswap(x[0], x[1]);
                Guswap(y[0], y[1]);
                Guswap(color[0], color[1]);
            }
            // first two are sorted.
            assert(y[0] <= y[1]);
            if (y[1] > y[2]) {
                const int tmpX = x[2];
                const int tmpY = y[2];
                const double tmpC = color[2];
                x[2] = x[1];
                y[2] = y[1];
                color[2] = color[1];

                if (y[0] > tmpY) {
                    x[1] = x[0];
                    y[1] = y[0];
                    color[1] = color[0];
                    x[0] = tmpX;
                    y[0] = tmpY;
                    color[0] = tmpC;
                } else {
                    x[1] = tmpX;
                    y[1] = tmpY;
                    color[1] = tmpC;
                }
            }
            // first three are sorted
            assert(y[0] <= y[1]);
            assert(y[1] <= y[2]);
            /////

            // this here is det( T ) == 0
            // where T is the matrix to map to barycentric coordinates.
            if ((x[0] - x[2]) * (y[1] - y[2]) - (x[1] - x[2]) * (y[0] - y[2]) == 0) {
                continue; // degenerate triangle.
            }

            // this here initialises the scanline generation.
            // We start with low Y coordinates and sweep up to the large Y
            // coordinates.
            //
            // scanEdgeL[m] in {0,1,2} m=0,1
            // scanEdgeR[m] in {0,1,2} m=0,1
            //
            // are the two edges between which scanlines are (currently)
            // sweeped. The values {0,1,2} are indices into 'x' and 'y'.
            // scanEdgeL[0] = 0 means: the left scan edge has (x[0],y[0]) as vertex.
            //
            scanEdgeL[0] = 0;
            scanEdgeR[0] = 0;
            if (y[0] == y[1]) {
                scanEdgeL[0] = 1;
                scanEdgeL[1] = scanEdgeR[1] = 2;

            } else {
                scanEdgeL[1] = 1;
                scanEdgeR[1] = 2;
            }
            assert(y[scanEdgeL[0]] < y[scanEdgeL[1]]);
            assert(y[scanEdgeR[0]] < y[scanEdgeR[1]]);

            // Ok. Now prepare the linear maps which map the y coordinate of
            // the current scanline to the corresponding LEFT and RIGHT x
            // coordinate (which define the scanline).
            scanLimitMapL[0] = double(x[scanEdgeL[1]] - x[scanEdgeL[0]]) / (y[scanEdgeL[1]] - y[scanEdgeL[0]]);
            scanLimitMapL[1] = x[scanEdgeL[0]] - y[scanEdgeL[0]] * scanLimitMapL[0];
            scanLimitMapR[0] = double(x[scanEdgeR[1]] - x[scanEdgeR[0]]) / (y[scanEdgeR[1]] - y[scanEdgeR[0]]);
            scanLimitMapR[1] = x[scanEdgeR[0]] - y[scanEdgeR[0]] * scanLimitMapR[0];

            xa = y[1] * scanLimitMapL[0] + scanLimitMapL[1];
            xt = y[1] * scanLimitMapR[0] + scanLimitMapR[1];
            if (xa > xt) {
                // I have "left" is to the right of "right".
                // Exchange sides!
                Guswap(scanEdgeL[0], scanEdgeR[0]);
                Guswap(scanEdgeL[1], scanEdgeR[1]);
                Guswap(scanLimitMapL[0], scanLimitMapR[0]);
                Guswap(scanLimitMapL[1], scanLimitMapR[1]);
                // FIXME I'm sure there is a more efficient way to check this.
            }

            // Same game: we can linearly interpolate the color based on the
            // current y coordinate (that's correct for triangle
            // interpolation due to linearity. We could also have done it in
            // barycentric coordinates, but that's slightly more involved)
            scanColorMapL[0] = (color[scanEdgeL[1]] - color[scanEdgeL[0]]) / (y[scanEdgeL[1]] - y[scanEdgeL[0]]);
            scanColorMapL[1] = color[scanEdgeL[0]] - y[scanEdgeL[0]] * scanColorMapL[0];
            scanColorMapR[0] = (color[scanEdgeR[1]] - color[scanEdgeR[0]]) / (y[scanEdgeR[1]] - y[scanEdgeR[0]]);
            scanColorMapR[1] = color[scanEdgeR[0]] - y[scanEdgeR[0]] * scanColorMapR[0];

            bool hasFurtherSegment = (y[1] < y[2]);
            int scanLineOff = y[0] * rowSize;

            for (int Y = y[0]; Y <= y[2]; ++Y, scanLineOff += rowSize) {
                if (hasFurtherSegment && Y == y[1]) {
                    // SWEEP EVENT: we encountered the next segment.
                    //
                    // switch to next segment, either at left end or at right
                    // end:
                    if (scanEdgeL[1] == 1) {
                        scanEdgeL[0] = 1;
                        scanEdgeL[1] = 2;
                        scanLimitMapL[0] = double(x[scanEdgeL[1]] - x[scanEdgeL[0]]) / (y[scanEdgeL[1]] - y[scanEdgeL[0]]);
                        scanLimitMapL[1] = x[scanEdgeL[0]] - y[scanEdgeL[0]] * scanLimitMapL[0];

                        scanColorMapL[0] = (color[scanEdgeL[1]] - color[scanEdgeL[0]]) / (y[scanEdgeL[1]] - y[scanEdgeL[0]]);
                        scanColorMapL[1] = color[scanEdgeL[0]] - y[scanEdgeL[0]] * scanColorMapL[0];
                    } else if (scanEdgeR[1] == 1) {
                        scanEdgeR[0] = 1;
                        scanEdgeR[1] = 2;
                        scanLimitMapR[0] = double(x[scanEdgeR[1]] - x[scanEdgeR[0]]) / (y[scanEdgeR[1]] - y[scanEdgeR[0]]);
                        scanLimitMapR[1] = x[scanEdgeR[0]] - y[scanEdgeR[0]] * scanLimitMapR[0];

                        scanColorMapR[0] = (color[scanEdgeR[1]] - color[scanEdgeR[0]]) / (y[scanEdgeR[1]] - y[scanEdgeR[0]]);
                        scanColorMapR[1] = color[scanEdgeR[0]] - y[scanEdgeR[0]] * scanColorMapR[0];
                    }
                    assert(y[scanEdgeL[0]] < y[scanEdgeL[1]]);
                    assert(y[scanEdgeR[0]] < y[scanEdgeR[1]]);
                    hasFurtherSegment = false;
                }

                yt = Y;

                xa = yt * scanLimitMapL[0] + scanLimitMapL[1];
                xt = yt * scanLimitMapR[0] + scanLimitMapR[1];

                const double ca = yt * scanColorMapL[0] + scanColorMapL[1];
                const double ct = yt * scanColorMapR[0] + scanColorMapR[1];

                const int scanLimitL = splashRound(xa);
                const int scanLimitR = splashRound(xt);

                // Ok. Now: init the color interpolation depending on the X
                // coordinate inside of the current scanline:
                const double scanColorMap0 = (scanLimitR == scanLimitL) ? 0. : ((ct - ca) / (scanLimitR - scanLimitL));
                const double scanColorMap1 = ca - scanLimitL * scanColorMap0;

                // handled by clipping:
                // assert( scanLimitL >= 0 && scanLimitR < bitmap->getWidth() );
                assert(scanLimitL <= scanLimitR || abs(scanLimitL - scanLimitR) <= 2); // allow rounding inaccuracies
                assert(scanLineOff == Y * rowSize);

                double colorinterp = scanColorMap0 * scanLimitL + scanColorMap1;

                int bitmapOff = scanLineOff + scanLimitL * colorComps;
                if (likely(bitmapOff >= 0)) {
                    for (int X = scanLimitL; X <= scanLimitR && bitmapOff + colorComps <= bitmapOffLimit; ++X, colorinterp += scanColorMap0, bitmapOff += colorComps) {
                        // FIXME : standard rectangular clipping can be done for a
                        // complete scanline which is faster
                        // --> see SplashClip and its methods
                        if (!clip->test(X, Y)) {
                            continue;
                        }

                        assert(fabs(colorinterp - (scanColorMap0 * X + scanColorMap1)) < 1e-7);
                        assert(bitmapOff == Y * rowSize + colorComps * X && scanLineOff == Y * rowSize);

                        shading->getParameterizedColor(colorinterp, bitmapMode, &bitmapData[bitmapOff]);

                        // make the shading visible.
                        // Note that opacity is handled by the bDirectBlit stuff, see
                        // above for comments and below for implementation.
                        if (hasAlpha) {
                            bitmapAlpha[Y * bitmapWidth + X] = 255;
                        }
                    }
                }
            }
        }
    } else {
        SplashColor color, auxColor1, auxColor2;
        double scanLimitMapL[2] = { 0., 0. };
        double scanLimitMapR[2] = { 0., 0. };
        int scanEdgeL[2] = { 0, 0 };
        int scanEdgeR[2] = { 0, 0 };

        for (int i = 0; i < shading->getNTriangles(); ++i) {
            // Sadly this current algorithm only supports shadings where the three triangle vertices have the same color
            shading->getNonParametrizedTriangle(i, bitmapMode, xdbl + 0, ydbl + 0, (SplashColorPtr)&color, xdbl + 1, ydbl + 1, (SplashColorPtr)&auxColor1, xdbl + 2, ydbl + 2, (SplashColorPtr)&auxColor2);
            if (!splashColorEqual(color, auxColor1) || !splashColorEqual(color, auxColor2)) {
                if (!bDirectBlit) {
                    delete blitTarget;
                }
                return false;
            }
            for (int m = 0; m < 3; ++m) {
                xt = xdbl[m] * (double)userToCanvasMatrix[0] + ydbl[m] * (double)userToCanvasMatrix[2] + (double)userToCanvasMatrix[4];
                yt = xdbl[m] * (double)userToCanvasMatrix[1] + ydbl[m] * (double)userToCanvasMatrix[3] + (double)userToCanvasMatrix[5];
                xdbl[m] = xt;
                ydbl[m] = yt;
                // we operate on scanlines which are integer offsets into the
                // raster image. The double offsets are of no use here.
                x[m] = splashRound(xt);
                y[m] = splashRound(yt);
            }
            // sort according to y coordinate to simplify sweep through scanlines:
            // INSERTION SORT.
            if (y[0] > y[1]) {
                Guswap(x[0], x[1]);
                Guswap(y[0], y[1]);
            }
            // first two are sorted.
            assert(y[0] <= y[1]);
            if (y[1] > y[2]) {
                const int tmpX = x[2];
                const int tmpY = y[2];
                x[2] = x[1];
                y[2] = y[1];

                if (y[0] > tmpY) {
                    x[1] = x[0];
                    y[1] = y[0];
                    x[0] = tmpX;
                    y[0] = tmpY;
                } else {
                    x[1] = tmpX;
                    y[1] = tmpY;
                }
            }
            // first three are sorted
            assert(y[0] <= y[1]);
            assert(y[1] <= y[2]);
            /////

            // this here is det( T ) == 0
            // where T is the matrix to map to barycentric coordinates.
            if ((x[0] - x[2]) * (y[1] - y[2]) - (x[1] - x[2]) * (y[0] - y[2]) == 0) {
                continue; // degenerate triangle.
            }

            // this here initialises the scanline generation.
            // We start with low Y coordinates and sweep up to the large Y
            // coordinates.
            //
            // scanEdgeL[m] in {0,1,2} m=0,1
            // scanEdgeR[m] in {0,1,2} m=0,1
            //
            // are the two edges between which scanlines are (currently)
            // sweeped. The values {0,1,2} are indices into 'x' and 'y'.
            // scanEdgeL[0] = 0 means: the left scan edge has (x[0],y[0]) as vertex.
            //
            scanEdgeL[0] = 0;
            scanEdgeR[0] = 0;
            if (y[0] == y[1]) {
                scanEdgeL[0] = 1;
                scanEdgeL[1] = scanEdgeR[1] = 2;

            } else {
                scanEdgeL[1] = 1;
                scanEdgeR[1] = 2;
            }
            assert(y[scanEdgeL[0]] < y[scanEdgeL[1]]);
            assert(y[scanEdgeR[0]] < y[scanEdgeR[1]]);

            // Ok. Now prepare the linear maps which map the y coordinate of
            // the current scanline to the corresponding LEFT and RIGHT x
            // coordinate (which define the scanline).
            scanLimitMapL[0] = double(x[scanEdgeL[1]] - x[scanEdgeL[0]]) / (y[scanEdgeL[1]] - y[scanEdgeL[0]]);
            scanLimitMapL[1] = x[scanEdgeL[0]] - y[scanEdgeL[0]] * scanLimitMapL[0];
            scanLimitMapR[0] = double(x[scanEdgeR[1]] - x[scanEdgeR[0]]) / (y[scanEdgeR[1]] - y[scanEdgeR[0]]);
            scanLimitMapR[1] = x[scanEdgeR[0]] - y[scanEdgeR[0]] * scanLimitMapR[0];

            xa = y[1] * scanLimitMapL[0] + scanLimitMapL[1];
            xt = y[1] * scanLimitMapR[0] + scanLimitMapR[1];
            if (xa > xt) {
                // I have "left" is to the right of "right".
                // Exchange sides!
                Guswap(scanEdgeL[0], scanEdgeR[0]);
                Guswap(scanEdgeL[1], scanEdgeR[1]);
                Guswap(scanLimitMapL[0], scanLimitMapR[0]);
                Guswap(scanLimitMapL[1], scanLimitMapR[1]);
                // FIXME I'm sure there is a more efficient way to check this.
            }

            bool hasFurtherSegment = (y[1] < y[2]);
            int scanLineOff = y[0] * rowSize;

            for (int Y = y[0]; Y <= y[2]; ++Y, scanLineOff += rowSize) {
                if (hasFurtherSegment && Y == y[1]) {
                    // SWEEP EVENT: we encountered the next segment.
                    //
                    // switch to next segment, either at left end or at right
                    // end:
                    if (scanEdgeL[1] == 1) {
                        scanEdgeL[0] = 1;
                        scanEdgeL[1] = 2;
                        scanLimitMapL[0] = double(x[scanEdgeL[1]] - x[scanEdgeL[0]]) / (y[scanEdgeL[1]] - y[scanEdgeL[0]]);
                        scanLimitMapL[1] = x[scanEdgeL[0]] - y[scanEdgeL[0]] * scanLimitMapL[0];
                    } else if (scanEdgeR[1] == 1) {
                        scanEdgeR[0] = 1;
                        scanEdgeR[1] = 2;
                        scanLimitMapR[0] = double(x[scanEdgeR[1]] - x[scanEdgeR[0]]) / (y[scanEdgeR[1]] - y[scanEdgeR[0]]);
                        scanLimitMapR[1] = x[scanEdgeR[0]] - y[scanEdgeR[0]] * scanLimitMapR[0];
                    }
                    assert(y[scanEdgeL[0]] < y[scanEdgeL[1]]);
                    assert(y[scanEdgeR[0]] < y[scanEdgeR[1]]);
                    hasFurtherSegment = false;
                }

                yt = Y;

                xa = yt * scanLimitMapL[0] + scanLimitMapL[1];
                xt = yt * scanLimitMapR[0] + scanLimitMapR[1];

                const int scanLimitL = splashRound(xa);
                const int scanLimitR = splashRound(xt);

                // handled by clipping:
                // assert( scanLimitL >= 0 && scanLimitR < bitmap->getWidth() );
                assert(scanLimitL <= scanLimitR || abs(scanLimitL - scanLimitR) <= 2); // allow rounding inaccuracies
                assert(scanLineOff == Y * rowSize);

                int bitmapOff = scanLineOff + scanLimitL * colorComps;
                if (likely(bitmapOff >= 0)) {
                    for (int X = scanLimitL; X <= scanLimitR && bitmapOff + colorComps <= bitmapOffLimit; ++X, bitmapOff += colorComps) {
                        // FIXME : standard rectangular clipping can be done for a
                        // complete scanline which is faster
                        // --> see SplashClip and its methods
                        if (!clip->test(X, Y)) {
                            continue;
                        }

                        assert(bitmapOff == Y * rowSize + colorComps * X && scanLineOff == Y * rowSize);

                        for (int k = 0; k < colorComps; ++k) {
                            bitmapData[bitmapOff + k] = color[k];
                        }

                        // make the shading visible.
                        // Note that opacity is handled by the bDirectBlit stuff, see
                        // above for comments and below for implementation.
                        if (hasAlpha) {
                            bitmapAlpha[Y * bitmapWidth + X] = 255;
                        }
                    }
                }
            }
        }
    }

    if (!bDirectBlit) {
        // ok. Finalize the stuff by blitting the shading into the final
        // geometry, this time respecting the rendering pipe.
        const int W = blitTarget->getWidth();
        const int H = blitTarget->getHeight();
        SplashColorPtr cur = cSrcVal;

        for (int X = 0; X < W; ++X) {
            for (int Y = 0; Y < H; ++Y) {
                if (!bitmapAlpha[Y * bitmapWidth + X]) {
                    continue; // draw only parts of the shading!
                }
                const int bitmapOff = Y * rowSize + colorComps * X;

                for (int m = 0; m < colorComps; ++m) {
                    cur[m] = bitmapData[bitmapOff + m];
                }
                if (vectorAntialias) {
                    drawAAPixel(&pipe, X, Y);
                } else {
                    drawPixel(&pipe, X, Y, true); // no clipping - has already been done.
                }
            }
        }

        delete blitTarget;
        blitTarget = nullptr;
    }

    return true;
}

SplashError Splash::blitTransparent(SplashBitmap *src, int xSrc, int ySrc, int xDest, int yDest, int w, int h)
{
    SplashColorPtr p, sp;
    unsigned char *q;
    int x, y, mask, srcMask, width = w, height = h;

    if (src->mode != bitmap->mode) {
        return splashErrModeMismatch;
    }

    if (unlikely(!bitmap->data)) {
        return splashErrZeroImage;
    }

    if (src->getWidth() - xSrc < width) {
        width = src->getWidth() - xSrc;
    }

    if (src->getHeight() - ySrc < height) {
        height = src->getHeight() - ySrc;
    }

    if (bitmap->getWidth() - xDest < width) {
        width = bitmap->getWidth() - xDest;
    }

    if (bitmap->getHeight() - yDest < height) {
        height = bitmap->getHeight() - yDest;
    }

    if (width < 0) {
        width = 0;
    }

    if (height < 0) {
        height = 0;
    }

    switch (bitmap->mode) {
    case splashModeMono1:
        for (y = 0; y < height; ++y) {
            p = &bitmap->data[(yDest + y) * bitmap->rowSize + (xDest >> 3)];
            mask = 0x80 >> (xDest & 7);
            sp = &src->data[(ySrc + y) * src->rowSize + (xSrc >> 3)];
            srcMask = 0x80 >> (xSrc & 7);
            for (x = 0; x < width; ++x) {
                if (*sp & srcMask) {
                    *p |= mask;
                } else {
                    *p &= ~mask;
                }
                if (!(mask >>= 1)) {
                    mask = 0x80;
                    ++p;
                }
                if (!(srcMask >>= 1)) {
                    srcMask = 0x80;
                    ++sp;
                }
            }
        }
        break;
    case splashModeMono8:
        for (y = 0; y < height; ++y) {
            p = &bitmap->data[(yDest + y) * bitmap->rowSize + xDest];
            sp = &src->data[(ySrc + y) * bitmap->rowSize + xSrc];
            for (x = 0; x < width; ++x) {
                *p++ = *sp++;
            }
        }
        break;
    case splashModeRGB8:
    case splashModeBGR8:
        for (y = 0; y < height; ++y) {
            p = &bitmap->data[(yDest + y) * bitmap->rowSize + 3 * xDest];
            sp = &src->data[(ySrc + y) * src->rowSize + 3 * xSrc];
            for (x = 0; x < width; ++x) {
                *p++ = *sp++;
                *p++ = *sp++;
                *p++ = *sp++;
            }
        }
        break;
    case splashModeXBGR8:
        for (y = 0; y < height; ++y) {
            p = &bitmap->data[(yDest + y) * bitmap->rowSize + 4 * xDest];
            sp = &src->data[(ySrc + y) * src->rowSize + 4 * xSrc];
            for (x = 0; x < width; ++x) {
                *p++ = *sp++;
                *p++ = *sp++;
                *p++ = *sp++;
                *p++ = 255;
                sp++;
            }
        }
        break;
    case splashModeCMYK8:
        for (y = 0; y < height; ++y) {
            p = &bitmap->data[(yDest + y) * bitmap->rowSize + 4 * xDest];
            sp = &src->data[(ySrc + y) * src->rowSize + 4 * xSrc];
            for (x = 0; x < width; ++x) {
                *p++ = *sp++;
                *p++ = *sp++;
                *p++ = *sp++;
                *p++ = *sp++;
            }
        }
        break;
    case splashModeDeviceN8:
        for (y = 0; y < height; ++y) {
            p = &bitmap->data[(yDest + y) * bitmap->rowSize + (SPOT_NCOMPS + 4) * xDest];
            sp = &src->data[(ySrc + y) * src->rowSize + (SPOT_NCOMPS + 4) * xSrc];
            for (x = 0; x < width; ++x) {
                for (int cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
                    *p++ = *sp++;
                }
            }
        }
        break;
    }

    if (bitmap->alpha) {
        for (y = 0; y < height; ++y) {
            q = &bitmap->alpha[(yDest + y) * bitmap->width + xDest];
            memset(q, 0x00, width);
        }
    }

    return splashOk;
}

SplashPath *Splash::makeStrokePath(SplashPath *path, SplashCoord w, bool flatten)
{
    SplashPath *pathIn, *dashPath, *pathOut;
    SplashCoord d, dx, dy, wdx, wdy, dxNext, dyNext, wdxNext, wdyNext;
    SplashCoord crossprod, dotprod, miter, m;
    bool first, last, closed, hasangle;
    int subpathStart0, subpathStart1, seg, i0, i1, j0, j1, k0, k1;
    int left0, left1, left2, right0, right1, right2, join0, join1, join2;
    int leftFirst, rightFirst, firstPt;

    pathOut = new SplashPath();

    if (path->length == 0) {
        return pathOut;
    }

    if (flatten) {
        pathIn = flattenPath(path, state->matrix, state->flatness);
        if (!state->lineDash.empty()) {
            dashPath = makeDashedPath(pathIn);
            delete pathIn;
            pathIn = dashPath;
            if (pathIn->length == 0) {
                delete pathIn;
                return pathOut;
            }
        }
    } else {
        pathIn = path;
    }

    subpathStart0 = subpathStart1 = 0; // make gcc happy
    seg = 0; // make gcc happy
    closed = false; // make gcc happy
    left0 = left1 = right0 = right1 = join0 = join1 = 0; // make gcc happy
    leftFirst = rightFirst = firstPt = 0; // make gcc happy

    i0 = 0;
    for (i1 = i0; !(pathIn->flags[i1] & splashPathLast) && i1 + 1 < pathIn->length && pathIn->pts[i1 + 1].x == pathIn->pts[i1].x && pathIn->pts[i1 + 1].y == pathIn->pts[i1].y; ++i1) {
        ;
    }

    while (i1 < pathIn->length) {
        if ((first = pathIn->flags[i0] & splashPathFirst)) {
            subpathStart0 = i0;
            subpathStart1 = i1;
            seg = 0;
            closed = pathIn->flags[i0] & splashPathClosed;
        }
        j0 = i1 + 1;
        if (j0 < pathIn->length) {
            for (j1 = j0; !(pathIn->flags[j1] & splashPathLast) && j1 + 1 < pathIn->length && pathIn->pts[j1 + 1].x == pathIn->pts[j1].x && pathIn->pts[j1 + 1].y == pathIn->pts[j1].y; ++j1) {
                ;
            }
        } else {
            j1 = j0;
        }
        if (pathIn->flags[i1] & splashPathLast) {
            if (first && state->lineCap == splashLineCapRound) {
                // special case: zero-length subpath with round line caps -->
                // draw a circle
                pathOut->moveTo(pathIn->pts[i0].x + (SplashCoord)0.5 * w, pathIn->pts[i0].y);
                pathOut->curveTo(pathIn->pts[i0].x + (SplashCoord)0.5 * w, pathIn->pts[i0].y + bezierCircle2 * w, pathIn->pts[i0].x + bezierCircle2 * w, pathIn->pts[i0].y + (SplashCoord)0.5 * w, pathIn->pts[i0].x,
                                 pathIn->pts[i0].y + (SplashCoord)0.5 * w);
                pathOut->curveTo(pathIn->pts[i0].x - bezierCircle2 * w, pathIn->pts[i0].y + (SplashCoord)0.5 * w, pathIn->pts[i0].x - (SplashCoord)0.5 * w, pathIn->pts[i0].y + bezierCircle2 * w, pathIn->pts[i0].x - (SplashCoord)0.5 * w,
                                 pathIn->pts[i0].y);
                pathOut->curveTo(pathIn->pts[i0].x - (SplashCoord)0.5 * w, pathIn->pts[i0].y - bezierCircle2 * w, pathIn->pts[i0].x - bezierCircle2 * w, pathIn->pts[i0].y - (SplashCoord)0.5 * w, pathIn->pts[i0].x,
                                 pathIn->pts[i0].y - (SplashCoord)0.5 * w);
                pathOut->curveTo(pathIn->pts[i0].x + bezierCircle2 * w, pathIn->pts[i0].y - (SplashCoord)0.5 * w, pathIn->pts[i0].x + (SplashCoord)0.5 * w, pathIn->pts[i0].y - bezierCircle2 * w, pathIn->pts[i0].x + (SplashCoord)0.5 * w,
                                 pathIn->pts[i0].y);
                pathOut->close();
            }
            i0 = j0;
            i1 = j1;
            continue;
        }
        last = pathIn->flags[j1] & splashPathLast;
        if (last) {
            k0 = subpathStart1 + 1;
        } else {
            k0 = j1 + 1;
        }
        for (k1 = k0; !(pathIn->flags[k1] & splashPathLast) && k1 + 1 < pathIn->length && pathIn->pts[k1 + 1].x == pathIn->pts[k1].x && pathIn->pts[k1 + 1].y == pathIn->pts[k1].y; ++k1) {
            ;
        }

        // compute the deltas for segment (i1, j0)
        d = (SplashCoord)1 / splashDist(pathIn->pts[i1].x, pathIn->pts[i1].y, pathIn->pts[j0].x, pathIn->pts[j0].y);
        dx = d * (pathIn->pts[j0].x - pathIn->pts[i1].x);
        dy = d * (pathIn->pts[j0].y - pathIn->pts[i1].y);
        wdx = (SplashCoord)0.5 * w * dx;
        wdy = (SplashCoord)0.5 * w * dy;

        // draw the start cap
        if (pathOut->moveTo(pathIn->pts[i0].x - wdy, pathIn->pts[i0].y + wdx) != splashOk) {
            break;
        }
        if (i0 == subpathStart0) {
            firstPt = pathOut->length - 1;
        }
        if (first && !closed) {
            switch (state->lineCap) {
            case splashLineCapButt:
                pathOut->lineTo(pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
                break;
            case splashLineCapRound:
                pathOut->curveTo(pathIn->pts[i0].x - wdy - bezierCircle * wdx, pathIn->pts[i0].y + wdx - bezierCircle * wdy, pathIn->pts[i0].x - wdx - bezierCircle * wdy, pathIn->pts[i0].y - wdy + bezierCircle * wdx,
                                 pathIn->pts[i0].x - wdx, pathIn->pts[i0].y - wdy);
                pathOut->curveTo(pathIn->pts[i0].x - wdx + bezierCircle * wdy, pathIn->pts[i0].y - wdy - bezierCircle * wdx, pathIn->pts[i0].x + wdy - bezierCircle * wdx, pathIn->pts[i0].y - wdx - bezierCircle * wdy,
                                 pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
                break;
            case splashLineCapProjecting:
                pathOut->lineTo(pathIn->pts[i0].x - wdx - wdy, pathIn->pts[i0].y + wdx - wdy);
                pathOut->lineTo(pathIn->pts[i0].x - wdx + wdy, pathIn->pts[i0].y - wdx - wdy);
                pathOut->lineTo(pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
                break;
            }
        } else {
            pathOut->lineTo(pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
        }

        // draw the left side of the segment rectangle
        left2 = pathOut->length - 1;
        pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);

        // draw the end cap
        if (last && !closed) {
            switch (state->lineCap) {
            case splashLineCapButt:
                pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
                break;
            case splashLineCapRound:
                pathOut->curveTo(pathIn->pts[j0].x + wdy + bezierCircle * wdx, pathIn->pts[j0].y - wdx + bezierCircle * wdy, pathIn->pts[j0].x + wdx + bezierCircle * wdy, pathIn->pts[j0].y + wdy - bezierCircle * wdx,
                                 pathIn->pts[j0].x + wdx, pathIn->pts[j0].y + wdy);
                pathOut->curveTo(pathIn->pts[j0].x + wdx - bezierCircle * wdy, pathIn->pts[j0].y + wdy + bezierCircle * wdx, pathIn->pts[j0].x - wdy + bezierCircle * wdx, pathIn->pts[j0].y + wdx + bezierCircle * wdy,
                                 pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
                break;
            case splashLineCapProjecting:
                pathOut->lineTo(pathIn->pts[j0].x + wdy + wdx, pathIn->pts[j0].y - wdx + wdy);
                pathOut->lineTo(pathIn->pts[j0].x - wdy + wdx, pathIn->pts[j0].y + wdx + wdy);
                pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
                break;
            }
        } else {
            pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
        }

        // draw the right side of the segment rectangle
        // (NB: if stroke adjustment is enabled, the closepath operation MUST
        // add a segment because this segment is used for a hint)
        right2 = pathOut->length - 1;
        pathOut->close(state->strokeAdjust);

        // draw the join
        join2 = pathOut->length;
        if (!last || closed) {

            // compute the deltas for segment (j1, k0)
            d = (SplashCoord)1 / splashDist(pathIn->pts[j1].x, pathIn->pts[j1].y, pathIn->pts[k0].x, pathIn->pts[k0].y);
            dxNext = d * (pathIn->pts[k0].x - pathIn->pts[j1].x);
            dyNext = d * (pathIn->pts[k0].y - pathIn->pts[j1].y);
            wdxNext = (SplashCoord)0.5 * w * dxNext;
            wdyNext = (SplashCoord)0.5 * w * dyNext;

            // compute the join parameters
            crossprod = dx * dyNext - dy * dxNext;
            dotprod = -(dx * dxNext + dy * dyNext);
            hasangle = crossprod != 0 || dx * dxNext < 0 || dy * dyNext < 0;
            if (dotprod > 0.9999) {
                // avoid a divide-by-zero -- set miter to something arbitrary
                // such that sqrt(miter) will exceed miterLimit (and m is never
                // used in that situation)
                // (note: the comparison value (0.9999) has to be less than
                // 1-epsilon, where epsilon is the smallest value
                // representable in the fixed point format)
                miter = (state->miterLimit + 1) * (state->miterLimit + 1);
                m = 0;
            } else {
                miter = (SplashCoord)2 / ((SplashCoord)1 - dotprod);
                if (miter < 1) {
                    // this can happen because of floating point inaccuracies
                    miter = 1;
                }
                m = splashSqrt(miter - 1);
            }

            // hasangle == false means that the current and and the next segment
            // are parallel.  In that case no join needs to be drawn.
            // round join
            if (hasangle && state->lineJoin == splashLineJoinRound) {
                // join angle < 180
                if (crossprod < 0) {
                    SplashCoord angle = atan2((double)dx, (double)-dy);
                    SplashCoord angleNext = atan2((double)dxNext, (double)-dyNext);
                    if (angle < angleNext) {
                        angle += 2 * M_PI;
                    }
                    SplashCoord dAngle = (angle - angleNext) / M_PI;
                    if (dAngle < 0.501) {
                        // span angle is <= 90 degrees -> draw a single arc
                        SplashCoord kappa = dAngle * bezierCircle * w;
                        SplashCoord cx1 = pathIn->pts[j0].x - wdy + kappa * dx;
                        SplashCoord cy1 = pathIn->pts[j0].y + wdx + kappa * dy;
                        SplashCoord cx2 = pathIn->pts[j0].x - wdyNext - kappa * dxNext;
                        SplashCoord cy2 = pathIn->pts[j0].y + wdxNext - kappa * dyNext;
                        pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
                        pathOut->lineTo(pathIn->pts[j0].x - wdyNext, pathIn->pts[j0].y + wdxNext);
                        pathOut->curveTo(cx2, cy2, cx1, cy1, pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
                    } else {
                        // span angle is > 90 degrees -> split into two arcs
                        SplashCoord dJoin = splashDist(-wdy, wdx, -wdyNext, wdxNext);
                        if (dJoin > 0) {
                            SplashCoord dxJoin = (-wdyNext + wdy) / dJoin;
                            SplashCoord dyJoin = (wdxNext - wdx) / dJoin;
                            SplashCoord xc = pathIn->pts[j0].x + (SplashCoord)0.5 * w * cos((double)((SplashCoord)0.5 * (angle + angleNext)));
                            SplashCoord yc = pathIn->pts[j0].y + (SplashCoord)0.5 * w * sin((double)((SplashCoord)0.5 * (angle + angleNext)));
                            SplashCoord kappa = dAngle * bezierCircle2 * w;
                            SplashCoord cx1 = pathIn->pts[j0].x - wdy + kappa * dx;
                            SplashCoord cy1 = pathIn->pts[j0].y + wdx + kappa * dy;
                            SplashCoord cx2 = xc - kappa * dxJoin;
                            SplashCoord cy2 = yc - kappa * dyJoin;
                            SplashCoord cx3 = xc + kappa * dxJoin;
                            SplashCoord cy3 = yc + kappa * dyJoin;
                            SplashCoord cx4 = pathIn->pts[j0].x - wdyNext - kappa * dxNext;
                            SplashCoord cy4 = pathIn->pts[j0].y + wdxNext - kappa * dyNext;
                            pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
                            pathOut->lineTo(pathIn->pts[j0].x - wdyNext, pathIn->pts[j0].y + wdxNext);
                            pathOut->curveTo(cx4, cy4, cx3, cy3, xc, yc);
                            pathOut->curveTo(cx2, cy2, cx1, cy1, pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
                        }
                    }

                    // join angle >= 180
                } else {
                    SplashCoord angle = atan2((double)-dx, (double)dy);
                    SplashCoord angleNext = atan2((double)-dxNext, (double)dyNext);
                    if (angleNext < angle) {
                        angleNext += 2 * M_PI;
                    }
                    SplashCoord dAngle = (angleNext - angle) / M_PI;
                    if (dAngle < 0.501) {
                        // span angle is <= 90 degrees -> draw a single arc
                        SplashCoord kappa = dAngle * bezierCircle * w;
                        SplashCoord cx1 = pathIn->pts[j0].x + wdy + kappa * dx;
                        SplashCoord cy1 = pathIn->pts[j0].y - wdx + kappa * dy;
                        SplashCoord cx2 = pathIn->pts[j0].x + wdyNext - kappa * dxNext;
                        SplashCoord cy2 = pathIn->pts[j0].y - wdxNext - kappa * dyNext;
                        pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
                        pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);
                        pathOut->curveTo(cx1, cy1, cx2, cy2, pathIn->pts[j0].x + wdyNext, pathIn->pts[j0].y - wdxNext);
                    } else {
                        // span angle is > 90 degrees -> split into two arcs
                        SplashCoord dJoin = splashDist(wdy, -wdx, wdyNext, -wdxNext);
                        if (dJoin > 0) {
                            SplashCoord dxJoin = (wdyNext - wdy) / dJoin;
                            SplashCoord dyJoin = (-wdxNext + wdx) / dJoin;
                            SplashCoord xc = pathIn->pts[j0].x + (SplashCoord)0.5 * w * cos((double)((SplashCoord)0.5 * (angle + angleNext)));
                            SplashCoord yc = pathIn->pts[j0].y + (SplashCoord)0.5 * w * sin((double)((SplashCoord)0.5 * (angle + angleNext)));
                            SplashCoord kappa = dAngle * bezierCircle2 * w;
                            SplashCoord cx1 = pathIn->pts[j0].x + wdy + kappa * dx;
                            SplashCoord cy1 = pathIn->pts[j0].y - wdx + kappa * dy;
                            SplashCoord cx2 = xc - kappa * dxJoin;
                            SplashCoord cy2 = yc - kappa * dyJoin;
                            SplashCoord cx3 = xc + kappa * dxJoin;
                            SplashCoord cy3 = yc + kappa * dyJoin;
                            SplashCoord cx4 = pathIn->pts[j0].x + wdyNext - kappa * dxNext;
                            SplashCoord cy4 = pathIn->pts[j0].y - wdxNext - kappa * dyNext;
                            pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
                            pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);
                            pathOut->curveTo(cx1, cy1, cx2, cy2, xc, yc);
                            pathOut->curveTo(cx3, cy3, cx4, cy4, pathIn->pts[j0].x + wdyNext, pathIn->pts[j0].y - wdxNext);
                        }
                    }
                }

            } else if (hasangle) {
                pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);

                // angle < 180
                if (crossprod < 0) {
                    pathOut->lineTo(pathIn->pts[j0].x - wdyNext, pathIn->pts[j0].y + wdxNext);
                    // miter join inside limit
                    if (state->lineJoin == splashLineJoinMiter && splashSqrt(miter) <= state->miterLimit) {
                        pathOut->lineTo(pathIn->pts[j0].x - wdy + wdx * m, pathIn->pts[j0].y + wdx + wdy * m);
                        pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
                        // bevel join or miter join outside limit
                    } else {
                        pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
                    }

                    // angle >= 180
                } else {
                    pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);
                    // miter join inside limit
                    if (state->lineJoin == splashLineJoinMiter && splashSqrt(miter) <= state->miterLimit) {
                        pathOut->lineTo(pathIn->pts[j0].x + wdy + wdx * m, pathIn->pts[j0].y - wdx + wdy * m);
                        pathOut->lineTo(pathIn->pts[j0].x + wdyNext, pathIn->pts[j0].y - wdxNext);
                        // bevel join or miter join outside limit
                    } else {
                        pathOut->lineTo(pathIn->pts[j0].x + wdyNext, pathIn->pts[j0].y - wdxNext);
                    }
                }
            }

            pathOut->close();
        }

        // add stroke adjustment hints
        if (state->strokeAdjust) {
            if (seg == 0 && !closed) {
                if (state->lineCap == splashLineCapButt) {
                    pathOut->addStrokeAdjustHint(firstPt, left2 + 1, firstPt, firstPt + 1);
                    if (last) {
                        pathOut->addStrokeAdjustHint(firstPt, left2 + 1, left2 + 1, left2 + 2);
                    }
                } else if (state->lineCap == splashLineCapProjecting) {
                    if (last) {
                        pathOut->addStrokeAdjustHint(firstPt + 1, left2 + 2, firstPt + 1, firstPt + 2);
                        pathOut->addStrokeAdjustHint(firstPt + 1, left2 + 2, left2 + 2, left2 + 3);
                    } else {
                        pathOut->addStrokeAdjustHint(firstPt + 1, left2 + 1, firstPt + 1, firstPt + 2);
                    }
                }
            }
            if (seg >= 1) {
                if (seg >= 2) {
                    pathOut->addStrokeAdjustHint(left1, right1, left0 + 1, right0);
                    pathOut->addStrokeAdjustHint(left1, right1, join0, left2);
                } else {
                    pathOut->addStrokeAdjustHint(left1, right1, firstPt, left2);
                }
                pathOut->addStrokeAdjustHint(left1, right1, right2 + 1, right2 + 1);
            }
            left0 = left1;
            left1 = left2;
            right0 = right1;
            right1 = right2;
            join0 = join1;
            join1 = join2;
            if (seg == 0) {
                leftFirst = left2;
                rightFirst = right2;
            }
            if (last) {
                if (seg >= 2) {
                    pathOut->addStrokeAdjustHint(left1, right1, left0 + 1, right0);
                    pathOut->addStrokeAdjustHint(left1, right1, join0, pathOut->length - 1);
                } else {
                    pathOut->addStrokeAdjustHint(left1, right1, firstPt, pathOut->length - 1);
                }
                if (closed) {
                    pathOut->addStrokeAdjustHint(left1, right1, firstPt, leftFirst);
                    pathOut->addStrokeAdjustHint(left1, right1, rightFirst + 1, rightFirst + 1);
                    pathOut->addStrokeAdjustHint(leftFirst, rightFirst, left1 + 1, right1);
                    pathOut->addStrokeAdjustHint(leftFirst, rightFirst, join1, pathOut->length - 1);
                }
                if (!closed && seg > 0) {
                    if (state->lineCap == splashLineCapButt) {
                        pathOut->addStrokeAdjustHint(left1 - 1, left1 + 1, left1 + 1, left1 + 2);
                    } else if (state->lineCap == splashLineCapProjecting) {
                        pathOut->addStrokeAdjustHint(left1 - 1, left1 + 2, left1 + 2, left1 + 3);
                    }
                }
            }
        }

        i0 = j0;
        i1 = j1;
        ++seg;
    }

    if (pathIn != path) {
        delete pathIn;
    }

    return pathOut;
}

void Splash::dumpPath(SplashPath *path)
{
    int i;

    for (i = 0; i < path->length; ++i) {
        printf("  %3d: x=%8.2f y=%8.2f%s%s%s%s\n", i, (double)path->pts[i].x, (double)path->pts[i].y, (path->flags[i] & splashPathFirst) ? " first" : "", (path->flags[i] & splashPathLast) ? " last" : "",
               (path->flags[i] & splashPathClosed) ? " closed" : "", (path->flags[i] & splashPathCurve) ? " curve" : "");
    }
}

void Splash::dumpXPath(SplashXPath *path)
{
    int i;

    for (i = 0; i < path->length; ++i) {
        printf("  %4d: x0=%8.2f y0=%8.2f x1=%8.2f y1=%8.2f %s%s%s\n", i, (double)path->segs[i].x0, (double)path->segs[i].y0, (double)path->segs[i].x1, (double)path->segs[i].y1, (path->segs[i].flags & splashXPathHoriz) ? "H" : " ",
               (path->segs[i].flags & splashXPathVert) ? "V" : " ", (path->segs[i].flags & splashXPathFlip) ? "P" : " ");
    }
}

SplashError Splash::shadedFill(SplashPath *path, bool hasBBox, SplashPattern *pattern, bool clipToStrokePath)
{
    SplashPipe pipe;
    int xMinI, yMinI, xMaxI, yMaxI, x0, x1, y;
    SplashClipResult clipRes;

    if (vectorAntialias && aaBuf == nullptr) { // should not happen, but to be secure
        return splashErrGeneric;
    }
    if (path->length == 0) {
        return splashErrEmptyPath;
    }
    SplashXPath xPath(path, state->matrix, state->flatness, true);
    if (vectorAntialias) {
        xPath.aaScale();
    }
    xPath.sort();
    yMinI = state->clip->getYMinI();
    yMaxI = state->clip->getYMaxI();
    if (vectorAntialias && !inShading) {
        yMinI = yMinI * splashAASize;
        yMaxI = (yMaxI + 1) * splashAASize - 1;
    }
    SplashXPathScanner scanner(xPath, false, yMinI, yMaxI);

    // get the min and max x and y values
    if (vectorAntialias) {
        scanner.getBBoxAA(&xMinI, &yMinI, &xMaxI, &yMaxI);
    } else {
        scanner.getBBox(&xMinI, &yMinI, &xMaxI, &yMaxI);
    }

    // check clipping
    if ((clipRes = state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI)) != splashClipAllOutside) {
        // limit the y range
        if (yMinI < state->clip->getYMinI()) {
            yMinI = state->clip->getYMinI();
        }
        if (yMaxI > state->clip->getYMaxI()) {
            yMaxI = state->clip->getYMaxI();
        }

        unsigned char alpha = splashRound((clipToStrokePath) ? state->strokeAlpha * 255 : state->fillAlpha * 255);
        pipeInit(&pipe, 0, yMinI, pattern, nullptr, alpha, vectorAntialias && !hasBBox, false);

        // draw the spans
        if (vectorAntialias) {
            for (y = yMinI; y <= yMaxI; ++y) {
                scanner.renderAALine(aaBuf, &x0, &x1, y);
                if (clipRes != splashClipAllInside) {
                    state->clip->clipAALine(aaBuf, &x0, &x1, y);
                }
#if splashAASize == 4
                if (!hasBBox && y > yMinI && y < yMaxI) {
                    // correct shape on left side if clip is
                    // vertical through the middle of shading:
                    unsigned char *p0, *p1, *p2, *p3;
                    unsigned char c1, c2, c3, c4;
                    p0 = aaBuf->getDataPtr() + (x0 >> 1);
                    p1 = p0 + aaBuf->getRowSize();
                    p2 = p1 + aaBuf->getRowSize();
                    p3 = p2 + aaBuf->getRowSize();
                    if (x0 & 1) {
                        c1 = (*p0 & 0x0f);
                        c2 = (*p1 & 0x0f);
                        c3 = (*p2 & 0x0f);
                        c4 = (*p3 & 0x0f);
                    } else {
                        c1 = (*p0 >> 4);
                        c2 = (*p1 >> 4);
                        c3 = (*p2 >> 4);
                        c4 = (*p3 >> 4);
                    }
                    if ((c1 & 0x03) == 0x03 && (c2 & 0x03) == 0x03 && (c3 & 0x03) == 0x03 && (c4 & 0x03) == 0x03 && c1 == c2 && c2 == c3 && c3 == c4 && pattern->testPosition(x0 - 1, y)) {
                        unsigned char shapeCorrection = (x0 & 1) ? 0x0f : 0xf0;
                        *p0 |= shapeCorrection;
                        *p1 |= shapeCorrection;
                        *p2 |= shapeCorrection;
                        *p3 |= shapeCorrection;
                    }
                    // correct shape on right side if clip is
                    // through the middle of shading:
                    p0 = aaBuf->getDataPtr() + (x1 >> 1);
                    p1 = p0 + aaBuf->getRowSize();
                    p2 = p1 + aaBuf->getRowSize();
                    p3 = p2 + aaBuf->getRowSize();
                    if (x1 & 1) {
                        c1 = (*p0 & 0x0f);
                        c2 = (*p1 & 0x0f);
                        c3 = (*p2 & 0x0f);
                        c4 = (*p3 & 0x0f);
                    } else {
                        c1 = (*p0 >> 4);
                        c2 = (*p1 >> 4);
                        c3 = (*p2 >> 4);
                        c4 = (*p3 >> 4);
                    }

                    if ((c1 & 0xc) == 0x0c && (c2 & 0x0c) == 0x0c && (c3 & 0x0c) == 0x0c && (c4 & 0x0c) == 0x0c && c1 == c2 && c2 == c3 && c3 == c4 && pattern->testPosition(x1 + 1, y)) {
                        unsigned char shapeCorrection = (x1 & 1) ? 0x0f : 0xf0;
                        *p0 |= shapeCorrection;
                        *p1 |= shapeCorrection;
                        *p2 |= shapeCorrection;
                        *p3 |= shapeCorrection;
                    }
                }
#endif
                drawAALine(&pipe, x0, x1, y);
            }
        } else {
            SplashClipResult clipRes2;
            for (y = yMinI; y <= yMaxI; ++y) {
                SplashXPathScanIterator iterator(scanner, y);
                while (iterator.getNextSpan(&x0, &x1)) {
                    if (clipRes == splashClipAllInside) {
                        drawSpan(&pipe, x0, x1, y, true);
                    } else {
                        // limit the x range
                        if (x0 < state->clip->getXMinI()) {
                            x0 = state->clip->getXMinI();
                        }
                        if (x1 > state->clip->getXMaxI()) {
                            x1 = state->clip->getXMaxI();
                        }
                        clipRes2 = state->clip->testSpan(x0, x1, y);
                        drawSpan(&pipe, x0, x1, y, clipRes2 == splashClipAllInside);
                    }
                }
            }
        }
    }
    opClipRes = clipRes;

    return splashOk;
}
