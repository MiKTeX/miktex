//========================================================================
//
// SplashState.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2009, 2011, 2012, 2015 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2019, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2020 Peter Wang <novalazy@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstring>
#include "goo/gmem.h"
#include "SplashPattern.h"
#include "SplashScreen.h"
#include "SplashClip.h"
#include "SplashBitmap.h"
#include "SplashState.h"

//------------------------------------------------------------------------
// SplashState
//------------------------------------------------------------------------

// number of components in each color mode
int splashColorModeNComps[] = { 1, 1, 3, 3, 4, 4, 4 + SPOT_NCOMPS };

SplashState::SplashState(int width, int height, bool vectorAntialias, SplashScreenParams *screenParams)
{
    SplashColor color;
    int i;

    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 1;
    matrix[4] = 0;
    matrix[5] = 0;
    memset(&color, 0, sizeof(SplashColor));
    strokePattern = new SplashSolidColor(color);
    fillPattern = new SplashSolidColor(color);
    screen = new SplashScreen(screenParams);
    blendFunc = nullptr;
    strokeAlpha = 1;
    fillAlpha = 1;
    multiplyPatternAlpha = false;
    patternStrokeAlpha = 1;
    patternFillAlpha = 1;
    lineWidth = 1;
    lineCap = splashLineCapButt;
    lineJoin = splashLineJoinMiter;
    miterLimit = 10;
    flatness = 1;
    lineDashPhase = 0;
    strokeAdjust = false;
    clip = new SplashClip(0, 0, width - 0.001, height - 0.001, vectorAntialias);
    softMask = nullptr;
    deleteSoftMask = false;
    inNonIsolatedGroup = false;
    fillOverprint = false;
    strokeOverprint = false;
    overprintMode = 0;
    for (i = 0; i < 256; ++i) {
        rgbTransferR[i] = (unsigned char)i;
        rgbTransferG[i] = (unsigned char)i;
        rgbTransferB[i] = (unsigned char)i;
        grayTransfer[i] = (unsigned char)i;
        cmykTransferC[i] = (unsigned char)i;
        cmykTransferM[i] = (unsigned char)i;
        cmykTransferY[i] = (unsigned char)i;
        cmykTransferK[i] = (unsigned char)i;
        for (auto &cp : deviceNTransfer) {
            cp[i] = (unsigned char)i;
        }
    }
    overprintMask = 0xffffffff;
    overprintAdditive = false;
    next = nullptr;
}

SplashState::SplashState(int width, int height, bool vectorAntialias, SplashScreen *screenA)
{
    SplashColor color;
    int i;

    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 1;
    matrix[4] = 0;
    matrix[5] = 0;
    memset(&color, 0, sizeof(SplashColor));
    strokePattern = new SplashSolidColor(color);
    fillPattern = new SplashSolidColor(color);
    screen = screenA->copy();
    blendFunc = nullptr;
    strokeAlpha = 1;
    fillAlpha = 1;
    multiplyPatternAlpha = false;
    patternStrokeAlpha = 1;
    patternFillAlpha = 1;
    lineWidth = 1;
    lineCap = splashLineCapButt;
    lineJoin = splashLineJoinMiter;
    miterLimit = 10;
    flatness = 1;
    lineDashPhase = 0;
    strokeAdjust = false;
    clip = new SplashClip(0, 0, width - 0.001, height - 0.001, vectorAntialias);
    softMask = nullptr;
    deleteSoftMask = false;
    inNonIsolatedGroup = false;
    fillOverprint = false;
    strokeOverprint = false;
    overprintMode = 0;
    for (i = 0; i < 256; ++i) {
        rgbTransferR[i] = (unsigned char)i;
        rgbTransferG[i] = (unsigned char)i;
        rgbTransferB[i] = (unsigned char)i;
        grayTransfer[i] = (unsigned char)i;
        cmykTransferC[i] = (unsigned char)i;
        cmykTransferM[i] = (unsigned char)i;
        cmykTransferY[i] = (unsigned char)i;
        cmykTransferK[i] = (unsigned char)i;
        for (auto &cp : deviceNTransfer) {
            cp[i] = (unsigned char)i;
        }
    }
    overprintMask = 0xffffffff;
    overprintAdditive = false;
    next = nullptr;
}

SplashState::SplashState(const SplashState *state)
{
    memcpy(matrix, state->matrix, 6 * sizeof(SplashCoord));
    strokePattern = state->strokePattern->copy();
    fillPattern = state->fillPattern->copy();
    screen = state->screen->copy();
    blendFunc = state->blendFunc;
    strokeAlpha = state->strokeAlpha;
    fillAlpha = state->fillAlpha;
    multiplyPatternAlpha = state->multiplyPatternAlpha;
    patternStrokeAlpha = state->patternStrokeAlpha;
    patternFillAlpha = state->patternFillAlpha;
    lineWidth = state->lineWidth;
    lineCap = state->lineCap;
    lineJoin = state->lineJoin;
    miterLimit = state->miterLimit;
    flatness = state->flatness;
    lineDash = state->lineDash;
    lineDashPhase = state->lineDashPhase;
    strokeAdjust = state->strokeAdjust;
    clip = state->clip->copy();
    softMask = state->softMask;
    deleteSoftMask = false;
    inNonIsolatedGroup = state->inNonIsolatedGroup;
    fillOverprint = state->fillOverprint;
    strokeOverprint = state->strokeOverprint;
    overprintMode = state->overprintMode;
    memcpy(rgbTransferR, state->rgbTransferR, 256);
    memcpy(rgbTransferG, state->rgbTransferG, 256);
    memcpy(rgbTransferB, state->rgbTransferB, 256);
    memcpy(grayTransfer, state->grayTransfer, 256);
    memcpy(cmykTransferC, state->cmykTransferC, 256);
    memcpy(cmykTransferM, state->cmykTransferM, 256);
    memcpy(cmykTransferY, state->cmykTransferY, 256);
    memcpy(cmykTransferK, state->cmykTransferK, 256);
    for (int cp = 0; cp < SPOT_NCOMPS + 4; cp++) {
        memcpy(deviceNTransfer[cp], state->deviceNTransfer[cp], 256);
    }
    overprintMask = state->overprintMask;
    overprintAdditive = state->overprintAdditive;
    next = nullptr;
}

SplashState::~SplashState()
{
    delete strokePattern;
    delete fillPattern;
    delete screen;
    delete clip;
    if (deleteSoftMask && softMask) {
        delete softMask;
    }
}

void SplashState::setStrokePattern(SplashPattern *strokePatternA)
{
    delete strokePattern;
    strokePattern = strokePatternA;
}

void SplashState::setFillPattern(SplashPattern *fillPatternA)
{
    delete fillPattern;
    fillPattern = fillPatternA;
}

void SplashState::setScreen(SplashScreen *screenA)
{
    delete screen;
    screen = screenA;
}

void SplashState::setLineDash(std::vector<SplashCoord> &&lineDashA, SplashCoord lineDashPhaseA)
{
    lineDash = lineDashA;
    lineDashPhase = lineDashPhaseA;
}

void SplashState::setSoftMask(SplashBitmap *softMaskA)
{
    if (deleteSoftMask) {
        delete softMask;
    }
    softMask = softMaskA;
    deleteSoftMask = true;
}

void SplashState::setTransfer(unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *gray)
{
    for (int i = 0; i < 256; ++i) {
        cmykTransferC[i] = 255 - rgbTransferR[255 - i];
        cmykTransferM[i] = 255 - rgbTransferG[255 - i];
        cmykTransferY[i] = 255 - rgbTransferB[255 - i];
        cmykTransferK[i] = 255 - grayTransfer[255 - i];
    }
    for (int i = 0; i < 256; ++i) {
        deviceNTransfer[0][i] = 255 - rgbTransferR[255 - i];
        deviceNTransfer[1][i] = 255 - rgbTransferG[255 - i];
        deviceNTransfer[2][i] = 255 - rgbTransferB[255 - i];
        deviceNTransfer[3][i] = 255 - grayTransfer[255 - i];
    }
    memcpy(rgbTransferR, red, 256);
    memcpy(rgbTransferG, green, 256);
    memcpy(rgbTransferB, blue, 256);
    memcpy(grayTransfer, gray, 256);
}
