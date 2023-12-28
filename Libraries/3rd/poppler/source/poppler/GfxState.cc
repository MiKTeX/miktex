//========================================================================
//
// GfxState.cc
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
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006, 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2006, 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2006-2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009, 2012 Koji Otani <sho@bbr.jp>
// Copyright (C) 2009, 2011-2016, 2020, 2023 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009, 2019 Christian Persch <chpe@gnome.org>
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011 Andrea Canciani <ranma42@gmail.com>
// Copyright (C) 2012, 2020 William Bader <williambader@hotmail.com>
// Copyright (C) 2013 Lu Wang <coolwanglu@gmail.com>
// Copyright (C) 2013 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2015, 2020 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2016 Marek Kasik <mkasik@redhat.com>
// Copyright (C) 2017, 2019, 2022 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Volker Krause <vkrause@kde.org>
// Copyright (C) 2018, 2019 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 LE GARREC Vincent <legarrec.vincent@gmail.com>
// Copyright (C) 2020, 2021 Philipp Knechtges <philipp-dev@knechtges.com>
// Copyright (C) 2020 Lluís Batlle i Rossell <viric@viric.name>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <algorithm>
#include <memory>
#include <cstddef>
#include <cmath>
#include <cstring>
#include "goo/gfile.h"
#include "goo/gmem.h"
#include "Error.h"
#include "Object.h"
#include "Array.h"
#include "Page.h"
#include "Gfx.h"
#include "GfxState.h"
#include "GfxState_helpers.h"
#include "GfxFont.h"
#include "GlobalParams.h"
#include "PopplerCache.h"
#include "OutputDev.h"
#include "splash/SplashTypes.h"

//------------------------------------------------------------------------

// Max depth of nested color spaces.  This is used to catch infinite
// loops in the color space object structure.
#define colorSpaceRecursionLimit 8

//------------------------------------------------------------------------

bool Matrix::invertTo(Matrix *other) const
{
    const double det_denominator = determinant();
    if (unlikely(det_denominator == 0)) {
        *other = { 1, 0, 0, 1, 0, 0 };
        return false;
    }

    const double det = 1 / det_denominator;
    other->m[0] = m[3] * det;
    other->m[1] = -m[1] * det;
    other->m[2] = -m[2] * det;
    other->m[3] = m[0] * det;
    other->m[4] = (m[2] * m[5] - m[3] * m[4]) * det;
    other->m[5] = (m[1] * m[4] - m[0] * m[5]) * det;

    return true;
}

void Matrix::translate(double tx, double ty)
{
    double x0 = tx * m[0] + ty * m[2] + m[4];
    double y0 = tx * m[1] + ty * m[3] + m[5];
    m[4] = x0;
    m[5] = y0;
}

void Matrix::scale(double sx, double sy)
{
    m[0] *= sx;
    m[1] *= sx;
    m[2] *= sy;
    m[3] *= sy;
}

void Matrix::transform(double x, double y, double *tx, double *ty) const
{
    double temp_x, temp_y;

    temp_x = m[0] * x + m[2] * y + m[4];
    temp_y = m[1] * x + m[3] * y + m[5];

    *tx = temp_x;
    *ty = temp_y;
}

// Matrix norm, taken from _cairo_matrix_transformed_circle_major_axis
double Matrix::norm() const
{
    double f, g, h, i, j;

    i = m[0] * m[0] + m[1] * m[1];
    j = m[2] * m[2] + m[3] * m[3];

    f = 0.5 * (i + j);
    g = 0.5 * (i - j);
    h = m[0] * m[2] + m[1] * m[3];

    return sqrt(f + hypot(g, h));
}

//------------------------------------------------------------------------

struct GfxBlendModeInfo
{
    const char *name;
    GfxBlendMode mode;
};

static const GfxBlendModeInfo gfxBlendModeNames[] = { { "Normal", gfxBlendNormal },         { "Compatible", gfxBlendNormal },
                                                      { "Multiply", gfxBlendMultiply },     { "Screen", gfxBlendScreen },
                                                      { "Overlay", gfxBlendOverlay },       { "Darken", gfxBlendDarken },
                                                      { "Lighten", gfxBlendLighten },       { "ColorDodge", gfxBlendColorDodge },
                                                      { "ColorBurn", gfxBlendColorBurn },   { "HardLight", gfxBlendHardLight },
                                                      { "SoftLight", gfxBlendSoftLight },   { "Difference", gfxBlendDifference },
                                                      { "Exclusion", gfxBlendExclusion },   { "Hue", gfxBlendHue },
                                                      { "Saturation", gfxBlendSaturation }, { "Color", gfxBlendColor },
                                                      { "Luminosity", gfxBlendLuminosity } };

#define nGfxBlendModeNames ((int)((sizeof(gfxBlendModeNames) / sizeof(GfxBlendModeInfo))))

//------------------------------------------------------------------------
//
// NB: This must match the GfxColorSpaceMode enum defined in
// GfxState.h
static const char *gfxColorSpaceModeNames[] = { "DeviceGray", "CalGray", "DeviceRGB", "CalRGB", "DeviceCMYK", "Lab", "ICCBased", "Indexed", "Separation", "DeviceN", "Pattern" };

#define nGfxColorSpaceModes ((sizeof(gfxColorSpaceModeNames) / sizeof(char *)))

#ifdef USE_CMS

static const std::map<unsigned int, unsigned int>::size_type CMSCACHE_LIMIT = 2048;

#    include <lcms2.h>
#    define LCMS_FLAGS cmsFLAGS_NOOPTIMIZE | cmsFLAGS_BLACKPOINTCOMPENSATION

static void lcmsprofiledeleter(void *profile)
{
    cmsCloseProfile(profile);
}

GfxLCMSProfilePtr make_GfxLCMSProfilePtr(void *profile)
{
    if (profile == nullptr) {
        return GfxLCMSProfilePtr();
    }
    return GfxLCMSProfilePtr(profile, lcmsprofiledeleter);
}

void GfxColorTransform::doTransform(void *in, void *out, unsigned int size)
{
    cmsDoTransform(transform, in, out, size);
}

// transformA should be a cmsHTRANSFORM
GfxColorTransform::GfxColorTransform(void *transformA, int cmsIntentA, unsigned int inputPixelTypeA, unsigned int transformPixelTypeA)
{
    transform = transformA;
    cmsIntent = cmsIntentA;
    inputPixelType = inputPixelTypeA;
    transformPixelType = transformPixelTypeA;
}

GfxColorTransform::~GfxColorTransform()
{
    cmsDeleteTransform(transform);
}

// convert color space signature to cmsColor type
static unsigned int getCMSColorSpaceType(cmsColorSpaceSignature cs);
static unsigned int getCMSNChannels(cmsColorSpaceSignature cs);

#endif

//------------------------------------------------------------------------
// GfxColorSpace
//------------------------------------------------------------------------

GfxColorSpace::GfxColorSpace()
{
    overprintMask = 0x0f;
    mapping = nullptr;
}

GfxColorSpace::~GfxColorSpace() { }

GfxColorSpace *GfxColorSpace::parse(GfxResources *res, Object *csObj, OutputDev *out, GfxState *state, int recursion)
{
    GfxColorSpace *cs;
    Object obj1;

    if (recursion > colorSpaceRecursionLimit) {
        error(errSyntaxError, -1, "Loop detected in color space objects");
        return nullptr;
    }

    cs = nullptr;
    if (csObj->isName()) {
        if (csObj->isName("DeviceGray") || csObj->isName("G")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultGray");
                if (objCS.isNull()) {
                    cs = state->copyDefaultGrayColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultGrayColorSpace();
            }
        } else if (csObj->isName("DeviceRGB") || csObj->isName("RGB")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultRGB");
                if (objCS.isNull()) {
                    cs = state->copyDefaultRGBColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultRGBColorSpace();
            }
        } else if (csObj->isName("DeviceCMYK") || csObj->isName("CMYK")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultCMYK");
                if (objCS.isNull()) {
                    cs = state->copyDefaultCMYKColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultCMYKColorSpace();
            }
        } else if (csObj->isName("Pattern")) {
            cs = new GfxPatternColorSpace(nullptr);
        } else {
            error(errSyntaxWarning, -1, "Bad color space '{0:s}'", csObj->getName());
        }
    } else if (csObj->isArray() && csObj->arrayGetLength() > 0) {
        obj1 = csObj->arrayGet(0);
        if (obj1.isName("DeviceGray") || obj1.isName("G")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultGray");
                if (objCS.isNull()) {
                    cs = state->copyDefaultGrayColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultGrayColorSpace();
            }
        } else if (obj1.isName("DeviceRGB") || obj1.isName("RGB")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultRGB");
                if (objCS.isNull()) {
                    cs = state->copyDefaultRGBColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultRGBColorSpace();
            }
        } else if (obj1.isName("DeviceCMYK") || obj1.isName("CMYK")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultCMYK");
                if (objCS.isNull()) {
                    cs = state->copyDefaultCMYKColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultCMYKColorSpace();
            }
        } else if (obj1.isName("CalGray")) {
            cs = GfxCalGrayColorSpace::parse(csObj->getArray(), state);
        } else if (obj1.isName("CalRGB")) {
            cs = GfxCalRGBColorSpace::parse(csObj->getArray(), state);
        } else if (obj1.isName("Lab")) {
            cs = GfxLabColorSpace::parse(csObj->getArray(), state);
        } else if (obj1.isName("ICCBased")) {
            cs = GfxICCBasedColorSpace::parse(csObj->getArray(), out, state, recursion);
        } else if (obj1.isName("Indexed") || obj1.isName("I")) {
            cs = GfxIndexedColorSpace::parse(res, csObj->getArray(), out, state, recursion);
        } else if (obj1.isName("Separation")) {
            cs = GfxSeparationColorSpace::parse(res, csObj->getArray(), out, state, recursion);
        } else if (obj1.isName("DeviceN")) {
            cs = GfxDeviceNColorSpace::parse(res, csObj->getArray(), out, state, recursion);
        } else if (obj1.isName("Pattern")) {
            cs = GfxPatternColorSpace::parse(res, csObj->getArray(), out, state, recursion);
        } else {
            error(errSyntaxWarning, -1, "Bad color space");
        }
    } else if (csObj->isDict()) {
        obj1 = csObj->dictLookup("ColorSpace");
        if (obj1.isName("DeviceGray")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultGray");
                if (objCS.isNull()) {
                    cs = state->copyDefaultGrayColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultGrayColorSpace();
            }
        } else if (obj1.isName("DeviceRGB")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultRGB");
                if (objCS.isNull()) {
                    cs = state->copyDefaultRGBColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultRGBColorSpace();
            }
        } else if (obj1.isName("DeviceCMYK")) {
            if (res != nullptr) {
                Object objCS = res->lookupColorSpace("DefaultCMYK");
                if (objCS.isNull()) {
                    cs = state->copyDefaultCMYKColorSpace();
                } else {
                    cs = GfxColorSpace::parse(nullptr, &objCS, out, state);
                }
            } else {
                cs = state->copyDefaultCMYKColorSpace();
            }
        } else {
            error(errSyntaxWarning, -1, "Bad color space dict'");
        }
    } else {
        error(errSyntaxWarning, -1, "Bad color space - expected name or array or dict");
    }
    return cs;
}

void GfxColorSpace::createMapping(std::vector<GfxSeparationColorSpace *> *separationList, int maxSepComps)
{
    return;
}

void GfxColorSpace::getDefaultRanges(double *decodeLow, double *decodeRange, int maxImgPixel) const
{
    int i;

    for (i = 0; i < getNComps(); ++i) {
        decodeLow[i] = 0;
        decodeRange[i] = 1;
    }
}

int GfxColorSpace::getNumColorSpaceModes()
{
    return nGfxColorSpaceModes;
}

const char *GfxColorSpace::getColorSpaceModeName(int idx)
{
    return gfxColorSpaceModeNames[idx];
}

#ifdef USE_CMS

static void CMSError(cmsContext /*contextId*/, cmsUInt32Number /*ecode*/, const char *text)
{
    error(errSyntaxWarning, -1, "{0:s}", text);
}

unsigned int getCMSColorSpaceType(cmsColorSpaceSignature cs)
{
    switch (cs) {
    case cmsSigXYZData:
        return PT_XYZ;
        break;
    case cmsSigLabData:
        return PT_Lab;
        break;
    case cmsSigLuvData:
        return PT_YUV;
        break;
    case cmsSigYCbCrData:
        return PT_YCbCr;
        break;
    case cmsSigYxyData:
        return PT_Yxy;
        break;
    case cmsSigRgbData:
        return PT_RGB;
        break;
    case cmsSigGrayData:
        return PT_GRAY;
        break;
    case cmsSigHsvData:
        return PT_HSV;
        break;
    case cmsSigHlsData:
        return PT_HLS;
        break;
    case cmsSigCmykData:
        return PT_CMYK;
        break;
    case cmsSigCmyData:
        return PT_CMY;
        break;
    case cmsSig2colorData:
    case cmsSig3colorData:
    case cmsSig4colorData:
    case cmsSig5colorData:
    case cmsSig6colorData:
    case cmsSig7colorData:
    case cmsSig8colorData:
    case cmsSig9colorData:
    case cmsSig10colorData:
    case cmsSig11colorData:
    case cmsSig12colorData:
    case cmsSig13colorData:
    case cmsSig14colorData:
    case cmsSig15colorData:
    default:
        break;
    }
    return PT_RGB;
}

unsigned int getCMSNChannels(cmsColorSpaceSignature cs)
{
    switch (cs) {
    case cmsSigXYZData:
    case cmsSigLuvData:
    case cmsSigLabData:
    case cmsSigYCbCrData:
    case cmsSigYxyData:
    case cmsSigRgbData:
    case cmsSigHsvData:
    case cmsSigHlsData:
    case cmsSigCmyData:
    case cmsSig3colorData:
        return 3;
        break;
    case cmsSigGrayData:
        return 1;
        break;
    case cmsSigCmykData:
    case cmsSig4colorData:
        return 4;
        break;
    case cmsSig2colorData:
        return 2;
        break;
    case cmsSig5colorData:
        return 5;
        break;
    case cmsSig6colorData:
        return 6;
        break;
    case cmsSig7colorData:
        return 7;
        break;
    case cmsSig8colorData:
        return 8;
        break;
    case cmsSig9colorData:
        return 9;
        break;
    case cmsSig10colorData:
        return 10;
        break;
    case cmsSig11colorData:
        return 11;
        break;
    case cmsSig12colorData:
        return 12;
        break;
    case cmsSig13colorData:
        return 13;
        break;
    case cmsSig14colorData:
        return 14;
        break;
    case cmsSig15colorData:
        return 15;
    default:
        break;
    }
    return 3;
}
#endif

//------------------------------------------------------------------------
// GfxDeviceGrayColorSpace
//------------------------------------------------------------------------

GfxDeviceGrayColorSpace::GfxDeviceGrayColorSpace() { }

GfxDeviceGrayColorSpace::~GfxDeviceGrayColorSpace() { }

GfxColorSpace *GfxDeviceGrayColorSpace::copy() const
{
    return new GfxDeviceGrayColorSpace();
}

void GfxDeviceGrayColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    *gray = clip01(color->c[0]);
}

void GfxDeviceGrayColorSpace::getGrayLine(unsigned char *in, unsigned char *out, int length)
{
    memcpy(out, in, length);
}

void GfxDeviceGrayColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    rgb->r = rgb->g = rgb->b = clip01(color->c[0]);
}

void GfxDeviceGrayColorSpace::getRGBLine(unsigned char *in, unsigned int *out, int length)
{
    int i;

    for (i = 0; i < length; i++) {
        out[i] = (in[i] << 16) | (in[i] << 8) | (in[i] << 0);
    }
}

void GfxDeviceGrayColorSpace::getRGBLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        *out++ = in[i];
        *out++ = in[i];
        *out++ = in[i];
    }
}

void GfxDeviceGrayColorSpace::getRGBXLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        *out++ = in[i];
        *out++ = in[i];
        *out++ = in[i];
        *out++ = 255;
    }
}

void GfxDeviceGrayColorSpace::getCMYKLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        *out++ = 0;
        *out++ = 0;
        *out++ = 0;
        *out++ = in[i];
    }
}

void GfxDeviceGrayColorSpace::getDeviceNLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < SPOT_NCOMPS + 4; j++) {
            out[j] = 0;
        }
        out[4] = in[i];
        out += (SPOT_NCOMPS + 4);
    }
}

void GfxDeviceGrayColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    cmyk->c = cmyk->m = cmyk->y = 0;
    cmyk->k = clip01(gfxColorComp1 - color->c[0]);
}

void GfxDeviceGrayColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    clearGfxColor(deviceN);
    deviceN->c[3] = clip01(gfxColorComp1 - color->c[0]);
}

void GfxDeviceGrayColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
}

//------------------------------------------------------------------------
// GfxCalGrayColorSpace
//------------------------------------------------------------------------

GfxCalGrayColorSpace::GfxCalGrayColorSpace()
{
    whiteX = whiteY = whiteZ = 1;
    blackX = blackY = blackZ = 0;
    gamma = 1;
}

GfxCalGrayColorSpace::~GfxCalGrayColorSpace() { }

GfxColorSpace *GfxCalGrayColorSpace::copy() const
{
    GfxCalGrayColorSpace *cs;

    cs = new GfxCalGrayColorSpace();
    cs->whiteX = whiteX;
    cs->whiteY = whiteY;
    cs->whiteZ = whiteZ;
    cs->blackX = blackX;
    cs->blackY = blackY;
    cs->blackZ = blackZ;
    cs->gamma = gamma;
#ifdef USE_CMS
    cs->transform = transform;
#endif
    return cs;
}

// This is the inverse of MatrixLMN in Example 4.10 from the PostScript
// Language Reference, Third Edition.
static const double xyzrgb[3][3] = { { 3.240449, -1.537136, -0.498531 }, { -0.969265, 1.876011, 0.041556 }, { 0.055643, -0.204026, 1.057229 } };

// From the same reference as above, the inverse of the DecodeLMN function.
// This is essentially the gamma function of the sRGB profile.
static double srgb_gamma_function(double x)
{
    // 0.04045 is what lcms2 uses, but the PS Reference Example 4.10 specifies 0.03928???
    // if (x <= 0.04045 / 12.92321) {
    if (x <= 0.03928 / 12.92321) {
        return x * 12.92321;
    }
    return 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

// D65 is the white point of the sRGB profile as it is specified above in the xyzrgb array
static const double white_d65_X = 0.9505;
static const double white_d65_Y = 1.0;
static const double white_d65_Z = 1.0890;

#ifdef USE_CMS
// D50 is the default white point as used in ICC profiles and in the lcms2 library
static const double white_d50_X = 0.96422;
static const double white_d50_Y = 1.0;
static const double white_d50_Z = 0.82521;

static void inline bradford_transform_to_d50(double &X, double &Y, double &Z, const double source_whiteX, const double source_whiteY, const double source_whiteZ)
{
    if (source_whiteX == white_d50_X && source_whiteY == white_d50_Y && source_whiteZ == white_d50_Z) {
        // early exit if noop
        return;
    }
    // at first apply Bradford matrix
    double rho_in = 0.8951000 * X + 0.2664000 * Y - 0.1614000 * Z;
    double gamma_in = -0.7502000 * X + 1.7135000 * Y + 0.0367000 * Z;
    double beta_in = 0.0389000 * X - 0.0685000 * Y + 1.0296000 * Z;

    // apply a diagonal matrix with the diagonal entries being the inverse bradford-transformed white point
    rho_in /= 0.8951000 * source_whiteX + 0.2664000 * source_whiteY - 0.1614000 * source_whiteZ;
    gamma_in /= -0.7502000 * source_whiteX + 1.7135000 * source_whiteY + 0.0367000 * source_whiteZ;
    beta_in /= 0.0389000 * source_whiteX - 0.0685000 * source_whiteY + 1.0296000 * source_whiteZ;

    // now revert the two steps above, but substituting the source white point by the device white point (D50)
    // Since the white point is known a priori this has been combined into a single operation.
    X = 0.98332566 * rho_in - 0.15005819 * gamma_in + 0.13095252 * beta_in;
    Y = 0.43069901 * rho_in + 0.52894900 * gamma_in + 0.04035199 * beta_in;
    Z = 0.00849698 * rho_in + 0.04086079 * gamma_in + 0.79284618 * beta_in;
}
#endif

static void inline bradford_transform_to_d65(double &X, double &Y, double &Z, const double source_whiteX, const double source_whiteY, const double source_whiteZ)
{
    if (source_whiteX == white_d65_X && source_whiteY == white_d65_Y && source_whiteZ == white_d65_Z) {
        // early exit if noop
        return;
    }
    // at first apply Bradford matrix
    double rho_in = 0.8951000 * X + 0.2664000 * Y - 0.1614000 * Z;
    double gamma_in = -0.7502000 * X + 1.7135000 * Y + 0.0367000 * Z;
    double beta_in = 0.0389000 * X - 0.0685000 * Y + 1.0296000 * Z;

    // apply a diagonal matrix with the diagonal entries being the inverse bradford-transformed white point
    rho_in /= 0.8951000 * source_whiteX + 0.2664000 * source_whiteY - 0.1614000 * source_whiteZ;
    gamma_in /= -0.7502000 * source_whiteX + 1.7135000 * source_whiteY + 0.0367000 * source_whiteZ;
    beta_in /= 0.0389000 * source_whiteX - 0.0685000 * source_whiteY + 1.0296000 * source_whiteZ;

    // now revert the two steps above, but substituting the source white point by the device white point (D65)
    // Since the white point is known a priori this has been combined into a single operation.
    X = 0.92918329 * rho_in - 0.15299782 * gamma_in + 0.17428453 * beta_in;
    Y = 0.40698452 * rho_in + 0.53931108 * gamma_in + 0.05370440 * beta_in;
    Z = -0.00802913 * rho_in + 0.04166125 * gamma_in + 1.05519788 * beta_in;
}

GfxColorSpace *GfxCalGrayColorSpace::parse(Array *arr, GfxState *state)
{
    GfxCalGrayColorSpace *cs;
    Object obj1, obj2;

    obj1 = arr->get(1);
    if (!obj1.isDict()) {
        error(errSyntaxWarning, -1, "Bad CalGray color space");
        return nullptr;
    }
    cs = new GfxCalGrayColorSpace();
    obj2 = obj1.dictLookup("WhitePoint");
    if (obj2.isArray() && obj2.arrayGetLength() == 3) {
        cs->whiteX = obj2.arrayGet(0).getNumWithDefaultValue(1);
        cs->whiteY = obj2.arrayGet(1).getNumWithDefaultValue(1);
        cs->whiteZ = obj2.arrayGet(2).getNumWithDefaultValue(1);
    }
    obj2 = obj1.dictLookup("BlackPoint");
    if (obj2.isArray() && obj2.arrayGetLength() == 3) {
        cs->blackX = obj2.arrayGet(0).getNumWithDefaultValue(0);
        cs->blackY = obj2.arrayGet(1).getNumWithDefaultValue(0);
        cs->blackZ = obj2.arrayGet(2).getNumWithDefaultValue(0);
    }

    cs->gamma = obj1.dictLookup("Gamma").getNumWithDefaultValue(1);

#ifdef USE_CMS
    cs->transform = (state != nullptr) ? state->getXYZ2DisplayTransform() : nullptr;
#endif
    return cs;
}

// convert CalGray to media XYZ color space
// (not multiply by the white point)
void GfxCalGrayColorSpace::getXYZ(const GfxColor *color, double *pX, double *pY, double *pZ) const
{
    const double A = colToDbl(color->c[0]);
    const double xyzColor = pow(A, gamma);
    *pX = xyzColor;
    *pY = xyzColor;
    *pZ = xyzColor;
}

void GfxCalGrayColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    GfxRGB rgb;

#ifdef USE_CMS
    if (transform && transform->getTransformPixelType() == PT_GRAY) {
        unsigned char out[gfxColorMaxComps];
        double in[gfxColorMaxComps];
        double X, Y, Z;

        getXYZ(color, &X, &Y, &Z);
        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        *gray = byteToCol(out[0]);
        return;
    }
#endif
    getRGB(color, &rgb);
    *gray = clip01((GfxColorComp)(0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b + 0.5));
}

void GfxCalGrayColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    double X, Y, Z;
    double r, g, b;

    getXYZ(color, &X, &Y, &Z);
#ifdef USE_CMS
    if (transform && transform->getTransformPixelType() == PT_RGB) {
        unsigned char out[gfxColorMaxComps];
        double in[gfxColorMaxComps];

        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        rgb->r = byteToCol(out[0]);
        rgb->g = byteToCol(out[1]);
        rgb->b = byteToCol(out[2]);
        return;
    }
#endif
    bradford_transform_to_d65(X, Y, Z, whiteX, whiteY, whiteZ);
    // convert XYZ to RGB, including gamut mapping and gamma correction
    r = xyzrgb[0][0] * X + xyzrgb[0][1] * Y + xyzrgb[0][2] * Z;
    g = xyzrgb[1][0] * X + xyzrgb[1][1] * Y + xyzrgb[1][2] * Z;
    b = xyzrgb[2][0] * X + xyzrgb[2][1] * Y + xyzrgb[2][2] * Z;
    rgb->r = dblToCol(srgb_gamma_function(clip01(r)));
    rgb->g = dblToCol(srgb_gamma_function(clip01(g)));
    rgb->b = dblToCol(srgb_gamma_function(clip01(b)));
}

void GfxCalGrayColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    GfxRGB rgb;
    GfxColorComp c, m, y, k;

#ifdef USE_CMS
    if (transform && transform->getTransformPixelType() == PT_CMYK) {
        double in[gfxColorMaxComps];
        unsigned char out[gfxColorMaxComps];
        double X, Y, Z;

        getXYZ(color, &X, &Y, &Z);
        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        cmyk->c = byteToCol(out[0]);
        cmyk->m = byteToCol(out[1]);
        cmyk->y = byteToCol(out[2]);
        cmyk->k = byteToCol(out[3]);
        return;
    }
#endif
    getRGB(color, &rgb);
    c = clip01(gfxColorComp1 - rgb.r);
    m = clip01(gfxColorComp1 - rgb.g);
    y = clip01(gfxColorComp1 - rgb.b);
    k = c;
    if (m < k) {
        k = m;
    }
    if (y < k) {
        k = y;
    }
    cmyk->c = c - k;
    cmyk->m = m - k;
    cmyk->y = y - k;
    cmyk->k = k;
}

void GfxCalGrayColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    GfxCMYK cmyk;
    clearGfxColor(deviceN);
    getCMYK(color, &cmyk);
    deviceN->c[0] = cmyk.c;
    deviceN->c[1] = cmyk.m;
    deviceN->c[2] = cmyk.y;
    deviceN->c[3] = cmyk.k;
}

void GfxCalGrayColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
}

//------------------------------------------------------------------------
// GfxDeviceRGBColorSpace
//------------------------------------------------------------------------

GfxDeviceRGBColorSpace::GfxDeviceRGBColorSpace() { }

GfxDeviceRGBColorSpace::~GfxDeviceRGBColorSpace() { }

GfxColorSpace *GfxDeviceRGBColorSpace::copy() const
{
    return new GfxDeviceRGBColorSpace();
}

void GfxDeviceRGBColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    *gray = clip01((GfxColorComp)(0.3 * color->c[0] + 0.59 * color->c[1] + 0.11 * color->c[2] + 0.5));
}

void GfxDeviceRGBColorSpace::getGrayLine(unsigned char *in, unsigned char *out, int length)
{
    int i;

    for (i = 0; i < length; i++) {
        out[i] = (in[i * 3 + 0] * 19595 + in[i * 3 + 1] * 38469 + in[i * 3 + 2] * 7472) / 65536;
    }
}

void GfxDeviceRGBColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    rgb->r = clip01(color->c[0]);
    rgb->g = clip01(color->c[1]);
    rgb->b = clip01(color->c[2]);
}

void GfxDeviceRGBColorSpace::getRGBLine(unsigned char *in, unsigned int *out, int length)
{
    unsigned char *p;
    int i;

    for (i = 0, p = in; i < length; i++, p += 3) {
        out[i] = (p[0] << 16) | (p[1] << 8) | (p[2] << 0);
    }
}

void GfxDeviceRGBColorSpace::getRGBLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        *out++ = *in++;
        *out++ = *in++;
        *out++ = *in++;
    }
}

void GfxDeviceRGBColorSpace::getRGBXLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        *out++ = *in++;
        *out++ = *in++;
        *out++ = *in++;
        *out++ = 255;
    }
}

void GfxDeviceRGBColorSpace::getCMYKLine(unsigned char *in, unsigned char *out, int length)
{
    GfxColorComp c, m, y, k;

    for (int i = 0; i < length; i++) {
        c = byteToCol(255 - *in++);
        m = byteToCol(255 - *in++);
        y = byteToCol(255 - *in++);
        k = c;
        if (m < k) {
            k = m;
        }
        if (y < k) {
            k = y;
        }
        *out++ = colToByte(c - k);
        *out++ = colToByte(m - k);
        *out++ = colToByte(y - k);
        *out++ = colToByte(k);
    }
}

void GfxDeviceRGBColorSpace::getDeviceNLine(unsigned char *in, unsigned char *out, int length)
{
    GfxColorComp c, m, y, k;

    for (int i = 0; i < length; i++) {
        for (int j = 0; j < SPOT_NCOMPS + 4; j++) {
            out[j] = 0;
        }
        c = byteToCol(255 - *in++);
        m = byteToCol(255 - *in++);
        y = byteToCol(255 - *in++);
        k = c;
        if (m < k) {
            k = m;
        }
        if (y < k) {
            k = y;
        }
        out[0] = colToByte(c - k);
        out[1] = colToByte(m - k);
        out[2] = colToByte(y - k);
        out[3] = colToByte(k);
        out += (SPOT_NCOMPS + 4);
    }
}

void GfxDeviceRGBColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    GfxColorComp c, m, y, k;

    c = clip01(gfxColorComp1 - color->c[0]);
    m = clip01(gfxColorComp1 - color->c[1]);
    y = clip01(gfxColorComp1 - color->c[2]);
    k = c;
    if (m < k) {
        k = m;
    }
    if (y < k) {
        k = y;
    }
    cmyk->c = c - k;
    cmyk->m = m - k;
    cmyk->y = y - k;
    cmyk->k = k;
}

void GfxDeviceRGBColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    GfxCMYK cmyk;
    clearGfxColor(deviceN);
    getCMYK(color, &cmyk);
    deviceN->c[0] = cmyk.c;
    deviceN->c[1] = cmyk.m;
    deviceN->c[2] = cmyk.y;
    deviceN->c[3] = cmyk.k;
}

void GfxDeviceRGBColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
    color->c[1] = 0;
    color->c[2] = 0;
}

//------------------------------------------------------------------------
// GfxCalRGBColorSpace
//------------------------------------------------------------------------

GfxCalRGBColorSpace::GfxCalRGBColorSpace()
{
    whiteX = whiteY = whiteZ = 1;
    blackX = blackY = blackZ = 0;
    gammaR = gammaG = gammaB = 1;
    mat[0] = 1;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;
    mat[4] = 1;
    mat[5] = 0;
    mat[6] = 0;
    mat[7] = 0;
    mat[8] = 1;
}

GfxCalRGBColorSpace::~GfxCalRGBColorSpace() { }

GfxColorSpace *GfxCalRGBColorSpace::copy() const
{
    GfxCalRGBColorSpace *cs;
    int i;

    cs = new GfxCalRGBColorSpace();
    cs->whiteX = whiteX;
    cs->whiteY = whiteY;
    cs->whiteZ = whiteZ;
    cs->blackX = blackX;
    cs->blackY = blackY;
    cs->blackZ = blackZ;
    cs->gammaR = gammaR;
    cs->gammaG = gammaG;
    cs->gammaB = gammaB;
    for (i = 0; i < 9; ++i) {
        cs->mat[i] = mat[i];
    }
#ifdef USE_CMS
    cs->transform = transform;
#endif
    return cs;
}

GfxColorSpace *GfxCalRGBColorSpace::parse(Array *arr, GfxState *state)
{
    GfxCalRGBColorSpace *cs;
    Object obj1, obj2;
    int i;

    obj1 = arr->get(1);
    if (!obj1.isDict()) {
        error(errSyntaxWarning, -1, "Bad CalRGB color space");
        return nullptr;
    }
    cs = new GfxCalRGBColorSpace();
    obj2 = obj1.dictLookup("WhitePoint");
    if (obj2.isArray() && obj2.arrayGetLength() == 3) {
        cs->whiteX = obj2.arrayGet(0).getNumWithDefaultValue(1);
        cs->whiteY = obj2.arrayGet(1).getNumWithDefaultValue(1);
        cs->whiteZ = obj2.arrayGet(2).getNumWithDefaultValue(1);
    }
    obj2 = obj1.dictLookup("BlackPoint");
    if (obj2.isArray() && obj2.arrayGetLength() == 3) {
        cs->blackX = obj2.arrayGet(0).getNumWithDefaultValue(0);
        cs->blackY = obj2.arrayGet(1).getNumWithDefaultValue(0);
        cs->blackZ = obj2.arrayGet(2).getNumWithDefaultValue(0);
    }
    obj2 = obj1.dictLookup("Gamma");
    if (obj2.isArray() && obj2.arrayGetLength() == 3) {
        cs->gammaR = obj2.arrayGet(0).getNumWithDefaultValue(1);
        cs->gammaG = obj2.arrayGet(1).getNumWithDefaultValue(1);
        cs->gammaB = obj2.arrayGet(2).getNumWithDefaultValue(1);
    }
    obj2 = obj1.dictLookup("Matrix");
    if (obj2.isArray() && obj2.arrayGetLength() == 9) {
        for (i = 0; i < 9; ++i) {
            Object obj3 = obj2.arrayGet(i);
            if (likely(obj3.isNum())) {
                cs->mat[i] = obj3.getNum();
            }
        }
    }

#ifdef USE_CMS
    cs->transform = (state != nullptr) ? state->getXYZ2DisplayTransform() : nullptr;
#endif
    return cs;
}

// convert CalRGB to XYZ color space
void GfxCalRGBColorSpace::getXYZ(const GfxColor *color, double *pX, double *pY, double *pZ) const
{
    double A, B, C;

    A = pow(colToDbl(color->c[0]), gammaR);
    B = pow(colToDbl(color->c[1]), gammaG);
    C = pow(colToDbl(color->c[2]), gammaB);
    *pX = mat[0] * A + mat[3] * B + mat[6] * C;
    *pY = mat[1] * A + mat[4] * B + mat[7] * C;
    *pZ = mat[2] * A + mat[5] * B + mat[8] * C;
}

void GfxCalRGBColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    GfxRGB rgb;

#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_GRAY) {
        unsigned char out[gfxColorMaxComps];
        double in[gfxColorMaxComps];
        double X, Y, Z;

        getXYZ(color, &X, &Y, &Z);
        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        *gray = byteToCol(out[0]);
        return;
    }
#endif
    getRGB(color, &rgb);
    *gray = clip01((GfxColorComp)(0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b + 0.5));
}

void GfxCalRGBColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    double X, Y, Z;
    double r, g, b;

    getXYZ(color, &X, &Y, &Z);
#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_RGB) {
        unsigned char out[gfxColorMaxComps];
        double in[gfxColorMaxComps];

        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        rgb->r = byteToCol(out[0]);
        rgb->g = byteToCol(out[1]);
        rgb->b = byteToCol(out[2]);

        return;
    }
#endif
    bradford_transform_to_d65(X, Y, Z, whiteX, whiteY, whiteZ);
    // convert XYZ to RGB, including gamut mapping and gamma correction
    r = xyzrgb[0][0] * X + xyzrgb[0][1] * Y + xyzrgb[0][2] * Z;
    g = xyzrgb[1][0] * X + xyzrgb[1][1] * Y + xyzrgb[1][2] * Z;
    b = xyzrgb[2][0] * X + xyzrgb[2][1] * Y + xyzrgb[2][2] * Z;
    rgb->r = dblToCol(srgb_gamma_function(clip01(r)));
    rgb->g = dblToCol(srgb_gamma_function(clip01(g)));
    rgb->b = dblToCol(srgb_gamma_function(clip01(b)));
}

void GfxCalRGBColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    GfxRGB rgb;
    GfxColorComp c, m, y, k;

#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_CMYK) {
        double in[gfxColorMaxComps];
        unsigned char out[gfxColorMaxComps];
        double X, Y, Z;

        getXYZ(color, &X, &Y, &Z);
        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        cmyk->c = byteToCol(out[0]);
        cmyk->m = byteToCol(out[1]);
        cmyk->y = byteToCol(out[2]);
        cmyk->k = byteToCol(out[3]);
        return;
    }
#endif
    getRGB(color, &rgb);
    c = clip01(gfxColorComp1 - rgb.r);
    m = clip01(gfxColorComp1 - rgb.g);
    y = clip01(gfxColorComp1 - rgb.b);
    k = c;
    if (m < k) {
        k = m;
    }
    if (y < k) {
        k = y;
    }
    cmyk->c = c - k;
    cmyk->m = m - k;
    cmyk->y = y - k;
    cmyk->k = k;
}

void GfxCalRGBColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    GfxCMYK cmyk;
    clearGfxColor(deviceN);
    getCMYK(color, &cmyk);
    deviceN->c[0] = cmyk.c;
    deviceN->c[1] = cmyk.m;
    deviceN->c[2] = cmyk.y;
    deviceN->c[3] = cmyk.k;
}

void GfxCalRGBColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
    color->c[1] = 0;
    color->c[2] = 0;
}

//------------------------------------------------------------------------
// GfxDeviceCMYKColorSpace
//------------------------------------------------------------------------

GfxDeviceCMYKColorSpace::GfxDeviceCMYKColorSpace() { }

GfxDeviceCMYKColorSpace::~GfxDeviceCMYKColorSpace() { }

GfxColorSpace *GfxDeviceCMYKColorSpace::copy() const
{
    return new GfxDeviceCMYKColorSpace();
}

void GfxDeviceCMYKColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    *gray = clip01((GfxColorComp)(gfxColorComp1 - color->c[3] - 0.3 * color->c[0] - 0.59 * color->c[1] - 0.11 * color->c[2] + 0.5));
}

void GfxDeviceCMYKColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    double c, m, y, k, c1, m1, y1, k1, r, g, b;

    c = colToDbl(color->c[0]);
    m = colToDbl(color->c[1]);
    y = colToDbl(color->c[2]);
    k = colToDbl(color->c[3]);
    c1 = 1 - c;
    m1 = 1 - m;
    y1 = 1 - y;
    k1 = 1 - k;
    cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
    rgb->r = clip01(dblToCol(r));
    rgb->g = clip01(dblToCol(g));
    rgb->b = clip01(dblToCol(b));
}

static inline void GfxDeviceCMYKColorSpacegetRGBLineHelper(unsigned char *&in, double &r, double &g, double &b)
{
    double c, m, y, k, c1, m1, y1, k1;

    c = byteToDbl(*in++);
    m = byteToDbl(*in++);
    y = byteToDbl(*in++);
    k = byteToDbl(*in++);
    c1 = 1 - c;
    m1 = 1 - m;
    y1 = 1 - y;
    k1 = 1 - k;
    cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
}

void GfxDeviceCMYKColorSpace::getRGBLine(unsigned char *in, unsigned int *out, int length)
{
    double r, g, b;
    for (int i = 0; i < length; i++) {
        GfxDeviceCMYKColorSpacegetRGBLineHelper(in, r, g, b);
        *out++ = (dblToByte(clip01(r)) << 16) | (dblToByte(clip01(g)) << 8) | dblToByte(clip01(b));
    }
}

void GfxDeviceCMYKColorSpace::getRGBLine(unsigned char *in, unsigned char *out, int length)
{
    double r, g, b;

    for (int i = 0; i < length; i++) {
        GfxDeviceCMYKColorSpacegetRGBLineHelper(in, r, g, b);
        *out++ = dblToByte(clip01(r));
        *out++ = dblToByte(clip01(g));
        *out++ = dblToByte(clip01(b));
    }
}

void GfxDeviceCMYKColorSpace::getRGBXLine(unsigned char *in, unsigned char *out, int length)
{
    double r, g, b;

    for (int i = 0; i < length; i++) {
        GfxDeviceCMYKColorSpacegetRGBLineHelper(in, r, g, b);
        *out++ = dblToByte(clip01(r));
        *out++ = dblToByte(clip01(g));
        *out++ = dblToByte(clip01(b));
        *out++ = 255;
    }
}

void GfxDeviceCMYKColorSpace::getCMYKLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        *out++ = *in++;
        *out++ = *in++;
        *out++ = *in++;
        *out++ = *in++;
    }
}

void GfxDeviceCMYKColorSpace::getDeviceNLine(unsigned char *in, unsigned char *out, int length)
{
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < SPOT_NCOMPS + 4; j++) {
            out[j] = 0;
        }
        out[0] = *in++;
        out[1] = *in++;
        out[2] = *in++;
        out[3] = *in++;
        out += (SPOT_NCOMPS + 4);
    }
}

void GfxDeviceCMYKColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    cmyk->c = clip01(color->c[0]);
    cmyk->m = clip01(color->c[1]);
    cmyk->y = clip01(color->c[2]);
    cmyk->k = clip01(color->c[3]);
}

void GfxDeviceCMYKColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    clearGfxColor(deviceN);
    deviceN->c[0] = clip01(color->c[0]);
    deviceN->c[1] = clip01(color->c[1]);
    deviceN->c[2] = clip01(color->c[2]);
    deviceN->c[3] = clip01(color->c[3]);
}

void GfxDeviceCMYKColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
    color->c[1] = 0;
    color->c[2] = 0;
    color->c[3] = gfxColorComp1;
}

//------------------------------------------------------------------------
// GfxLabColorSpace
//------------------------------------------------------------------------

GfxLabColorSpace::GfxLabColorSpace()
{
    whiteX = whiteY = whiteZ = 1;
    blackX = blackY = blackZ = 0;
    aMin = bMin = -100;
    aMax = bMax = 100;
}

GfxLabColorSpace::~GfxLabColorSpace() { }

GfxColorSpace *GfxLabColorSpace::copy() const
{
    GfxLabColorSpace *cs;

    cs = new GfxLabColorSpace();
    cs->whiteX = whiteX;
    cs->whiteY = whiteY;
    cs->whiteZ = whiteZ;
    cs->blackX = blackX;
    cs->blackY = blackY;
    cs->blackZ = blackZ;
    cs->aMin = aMin;
    cs->aMax = aMax;
    cs->bMin = bMin;
    cs->bMax = bMax;
#ifdef USE_CMS
    cs->transform = transform;
#endif
    return cs;
}

GfxColorSpace *GfxLabColorSpace::parse(Array *arr, GfxState *state)
{
    GfxLabColorSpace *cs;
    Object obj1, obj2;

    obj1 = arr->get(1);
    if (!obj1.isDict()) {
        error(errSyntaxWarning, -1, "Bad Lab color space");
        return nullptr;
    }
    cs = new GfxLabColorSpace();
    bool ok = true;
    obj2 = obj1.dictLookup("WhitePoint");
    if (obj2.isArray() && obj2.arrayGetLength() == 3) {
        cs->whiteX = obj2.arrayGet(0).getNum(&ok);
        cs->whiteY = obj2.arrayGet(1).getNum(&ok);
        cs->whiteZ = obj2.arrayGet(2).getNum(&ok);
    }
    obj2 = obj1.dictLookup("BlackPoint");
    if (obj2.isArray() && obj2.arrayGetLength() == 3) {
        cs->blackX = obj2.arrayGet(0).getNum(&ok);
        cs->blackY = obj2.arrayGet(1).getNum(&ok);
        cs->blackZ = obj2.arrayGet(2).getNum(&ok);
    }
    obj2 = obj1.dictLookup("Range");
    if (obj2.isArray() && obj2.arrayGetLength() == 4) {
        cs->aMin = obj2.arrayGet(0).getNum(&ok);
        cs->aMax = obj2.arrayGet(1).getNum(&ok);
        cs->bMin = obj2.arrayGet(2).getNum(&ok);
        cs->bMax = obj2.arrayGet(3).getNum(&ok);
    }

    if (!ok) {
        error(errSyntaxWarning, -1, "Bad Lab color space");
#ifdef USE_CMS
        cs->transform = nullptr;
#endif
        delete cs;
        return nullptr;
    }

#ifdef USE_CMS
    cs->transform = (state != nullptr) ? state->getXYZ2DisplayTransform() : nullptr;
#endif
    return cs;
}

void GfxLabColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    GfxRGB rgb;

#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_GRAY) {
        unsigned char out[gfxColorMaxComps];
        double in[gfxColorMaxComps];

        getXYZ(color, &in[0], &in[1], &in[2]);
        bradford_transform_to_d50(in[0], in[1], in[2], whiteX, whiteY, whiteZ);
        transform->doTransform(in, out, 1);
        *gray = byteToCol(out[0]);
        return;
    }
#endif
    getRGB(color, &rgb);
    *gray = clip01((GfxColorComp)(0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b + 0.5));
}

// convert L*a*b* to media XYZ color space
// (not multiply by the white point)
void GfxLabColorSpace::getXYZ(const GfxColor *color, double *pX, double *pY, double *pZ) const
{
    double X, Y, Z;
    double t1, t2;

    t1 = (colToDbl(color->c[0]) + 16) / 116;
    t2 = t1 + colToDbl(color->c[1]) / 500;
    if (t2 >= (6.0 / 29.0)) {
        X = t2 * t2 * t2;
    } else {
        X = (108.0 / 841.0) * (t2 - (4.0 / 29.0));
    }
    if (t1 >= (6.0 / 29.0)) {
        Y = t1 * t1 * t1;
    } else {
        Y = (108.0 / 841.0) * (t1 - (4.0 / 29.0));
    }
    t2 = t1 - colToDbl(color->c[2]) / 200;
    if (t2 >= (6.0 / 29.0)) {
        Z = t2 * t2 * t2;
    } else {
        Z = (108.0 / 841.0) * (t2 - (4.0 / 29.0));
    }
    *pX = X;
    *pY = Y;
    *pZ = Z;
}

void GfxLabColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    double X, Y, Z;

    getXYZ(color, &X, &Y, &Z);
    X *= whiteX;
    Y *= whiteY;
    Z *= whiteZ;
#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_RGB) {
        unsigned char out[gfxColorMaxComps];
        double in[gfxColorMaxComps];

        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        rgb->r = byteToCol(out[0]);
        rgb->g = byteToCol(out[1]);
        rgb->b = byteToCol(out[2]);
        return;
    } else if (transform != nullptr && transform->getTransformPixelType() == PT_CMYK) {
        unsigned char out[gfxColorMaxComps];
        double in[gfxColorMaxComps];
        double c, m, y, k, c1, m1, y1, k1, r, g, b;

        bradford_transform_to_d50(X, Y, Z, whiteX, whiteY, whiteZ);
        in[0] = X;
        in[1] = Y;
        in[2] = Z;
        transform->doTransform(in, out, 1);
        c = byteToDbl(out[0]);
        m = byteToDbl(out[1]);
        y = byteToDbl(out[2]);
        k = byteToDbl(out[3]);
        c1 = 1 - c;
        m1 = 1 - m;
        y1 = 1 - y;
        k1 = 1 - k;
        cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
        rgb->r = clip01(dblToCol(r));
        rgb->g = clip01(dblToCol(g));
        rgb->b = clip01(dblToCol(b));
        return;
    }
#endif
    bradford_transform_to_d65(X, Y, Z, whiteX, whiteY, whiteZ);
    // convert XYZ to RGB, including gamut mapping and gamma correction
    const double r = xyzrgb[0][0] * X + xyzrgb[0][1] * Y + xyzrgb[0][2] * Z;
    const double g = xyzrgb[1][0] * X + xyzrgb[1][1] * Y + xyzrgb[1][2] * Z;
    const double b = xyzrgb[2][0] * X + xyzrgb[2][1] * Y + xyzrgb[2][2] * Z;
    rgb->r = dblToCol(srgb_gamma_function(clip01(r)));
    rgb->g = dblToCol(srgb_gamma_function(clip01(g)));
    rgb->b = dblToCol(srgb_gamma_function(clip01(b)));
}

void GfxLabColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    GfxRGB rgb;
    GfxColorComp c, m, y, k;

#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_CMYK) {
        double in[gfxColorMaxComps];
        unsigned char out[gfxColorMaxComps];

        getXYZ(color, &in[0], &in[1], &in[2]);
        bradford_transform_to_d50(in[0], in[1], in[2], whiteX, whiteY, whiteZ);
        transform->doTransform(in, out, 1);
        cmyk->c = byteToCol(out[0]);
        cmyk->m = byteToCol(out[1]);
        cmyk->y = byteToCol(out[2]);
        cmyk->k = byteToCol(out[3]);
        return;
    }
#endif
    getRGB(color, &rgb);
    c = clip01(gfxColorComp1 - rgb.r);
    m = clip01(gfxColorComp1 - rgb.g);
    y = clip01(gfxColorComp1 - rgb.b);
    k = c;
    if (m < k) {
        k = m;
    }
    if (y < k) {
        k = y;
    }
    cmyk->c = c - k;
    cmyk->m = m - k;
    cmyk->y = y - k;
    cmyk->k = k;
}

void GfxLabColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    GfxCMYK cmyk;
    clearGfxColor(deviceN);
    getCMYK(color, &cmyk);
    deviceN->c[0] = cmyk.c;
    deviceN->c[1] = cmyk.m;
    deviceN->c[2] = cmyk.y;
    deviceN->c[3] = cmyk.k;
}

void GfxLabColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
    if (aMin > 0) {
        color->c[1] = dblToCol(aMin);
    } else if (aMax < 0) {
        color->c[1] = dblToCol(aMax);
    } else {
        color->c[1] = 0;
    }
    if (bMin > 0) {
        color->c[2] = dblToCol(bMin);
    } else if (bMax < 0) {
        color->c[2] = dblToCol(bMax);
    } else {
        color->c[2] = 0;
    }
}

void GfxLabColorSpace::getDefaultRanges(double *decodeLow, double *decodeRange, int maxImgPixel) const
{
    decodeLow[0] = 0;
    decodeRange[0] = 100;
    decodeLow[1] = aMin;
    decodeRange[1] = aMax - aMin;
    decodeLow[2] = bMin;
    decodeRange[2] = bMax - bMin;
}

//------------------------------------------------------------------------
// GfxICCBasedColorSpace
//------------------------------------------------------------------------

GfxICCBasedColorSpace::GfxICCBasedColorSpace(int nCompsA, GfxColorSpace *altA, const Ref *iccProfileStreamA)
{
    nComps = nCompsA;
    alt = altA;
    iccProfileStream = *iccProfileStreamA;
    rangeMin[0] = rangeMin[1] = rangeMin[2] = rangeMin[3] = 0;
    rangeMax[0] = rangeMax[1] = rangeMax[2] = rangeMax[3] = 1;
#ifdef USE_CMS
    transform = nullptr;
    lineTransform = nullptr;
    psCSA = nullptr;
#endif
}

GfxICCBasedColorSpace::~GfxICCBasedColorSpace()
{
    delete alt;
#ifdef USE_CMS
    if (psCSA) {
        gfree(psCSA);
    }
#endif
}

GfxColorSpace *GfxICCBasedColorSpace::copy() const
{
    GfxICCBasedColorSpace *cs;
    int i;

    cs = new GfxICCBasedColorSpace(nComps, alt->copy(), &iccProfileStream);
    for (i = 0; i < 4; ++i) {
        cs->rangeMin[i] = rangeMin[i];
        cs->rangeMax[i] = rangeMax[i];
    }
#ifdef USE_CMS
    cs->profile = profile;
    cs->transform = transform;
    cs->lineTransform = lineTransform;
#endif
    return cs;
}

GfxColorSpace *GfxICCBasedColorSpace::parse(Array *arr, OutputDev *out, GfxState *state, int recursion)
{
    GfxICCBasedColorSpace *cs;
    int nCompsA;
    GfxColorSpace *altA;
    Dict *dict;
    Object obj1, obj2;
    int i;

    if (arr->getLength() < 2) {
        error(errSyntaxError, -1, "Bad ICCBased color space");
        return nullptr;
    }
    const Object &obj1Ref = arr->getNF(1);
    const Ref iccProfileStreamA = obj1Ref.isRef() ? obj1Ref.getRef() : Ref::INVALID();
#ifdef USE_CMS
    // check cache
    if (out && iccProfileStreamA != Ref::INVALID()) {
        if (auto *item = out->getIccColorSpaceCache()->lookup(iccProfileStreamA)) {
            cs = static_cast<GfxICCBasedColorSpace *>(item->copy());
            int transformIntent = cs->getIntent();
            int cmsIntent = INTENT_RELATIVE_COLORIMETRIC;
            if (state != nullptr) {
                cmsIntent = state->getCmsRenderingIntent();
            }
            if (transformIntent == cmsIntent) {
                return cs;
            }
            delete cs;
        }
    }
#endif
    obj1 = arr->get(1);
    if (!obj1.isStream()) {
        error(errSyntaxWarning, -1, "Bad ICCBased color space (stream)");
        return nullptr;
    }
    dict = obj1.streamGetDict();
    obj2 = dict->lookup("N");
    if (!obj2.isInt()) {
        error(errSyntaxWarning, -1, "Bad ICCBased color space (N)");
        return nullptr;
    }
    nCompsA = obj2.getInt();
    if (nCompsA > 4) {
        error(errSyntaxError, -1, "ICCBased color space with too many ({0:d} > 4) components", nCompsA);
        nCompsA = 4;
    }
    obj2 = dict->lookup("Alternate");
    if (obj2.isNull() || !(altA = GfxColorSpace::parse(nullptr, &obj2, out, state, recursion + 1))) {
        switch (nCompsA) {
        case 1:
            altA = new GfxDeviceGrayColorSpace();
            break;
        case 3:
            altA = new GfxDeviceRGBColorSpace();
            break;
        case 4:
            altA = new GfxDeviceCMYKColorSpace();
            break;
        default:
            error(errSyntaxWarning, -1, "Bad ICCBased color space - invalid N");
            return nullptr;
        }
    }
    if (altA->getNComps() != nCompsA) {
        error(errSyntaxWarning, -1, "Bad ICCBased color space - N doesn't match alt color space");
        delete altA;
        return nullptr;
    }
    cs = new GfxICCBasedColorSpace(nCompsA, altA, &iccProfileStreamA);
    obj2 = dict->lookup("Range");
    if (obj2.isArray() && obj2.arrayGetLength() == 2 * nCompsA) {
        for (i = 0; i < nCompsA; ++i) {
            cs->rangeMin[i] = obj2.arrayGet(2 * i).getNumWithDefaultValue(0);
            cs->rangeMax[i] = obj2.arrayGet(2 * i + 1).getNumWithDefaultValue(1);
        }
    }

#ifdef USE_CMS
    obj1 = arr->get(1);
    if (!obj1.isStream()) {
        error(errSyntaxWarning, -1, "Bad ICCBased color space (stream)");
        delete cs;
        return nullptr;
    }
    Stream *iccStream = obj1.getStream();

    const std::vector<unsigned char> profBuf = iccStream->toUnsignedChars(65536, 65536);
    auto hp = make_GfxLCMSProfilePtr(cmsOpenProfileFromMem(profBuf.data(), profBuf.size()));
    cs->profile = hp;
    if (!hp) {
        error(errSyntaxWarning, -1, "read ICCBased color space profile error");
    } else {
        cs->buildTransforms(state);
    }
    // put this colorSpace into cache
    if (out && iccProfileStreamA != Ref::INVALID()) {
        out->getIccColorSpaceCache()->put(iccProfileStreamA, static_cast<GfxICCBasedColorSpace *>(cs->copy()));
    }
#endif
    return cs;
}

#ifdef USE_CMS
void GfxICCBasedColorSpace::buildTransforms(GfxState *state)
{
    auto dhp = (state != nullptr && state->getDisplayProfile() != nullptr) ? state->getDisplayProfile() : nullptr;
    if (!dhp) {
        dhp = GfxState::sRGBProfile;
    }
    unsigned int cst = getCMSColorSpaceType(cmsGetColorSpace(profile.get()));
    unsigned int dNChannels = getCMSNChannels(cmsGetColorSpace(dhp.get()));
    unsigned int dcst = getCMSColorSpaceType(cmsGetColorSpace(dhp.get()));
    cmsHTRANSFORM transformA;

    int cmsIntent = INTENT_RELATIVE_COLORIMETRIC;
    if (state != nullptr) {
        cmsIntent = state->getCmsRenderingIntent();
    }
    if ((transformA = cmsCreateTransform(profile.get(), COLORSPACE_SH(cst) | CHANNELS_SH(nComps) | BYTES_SH(1), dhp.get(), COLORSPACE_SH(dcst) | CHANNELS_SH(dNChannels) | BYTES_SH(1), cmsIntent, LCMS_FLAGS)) == nullptr) {
        error(errSyntaxWarning, -1, "Can't create transform");
        transform = nullptr;
    } else {
        transform = std::make_shared<GfxColorTransform>(transformA, cmsIntent, cst, dcst);
    }
    if (dcst == PT_RGB || dcst == PT_CMYK) {
        // create line transform only when the display is RGB type color space
        if ((transformA = cmsCreateTransform(profile.get(), CHANNELS_SH(nComps) | BYTES_SH(1), dhp.get(), (dcst == PT_RGB) ? TYPE_RGB_8 : TYPE_CMYK_8, cmsIntent, LCMS_FLAGS)) == nullptr) {
            error(errSyntaxWarning, -1, "Can't create transform");
            lineTransform = nullptr;
        } else {
            lineTransform = std::make_shared<GfxColorTransform>(transformA, cmsIntent, cst, dcst);
        }
    }
}
#endif

void GfxICCBasedColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_GRAY) {
        unsigned char in[gfxColorMaxComps];
        unsigned char out[gfxColorMaxComps];

        if (nComps == 3 && transform->getInputPixelType() == PT_Lab) {
            in[0] = colToByte(dblToCol(colToDbl(color->c[0]) / 100.0));
            in[1] = colToByte(dblToCol((colToDbl(color->c[1]) + 128.0) / 255.0));
            in[2] = colToByte(dblToCol((colToDbl(color->c[2]) + 128.0) / 255.0));
        } else {
            for (int i = 0; i < nComps; i++) {
                in[i] = colToByte(color->c[i]);
            }
        }
        if (nComps <= 4) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
            if (it != cmsCache.end()) {
                unsigned int value = it->second;
                *gray = byteToCol(value & 0xff);
                return;
            }
        }
        transform->doTransform(in, out, 1);
        *gray = byteToCol(out[0]);
        if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            unsigned int value = out[0];
            cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
        }
    } else {
        GfxRGB rgb;
        getRGB(color, &rgb);
        *gray = clip01((GfxColorComp)(0.3 * rgb.r + 0.59 * rgb.g + 0.11 * rgb.b + 0.5));
    }
#else
    alt->getGray(color, gray);
#endif
}

void GfxICCBasedColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_RGB) {
        unsigned char in[gfxColorMaxComps];
        unsigned char out[gfxColorMaxComps];

        if (nComps == 3 && transform->getInputPixelType() == PT_Lab) {
            in[0] = colToByte(dblToCol(colToDbl(color->c[0]) / 100.0));
            in[1] = colToByte(dblToCol((colToDbl(color->c[1]) + 128.0) / 255.0));
            in[2] = colToByte(dblToCol((colToDbl(color->c[2]) + 128.0) / 255.0));
        } else {
            for (int i = 0; i < nComps; i++) {
                in[i] = colToByte(color->c[i]);
            }
        }
        if (nComps <= 4) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
            if (it != cmsCache.end()) {
                unsigned int value = it->second;
                rgb->r = byteToCol(value >> 16);
                rgb->g = byteToCol((value >> 8) & 0xff);
                rgb->b = byteToCol(value & 0xff);
                return;
            }
        }
        transform->doTransform(in, out, 1);
        rgb->r = byteToCol(out[0]);
        rgb->g = byteToCol(out[1]);
        rgb->b = byteToCol(out[2]);
        if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            unsigned int value = (out[0] << 16) + (out[1] << 8) + out[2];
            cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
        }
    } else if (transform != nullptr && transform->getTransformPixelType() == PT_CMYK) {
        unsigned char in[gfxColorMaxComps];
        unsigned char out[gfxColorMaxComps];
        double c, m, y, k, c1, m1, y1, k1, r, g, b;

        if (nComps == 3 && transform->getInputPixelType() == PT_Lab) {
            in[0] = colToByte(dblToCol(colToDbl(color->c[0]) / 100.0));
            in[1] = colToByte(dblToCol((colToDbl(color->c[1]) + 128.0) / 255.0));
            in[2] = colToByte(dblToCol((colToDbl(color->c[2]) + 128.0) / 255.0));
        } else {
            for (int i = 0; i < nComps; i++) {
                in[i] = colToByte(color->c[i]);
            }
        }
        if (nComps <= 4) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
            if (it != cmsCache.end()) {
                unsigned int value = it->second;
                rgb->r = byteToCol(value >> 16);
                rgb->g = byteToCol((value >> 8) & 0xff);
                rgb->b = byteToCol(value & 0xff);
                return;
            }
        }
        transform->doTransform(in, out, 1);
        c = byteToDbl(out[0]);
        m = byteToDbl(out[1]);
        y = byteToDbl(out[2]);
        k = byteToDbl(out[3]);
        c1 = 1 - c;
        m1 = 1 - m;
        y1 = 1 - y;
        k1 = 1 - k;
        cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
        rgb->r = clip01(dblToCol(r));
        rgb->g = clip01(dblToCol(g));
        rgb->b = clip01(dblToCol(b));
        if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            unsigned int value = (dblToByte(r) << 16) + (dblToByte(g) << 8) + dblToByte(b);
            cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
        }
    } else {
        alt->getRGB(color, rgb);
    }
#else
    alt->getRGB(color, rgb);
#endif
}

void GfxICCBasedColorSpace::getRGBLine(unsigned char *in, unsigned int *out, int length)
{
#ifdef USE_CMS
    if (lineTransform != nullptr && lineTransform->getTransformPixelType() == PT_RGB) {
        unsigned char *tmp = (unsigned char *)gmallocn(3 * length, sizeof(unsigned char));
        lineTransform->doTransform(in, tmp, length);
        for (int i = 0; i < length; ++i) {
            unsigned char *current = tmp + (i * 3);
            out[i] = (current[0] << 16) | (current[1] << 8) | current[2];
        }
        gfree(tmp);
    } else {
        alt->getRGBLine(in, out, length);
    }
#else
    alt->getRGBLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getRGBLine(unsigned char *in, unsigned char *out, int length)
{
#ifdef USE_CMS
    if (lineTransform != nullptr && lineTransform->getTransformPixelType() == PT_RGB) {
        unsigned char *tmp = (unsigned char *)gmallocn(3 * length, sizeof(unsigned char));
        lineTransform->doTransform(in, tmp, length);
        unsigned char *current = tmp;
        for (int i = 0; i < length; ++i) {
            *out++ = *current++;
            *out++ = *current++;
            *out++ = *current++;
        }
        gfree(tmp);
    } else if (lineTransform != nullptr && lineTransform->getTransformPixelType() == PT_CMYK) {
        unsigned char *tmp = (unsigned char *)gmallocn(4 * length, sizeof(unsigned char));
        lineTransform->doTransform(in, tmp, length);
        unsigned char *current = tmp;
        double c, m, y, k, c1, m1, y1, k1, r, g, b;
        for (int i = 0; i < length; ++i) {
            c = byteToDbl(*current++);
            m = byteToDbl(*current++);
            y = byteToDbl(*current++);
            k = byteToDbl(*current++);
            c1 = 1 - c;
            m1 = 1 - m;
            y1 = 1 - y;
            k1 = 1 - k;
            cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
            *out++ = dblToByte(r);
            *out++ = dblToByte(g);
            *out++ = dblToByte(b);
        }
        gfree(tmp);
    } else {
        alt->getRGBLine(in, out, length);
    }
#else
    alt->getRGBLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getRGBXLine(unsigned char *in, unsigned char *out, int length)
{
#ifdef USE_CMS
    if (lineTransform != nullptr && lineTransform->getTransformPixelType() == PT_RGB) {
        unsigned char *tmp = (unsigned char *)gmallocn(3 * length, sizeof(unsigned char));
        lineTransform->doTransform(in, tmp, length);
        unsigned char *current = tmp;
        for (int i = 0; i < length; ++i) {
            *out++ = *current++;
            *out++ = *current++;
            *out++ = *current++;
            *out++ = 255;
        }
        gfree(tmp);
    } else {
        alt->getRGBXLine(in, out, length);
    }
#else
    alt->getRGBXLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getCMYKLine(unsigned char *in, unsigned char *out, int length)
{
#ifdef USE_CMS
    if (lineTransform != nullptr && lineTransform->getTransformPixelType() == PT_CMYK) {
        transform->doTransform(in, out, length);
    } else if (lineTransform != nullptr && nComps != 4) {
        GfxColorComp c, m, y, k;
        unsigned char *tmp = (unsigned char *)gmallocn(3 * length, sizeof(unsigned char));
        getRGBLine(in, tmp, length);
        unsigned char *p = tmp;
        for (int i = 0; i < length; i++) {
            c = byteToCol(255 - *p++);
            m = byteToCol(255 - *p++);
            y = byteToCol(255 - *p++);
            k = c;
            if (m < k) {
                k = m;
            }
            if (y < k) {
                k = y;
            }
            *out++ = colToByte(c - k);
            *out++ = colToByte(m - k);
            *out++ = colToByte(y - k);
            *out++ = colToByte(k);
        }
        gfree(tmp);
    } else {
        alt->getCMYKLine(in, out, length);
    }
#else
    alt->getCMYKLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getDeviceNLine(unsigned char *in, unsigned char *out, int length)
{
#ifdef USE_CMS
    if (lineTransform != nullptr && lineTransform->getTransformPixelType() == PT_CMYK) {
        unsigned char *tmp = (unsigned char *)gmallocn(4 * length, sizeof(unsigned char));
        transform->doTransform(in, tmp, length);
        unsigned char *p = tmp;
        for (int i = 0; i < length; i++) {
            for (int j = 0; j < 4; j++) {
                *out++ = *p++;
            }
            for (int j = 4; j < SPOT_NCOMPS + 4; j++) {
                *out++ = 0;
            }
        }
        gfree(tmp);
    } else if (lineTransform != nullptr && nComps != 4) {
        GfxColorComp c, m, y, k;
        unsigned char *tmp = (unsigned char *)gmallocn(3 * length, sizeof(unsigned char));
        getRGBLine(in, tmp, length);
        unsigned char *p = tmp;
        for (int i = 0; i < length; i++) {
            for (int j = 0; j < SPOT_NCOMPS + 4; j++) {
                out[j] = 0;
            }
            c = byteToCol(255 - *p++);
            m = byteToCol(255 - *p++);
            y = byteToCol(255 - *p++);
            k = c;
            if (m < k) {
                k = m;
            }
            if (y < k) {
                k = y;
            }
            out[0] = colToByte(c - k);
            out[1] = colToByte(m - k);
            out[2] = colToByte(y - k);
            out[3] = colToByte(k);
            out += (SPOT_NCOMPS + 4);
        }
        gfree(tmp);
    } else {
        alt->getDeviceNLine(in, out, length);
    }
#else
    alt->getDeviceNLine(in, out, length);
#endif
}

void GfxICCBasedColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
#ifdef USE_CMS
    if (transform != nullptr && transform->getTransformPixelType() == PT_CMYK) {
        unsigned char in[gfxColorMaxComps];
        unsigned char out[gfxColorMaxComps];

        if (nComps == 3 && transform->getInputPixelType() == PT_Lab) {
            in[0] = colToByte(dblToCol(colToDbl(color->c[0]) / 100.0));
            in[1] = colToByte(dblToCol((colToDbl(color->c[1]) + 128.0) / 255.0));
            in[2] = colToByte(dblToCol((colToDbl(color->c[2]) + 128.0) / 255.0));
        } else {
            for (int i = 0; i < nComps; i++) {
                in[i] = colToByte(color->c[i]);
            }
        }
        if (nComps <= 4) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            std::map<unsigned int, unsigned int>::iterator it = cmsCache.find(key);
            if (it != cmsCache.end()) {
                unsigned int value = it->second;
                cmyk->c = byteToCol(value >> 24);
                cmyk->m = byteToCol((value >> 16) & 0xff);
                cmyk->y = byteToCol((value >> 8) & 0xff);
                cmyk->k = byteToCol(value & 0xff);
                return;
            }
        }
        transform->doTransform(in, out, 1);
        cmyk->c = byteToCol(out[0]);
        cmyk->m = byteToCol(out[1]);
        cmyk->y = byteToCol(out[2]);
        cmyk->k = byteToCol(out[3]);
        if (nComps <= 4 && cmsCache.size() <= CMSCACHE_LIMIT) {
            unsigned int key = 0;
            for (int j = 0; j < nComps; j++) {
                key = (key << 8) + in[j];
            }
            unsigned int value = (out[0] << 24) + (out[1] << 16) + (out[2] << 8) + out[3];
            cmsCache.insert(std::pair<unsigned int, unsigned int>(key, value));
        }
    } else if (nComps != 4 && transform != nullptr && transform->getTransformPixelType() == PT_RGB) {
        GfxRGB rgb;
        GfxColorComp c, m, y, k;

        getRGB(color, &rgb);
        c = clip01(gfxColorComp1 - rgb.r);
        m = clip01(gfxColorComp1 - rgb.g);
        y = clip01(gfxColorComp1 - rgb.b);
        k = c;
        if (m < k) {
            k = m;
        }
        if (y < k) {
            k = y;
        }
        cmyk->c = c - k;
        cmyk->m = m - k;
        cmyk->y = y - k;
        cmyk->k = k;
    } else {
        alt->getCMYK(color, cmyk);
    }
#else
    alt->getCMYK(color, cmyk);
#endif
}

bool GfxICCBasedColorSpace::useGetRGBLine() const
{
#ifdef USE_CMS
    return lineTransform != nullptr || alt->useGetRGBLine();
#else
    return alt->useGetRGBLine();
#endif
}

bool GfxICCBasedColorSpace::useGetCMYKLine() const
{
#ifdef USE_CMS
    return lineTransform != nullptr || alt->useGetCMYKLine();
#else
    return alt->useGetCMYKLine();
#endif
}

bool GfxICCBasedColorSpace::useGetDeviceNLine() const
{
#ifdef USE_CMS
    return lineTransform != nullptr || alt->useGetDeviceNLine();
#else
    return alt->useGetDeviceNLine();
#endif
}

void GfxICCBasedColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    GfxCMYK cmyk;
    clearGfxColor(deviceN);
    getCMYK(color, &cmyk);
    deviceN->c[0] = cmyk.c;
    deviceN->c[1] = cmyk.m;
    deviceN->c[2] = cmyk.y;
    deviceN->c[3] = cmyk.k;
}

void GfxICCBasedColorSpace::getDefaultColor(GfxColor *color) const
{
    int i;

    for (i = 0; i < nComps; ++i) {
        if (rangeMin[i] > 0) {
            color->c[i] = dblToCol(rangeMin[i]);
        } else if (rangeMax[i] < 0) {
            color->c[i] = dblToCol(rangeMax[i]);
        } else {
            color->c[i] = 0;
        }
    }
}

void GfxICCBasedColorSpace::getDefaultRanges(double *decodeLow, double *decodeRange, int maxImgPixel) const
{
    alt->getDefaultRanges(decodeLow, decodeRange, maxImgPixel);

#if 0
  // this is nominally correct, but some PDF files don't set the
  // correct ranges in the ICCBased dict
  int i;

  for (i = 0; i < nComps; ++i) {
    decodeLow[i] = rangeMin[i];
    decodeRange[i] = rangeMax[i] - rangeMin[i];
  }
#endif
}

#ifdef USE_CMS
char *GfxICCBasedColorSpace::getPostScriptCSA()
{
#    if LCMS_VERSION >= 2070
    // The runtime version check of lcms2 is only available from release 2.7 upwards.
    // The generation of the CSA code only works reliably for version 2.10 and upwards.
    // Cf. the explanation in the corresponding lcms2 merge request [1], and the original mail thread [2].
    // [1] https://github.com/mm2/Little-CMS/pull/214
    // [2] https://sourceforge.net/p/lcms/mailman/message/33182987/
    if (cmsGetEncodedCMMversion() < 2100) {
        return nullptr;
    }

    int size;

    if (psCSA) {
        return psCSA;
    }

    if (!profile) {
        error(errSyntaxWarning, -1, "profile is nullptr");
        return nullptr;
    }

    void *rawprofile = profile.get();
    size = cmsGetPostScriptCSA(cmsGetProfileContextID(rawprofile), rawprofile, getIntent(), 0, nullptr, 0);
    if (size == 0) {
        error(errSyntaxWarning, -1, "PostScript CSA is nullptr");
        return nullptr;
    }

    psCSA = (char *)gmalloc(size + 1);
    cmsGetPostScriptCSA(cmsGetProfileContextID(rawprofile), rawprofile, getIntent(), 0, psCSA, size);
    psCSA[size] = 0;

    // TODO REMOVE-ME-IN-THE-FUTURE
    // until we can depend on https://github.com/mm2/Little-CMS/issues/223 being fixed
    // lcms returns ps code with , instead of . for some locales. The lcms author says
    // that there's no room for any , in the rest of the ps code, so replacing all the , with .
    // is an "acceptable" workaround
    for (int i = 0; i < size; ++i) {
        if (psCSA[i] == ',') {
            psCSA[i] = '.';
        }
    }

    return psCSA;
#    else
    return nullptr;
#    endif
}
#endif

//------------------------------------------------------------------------
// GfxIndexedColorSpace
//------------------------------------------------------------------------

GfxIndexedColorSpace::GfxIndexedColorSpace(GfxColorSpace *baseA, int indexHighA)
{
    base = baseA;
    indexHigh = indexHighA;
    lookup = (unsigned char *)gmallocn((indexHigh + 1) * base->getNComps(), sizeof(unsigned char));
    overprintMask = base->getOverprintMask();
}

GfxIndexedColorSpace::~GfxIndexedColorSpace()
{
    delete base;
    gfree(lookup);
}

GfxColorSpace *GfxIndexedColorSpace::copy() const
{
    GfxIndexedColorSpace *cs;

    cs = new GfxIndexedColorSpace(base->copy(), indexHigh);
    memcpy(cs->lookup, lookup, (indexHigh + 1) * base->getNComps() * sizeof(unsigned char));
    return cs;
}

GfxColorSpace *GfxIndexedColorSpace::parse(GfxResources *res, Array *arr, OutputDev *out, GfxState *state, int recursion)
{
    GfxColorSpace *baseA;
    int indexHighA;
    Object obj1;
    const char *s;
    int i, j;

    if (arr->getLength() != 4) {
        error(errSyntaxWarning, -1, "Bad Indexed color space");
        return nullptr;
    }
    obj1 = arr->get(1);
    if (!(baseA = GfxColorSpace::parse(res, &obj1, out, state, recursion + 1))) {
        error(errSyntaxWarning, -1, "Bad Indexed color space (base color space)");
        return nullptr;
    }
    obj1 = arr->get(2);
    if (!obj1.isInt()) {
        error(errSyntaxWarning, -1, "Bad Indexed color space (hival)");
        delete baseA;
        return nullptr;
    }
    indexHighA = obj1.getInt();
    if (indexHighA < 0 || indexHighA > 255) {
        // the PDF spec requires indexHigh to be in [0,255] -- allowing
        // values larger than 255 creates a security hole: if nComps *
        // indexHigh is greater than 2^31, the loop below may overwrite
        // past the end of the array
        int previousValue = indexHighA;
        if (indexHighA < 0) {
            indexHighA = 0;
        } else {
            indexHighA = 255;
        }
        error(errSyntaxWarning, -1, "Bad Indexed color space (invalid indexHigh value, was {0:d} using {1:d} to try to recover)", previousValue, indexHighA);
    }
    GfxIndexedColorSpace *cs = new GfxIndexedColorSpace(baseA, indexHighA);
    obj1 = arr->get(3);
    const int n = baseA->getNComps();
    if (obj1.isStream()) {
        obj1.streamReset();
        for (i = 0; i <= indexHighA; ++i) {
            const int readChars = obj1.streamGetChars(n, &cs->lookup[i * n]);
            for (j = readChars; j < n; ++j) {
                error(errSyntaxWarning, -1, "Bad Indexed color space (lookup table stream too short) padding with zeroes");
                cs->lookup[i * n + j] = 0;
            }
        }
        obj1.streamClose();
    } else if (obj1.isString()) {
        if (obj1.getString()->getLength() < (indexHighA + 1) * n) {
            error(errSyntaxWarning, -1, "Bad Indexed color space (lookup table string too short)");
            goto err3;
        }
        s = obj1.getString()->c_str();
        for (i = 0; i <= indexHighA; ++i) {
            for (j = 0; j < n; ++j) {
                cs->lookup[i * n + j] = (unsigned char)*s++;
            }
        }
    } else {
        error(errSyntaxWarning, -1, "Bad Indexed color space (lookup table)");
        goto err3;
    }
    return cs;

err3:
    delete cs;
    return nullptr;
}

GfxColor *GfxIndexedColorSpace::mapColorToBase(const GfxColor *color, GfxColor *baseColor) const
{
    unsigned char *p;
    double low[gfxColorMaxComps], range[gfxColorMaxComps];
    int n, i;

    n = base->getNComps();
    base->getDefaultRanges(low, range, indexHigh);
    const int idx = (int)(colToDbl(color->c[0]) + 0.5) * n;
    if (likely((idx + n - 1 < (indexHigh + 1) * base->getNComps()) && idx >= 0)) {
        p = &lookup[idx];
        for (i = 0; i < n; ++i) {
            baseColor->c[i] = dblToCol(low[i] + (p[i] / 255.0) * range[i]);
        }
    } else {
        for (i = 0; i < n; ++i) {
            baseColor->c[i] = 0;
        }
    }
    return baseColor;
}

void GfxIndexedColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    GfxColor color2;

    base->getGray(mapColorToBase(color, &color2), gray);
}

void GfxIndexedColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    GfxColor color2;

    base->getRGB(mapColorToBase(color, &color2), rgb);
}

void GfxIndexedColorSpace::getRGBLine(unsigned char *in, unsigned int *out, int length)
{
    unsigned char *line;
    int i, j, n;

    n = base->getNComps();
    line = (unsigned char *)gmallocn(length, n);
    for (i = 0; i < length; i++) {
        for (j = 0; j < n; j++) {
            line[i * n + j] = lookup[in[i] * n + j];
        }
    }

    base->getRGBLine(line, out, length);

    gfree(line);
}

void GfxIndexedColorSpace::getRGBLine(unsigned char *in, unsigned char *out, int length)
{
    unsigned char *line;
    int i, j, n;

    n = base->getNComps();
    line = (unsigned char *)gmallocn(length, n);
    for (i = 0; i < length; i++) {
        for (j = 0; j < n; j++) {
            line[i * n + j] = lookup[in[i] * n + j];
        }
    }

    base->getRGBLine(line, out, length);

    gfree(line);
}

void GfxIndexedColorSpace::getRGBXLine(unsigned char *in, unsigned char *out, int length)
{
    unsigned char *line;
    int i, j, n;

    n = base->getNComps();
    line = (unsigned char *)gmallocn(length, n);
    for (i = 0; i < length; i++) {
        for (j = 0; j < n; j++) {
            line[i * n + j] = lookup[in[i] * n + j];
        }
    }

    base->getRGBXLine(line, out, length);

    gfree(line);
}

void GfxIndexedColorSpace::getCMYKLine(unsigned char *in, unsigned char *out, int length)
{
    unsigned char *line;
    int i, j, n;

    n = base->getNComps();
    line = (unsigned char *)gmallocn(length, n);
    for (i = 0; i < length; i++) {
        for (j = 0; j < n; j++) {
            line[i * n + j] = lookup[in[i] * n + j];
        }
    }

    base->getCMYKLine(line, out, length);

    gfree(line);
}

void GfxIndexedColorSpace::getDeviceNLine(unsigned char *in, unsigned char *out, int length)
{
    unsigned char *line;
    int i, j, n;

    n = base->getNComps();
    line = (unsigned char *)gmallocn(length, n);
    for (i = 0; i < length; i++) {
        for (j = 0; j < n; j++) {
            line[i * n + j] = lookup[in[i] * n + j];
        }
    }

    base->getDeviceNLine(line, out, length);

    gfree(line);
}

void GfxIndexedColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    GfxColor color2;

    base->getCMYK(mapColorToBase(color, &color2), cmyk);
}

void GfxIndexedColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    GfxColor color2;

    base->getDeviceN(mapColorToBase(color, &color2), deviceN);
}

void GfxIndexedColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
}

void GfxIndexedColorSpace::getDefaultRanges(double *decodeLow, double *decodeRange, int maxImgPixel) const
{
    decodeLow[0] = 0;
    decodeRange[0] = maxImgPixel;
}

//------------------------------------------------------------------------
// GfxSeparationColorSpace
//------------------------------------------------------------------------

GfxSeparationColorSpace::GfxSeparationColorSpace(GooString *nameA, GfxColorSpace *altA, Function *funcA)
{
    name = nameA;
    alt = altA;
    func = funcA;
    nonMarking = !name->cmp("None");
    if (!name->cmp("Cyan")) {
        overprintMask = 0x01;
    } else if (!name->cmp("Magenta")) {
        overprintMask = 0x02;
    } else if (!name->cmp("Yellow")) {
        overprintMask = 0x04;
    } else if (!name->cmp("Black")) {
        overprintMask = 0x08;
    } else if (!name->cmp("All")) {
        overprintMask = 0xffffffff;
    }
}

GfxSeparationColorSpace::GfxSeparationColorSpace(GooString *nameA, GfxColorSpace *altA, Function *funcA, bool nonMarkingA, unsigned int overprintMaskA, int *mappingA)
{
    name = nameA;
    alt = altA;
    func = funcA;
    nonMarking = nonMarkingA;
    overprintMask = overprintMaskA;
    mapping = mappingA;
}

GfxSeparationColorSpace::~GfxSeparationColorSpace()
{
    delete name;
    delete alt;
    delete func;
    if (mapping != nullptr) {
        gfree(mapping);
    }
}

GfxColorSpace *GfxSeparationColorSpace::copy() const
{
    int *mappingA = nullptr;
    if (mapping != nullptr) {
        mappingA = (int *)gmalloc(sizeof(int));
        *mappingA = *mapping;
    }
    return new GfxSeparationColorSpace(name->copy(), alt->copy(), func->copy(), nonMarking, overprintMask, mappingA);
}

//~ handle the 'All' and 'None' colorants
GfxColorSpace *GfxSeparationColorSpace::parse(GfxResources *res, Array *arr, OutputDev *out, GfxState *state, int recursion)
{
    GooString *nameA;
    GfxColorSpace *altA;
    Function *funcA;
    Object obj1;

    if (arr->getLength() != 4) {
        error(errSyntaxWarning, -1, "Bad Separation color space");
        goto err1;
    }
    obj1 = arr->get(1);
    if (!obj1.isName()) {
        error(errSyntaxWarning, -1, "Bad Separation color space (name)");
        goto err1;
    }
    nameA = new GooString(obj1.getName());
    obj1 = arr->get(2);
    if (!(altA = GfxColorSpace::parse(res, &obj1, out, state, recursion + 1))) {
        error(errSyntaxWarning, -1, "Bad Separation color space (alternate color space)");
        goto err3;
    }
    obj1 = arr->get(3);
    if (!(funcA = Function::parse(&obj1))) {
        goto err4;
    }
    if (funcA->getInputSize() != 1) {
        error(errSyntaxWarning, -1, "Bad SeparationColorSpace function");
        goto err5;
    }
    if (altA->getNComps() <= funcA->getOutputSize()) {
        return new GfxSeparationColorSpace(nameA, altA, funcA);
    }

err5:
    delete funcA;
err4:
    delete altA;
err3:
    delete nameA;
err1:
    return nullptr;
}

void GfxSeparationColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    double x;
    double c[gfxColorMaxComps];
    GfxColor color2;
    int i;

    if (alt->getMode() == csDeviceGray && name->cmp("Black") == 0) {
        *gray = clip01(gfxColorComp1 - color->c[0]);
    } else {
        x = colToDbl(color->c[0]);
        func->transform(&x, c);
        for (i = 0; i < alt->getNComps(); ++i) {
            color2.c[i] = dblToCol(c[i]);
        }
        alt->getGray(&color2, gray);
    }
}

void GfxSeparationColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    double x;
    double c[gfxColorMaxComps];
    GfxColor color2;
    int i;

    if (alt->getMode() == csDeviceGray && name->cmp("Black") == 0) {
        rgb->r = clip01(gfxColorComp1 - color->c[0]);
        rgb->g = clip01(gfxColorComp1 - color->c[0]);
        rgb->b = clip01(gfxColorComp1 - color->c[0]);
    } else {
        x = colToDbl(color->c[0]);
        func->transform(&x, c);
        const int altNComps = alt->getNComps();
        for (i = 0; i < altNComps; ++i) {
            color2.c[i] = dblToCol(c[i]);
        }
        alt->getRGB(&color2, rgb);
    }
}

void GfxSeparationColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    double x;
    double c[gfxColorMaxComps];
    GfxColor color2;
    int i;

    if (name->cmp("Black") == 0) {
        cmyk->c = 0;
        cmyk->m = 0;
        cmyk->y = 0;
        cmyk->k = color->c[0];
    } else if (name->cmp("Cyan") == 0) {
        cmyk->c = color->c[0];
        cmyk->m = 0;
        cmyk->y = 0;
        cmyk->k = 0;
    } else if (name->cmp("Magenta") == 0) {
        cmyk->c = 0;
        cmyk->m = color->c[0];
        cmyk->y = 0;
        cmyk->k = 0;
    } else if (name->cmp("Yellow") == 0) {
        cmyk->c = 0;
        cmyk->m = 0;
        cmyk->y = color->c[0];
        cmyk->k = 0;
    } else {
        x = colToDbl(color->c[0]);
        func->transform(&x, c);
        for (i = 0; i < alt->getNComps(); ++i) {
            color2.c[i] = dblToCol(c[i]);
        }
        alt->getCMYK(&color2, cmyk);
    }
}

void GfxSeparationColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    clearGfxColor(deviceN);
    if (mapping == nullptr || mapping[0] == -1) {
        GfxCMYK cmyk;

        getCMYK(color, &cmyk);
        deviceN->c[0] = cmyk.c;
        deviceN->c[1] = cmyk.m;
        deviceN->c[2] = cmyk.y;
        deviceN->c[3] = cmyk.k;
    } else {
        deviceN->c[mapping[0]] = color->c[0];
    }
}

void GfxSeparationColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = gfxColorComp1;
}

void GfxSeparationColorSpace::createMapping(std::vector<GfxSeparationColorSpace *> *separationList, int maxSepComps)
{
    if (nonMarking) {
        return;
    }
    mapping = (int *)gmalloc(sizeof(int));
    switch (overprintMask) {
    case 0x01:
        *mapping = 0;
        break;
    case 0x02:
        *mapping = 1;
        break;
    case 0x04:
        *mapping = 2;
        break;
    case 0x08:
        *mapping = 3;
        break;
    default:
        unsigned int newOverprintMask = 0x10;
        for (std::size_t i = 0; i < separationList->size(); i++) {
            GfxSeparationColorSpace *sepCS = (*separationList)[i];
            if (!sepCS->getName()->cmp(name)) {
                if (sepCS->getFunc()->hasDifferentResultSet(func)) {
                    error(errSyntaxWarning, -1, "Different functions found for '{0:t}', convert immediately", name);
                    gfree(mapping);
                    mapping = nullptr;
                    return;
                }
                *mapping = i + 4;
                overprintMask = newOverprintMask;
                return;
            }
            newOverprintMask <<= 1;
        }
        if ((int)separationList->size() == maxSepComps) {
            error(errSyntaxWarning, -1, "Too many ({0:d}) spots, convert '{1:t}' immediately", maxSepComps, name);
            gfree(mapping);
            mapping = nullptr;
            return;
        }
        *mapping = separationList->size() + 4;
        separationList->push_back((GfxSeparationColorSpace *)copy());
        overprintMask = newOverprintMask;
        break;
    }
}

//------------------------------------------------------------------------
// GfxDeviceNColorSpace
//------------------------------------------------------------------------

GfxDeviceNColorSpace::GfxDeviceNColorSpace(int nCompsA, std::vector<std::string> &&namesA, GfxColorSpace *altA, Function *funcA, std::vector<GfxSeparationColorSpace *> *sepsCSA) : nComps(nCompsA), names(std::move(namesA))
{
    alt = altA;
    func = funcA;
    sepsCS = sepsCSA;
    nonMarking = true;
    overprintMask = 0;
    mapping = nullptr;
    for (int i = 0; i < nComps; ++i) {
        if (names[i] != "None") {
            nonMarking = false;
        }
        if (names[i] == "Cyan") {
            overprintMask |= 0x01;
        } else if (names[i] == "Magenta") {
            overprintMask |= 0x02;
        } else if (names[i] == "Yellow") {
            overprintMask |= 0x04;
        } else if (names[i] == "Black") {
            overprintMask |= 0x08;
        } else if (names[i] == "All") {
            overprintMask = 0xffffffff;
        } else if (names[i] != "None") {
            overprintMask = 0x0f;
        }
    }
}

GfxDeviceNColorSpace::GfxDeviceNColorSpace(int nCompsA, const std::vector<std::string> &namesA, GfxColorSpace *altA, Function *funcA, std::vector<GfxSeparationColorSpace *> *sepsCSA, int *mappingA, bool nonMarkingA,
                                           unsigned int overprintMaskA)
    : nComps(nCompsA), names(namesA)
{
    alt = altA;
    func = funcA;
    sepsCS = sepsCSA;
    mapping = mappingA;
    nonMarking = nonMarkingA;
    overprintMask = overprintMaskA;
}

GfxDeviceNColorSpace::~GfxDeviceNColorSpace()
{
    delete alt;
    delete func;
    for (auto entry : *sepsCS) {
        delete entry;
    }
    delete sepsCS;
    if (mapping != nullptr) {
        gfree(mapping);
    }
}

GfxColorSpace *GfxDeviceNColorSpace::copy() const
{
    int *mappingA = nullptr;

    auto sepsCSA = new std::vector<GfxSeparationColorSpace *>();
    sepsCSA->reserve(sepsCS->size());
    for (const GfxSeparationColorSpace *scs : *sepsCS) {
        if (likely(scs != nullptr)) {
            sepsCSA->push_back((GfxSeparationColorSpace *)scs->copy());
        }
    }
    if (mapping != nullptr) {
        mappingA = (int *)gmalloc(sizeof(int) * nComps);
        for (int i = 0; i < nComps; i++) {
            mappingA[i] = mapping[i];
        }
    }
    return new GfxDeviceNColorSpace(nComps, names, alt->copy(), func->copy(), sepsCSA, mappingA, nonMarking, overprintMask);
}

//~ handle the 'None' colorant
GfxColorSpace *GfxDeviceNColorSpace::parse(GfxResources *res, Array *arr, OutputDev *out, GfxState *state, int recursion)
{
    int nCompsA;
    std::vector<std::string> namesA;
    GfxColorSpace *altA;
    Function *funcA;
    Object obj1;
    auto separationList = new std::vector<GfxSeparationColorSpace *>();

    if (arr->getLength() != 4 && arr->getLength() != 5) {
        error(errSyntaxWarning, -1, "Bad DeviceN color space");
        goto err1;
    }
    obj1 = arr->get(1);
    if (!obj1.isArray()) {
        error(errSyntaxWarning, -1, "Bad DeviceN color space (names)");
        goto err1;
    }
    nCompsA = obj1.arrayGetLength();
    if (nCompsA > gfxColorMaxComps) {
        error(errSyntaxWarning, -1, "DeviceN color space with too many ({0:d} > {1:d}) components", nCompsA, gfxColorMaxComps);
        nCompsA = gfxColorMaxComps;
    }
    for (int i = 0; i < nCompsA; ++i) {
        Object obj2 = obj1.arrayGet(i);
        if (!obj2.isName()) {
            error(errSyntaxWarning, -1, "Bad DeviceN color space (names)");
            nCompsA = i;
            goto err1;
        }
        namesA.emplace_back(obj2.getName());
    }
    obj1 = arr->get(2);
    if (!(altA = GfxColorSpace::parse(res, &obj1, out, state, recursion + 1))) {
        error(errSyntaxWarning, -1, "Bad DeviceN color space (alternate color space)");
        goto err1;
    }
    obj1 = arr->get(3);
    if (!(funcA = Function::parse(&obj1))) {
        goto err4;
    }
    if (arr->getLength() == 5) {
        obj1 = arr->get(4);
        if (!obj1.isDict()) {
            error(errSyntaxWarning, -1, "Bad DeviceN color space (attributes)");
            goto err5;
        }
        Dict *attribs = obj1.getDict();
        Object obj2 = attribs->lookup("Colorants");
        if (obj2.isDict()) {
            Dict *colorants = obj2.getDict();
            for (int i = 0; i < colorants->getLength(); i++) {
                Object obj3 = colorants->getVal(i);
                if (obj3.isArray()) {
                    GfxSeparationColorSpace *cs = (GfxSeparationColorSpace *)GfxSeparationColorSpace::parse(res, obj3.getArray(), out, state, recursion);
                    if (cs) {
                        separationList->push_back(cs);
                    }
                } else {
                    error(errSyntaxWarning, -1, "Bad DeviceN color space (colorant value entry is not an Array)");
                    goto err5;
                }
            }
        }
    }

    if (likely(nCompsA >= funcA->getInputSize() && altA->getNComps() <= funcA->getOutputSize())) {
        return new GfxDeviceNColorSpace(nCompsA, std::move(namesA), altA, funcA, separationList);
    }

err5:
    delete funcA;
err4:
    delete altA;
err1:
    delete separationList;
    return nullptr;
}

void GfxDeviceNColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    double x[gfxColorMaxComps], c[gfxColorMaxComps];
    GfxColor color2;
    int i;

    for (i = 0; i < nComps; ++i) {
        x[i] = colToDbl(color->c[i]);
    }
    func->transform(x, c);
    for (i = 0; i < alt->getNComps(); ++i) {
        color2.c[i] = dblToCol(c[i]);
    }
    alt->getGray(&color2, gray);
}

void GfxDeviceNColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    double x[gfxColorMaxComps], c[gfxColorMaxComps];
    GfxColor color2;
    int i;

    for (i = 0; i < nComps; ++i) {
        x[i] = colToDbl(color->c[i]);
    }
    func->transform(x, c);
    for (i = 0; i < alt->getNComps(); ++i) {
        color2.c[i] = dblToCol(c[i]);
    }
    alt->getRGB(&color2, rgb);
}

void GfxDeviceNColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    double x[gfxColorMaxComps], c[gfxColorMaxComps];
    GfxColor color2;
    int i;

    for (i = 0; i < nComps; ++i) {
        x[i] = colToDbl(color->c[i]);
    }
    func->transform(x, c);
    for (i = 0; i < alt->getNComps(); ++i) {
        color2.c[i] = dblToCol(c[i]);
    }
    alt->getCMYK(&color2, cmyk);
}

void GfxDeviceNColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    clearGfxColor(deviceN);
    if (mapping == nullptr) {
        GfxCMYK cmyk;

        getCMYK(color, &cmyk);
        deviceN->c[0] = cmyk.c;
        deviceN->c[1] = cmyk.m;
        deviceN->c[2] = cmyk.y;
        deviceN->c[3] = cmyk.k;
    } else {
        for (int j = 0; j < nComps; j++) {
            if (mapping[j] != -1) {
                deviceN->c[mapping[j]] = color->c[j];
            }
        }
    }
}

void GfxDeviceNColorSpace::getDefaultColor(GfxColor *color) const
{
    int i;

    for (i = 0; i < nComps; ++i) {
        color->c[i] = gfxColorComp1;
    }
}

void GfxDeviceNColorSpace::createMapping(std::vector<GfxSeparationColorSpace *> *separationList, int maxSepComps)
{
    if (nonMarking) { // None
        return;
    }
    mapping = (int *)gmalloc(sizeof(int) * nComps);
    unsigned int newOverprintMask = 0;
    for (int i = 0; i < nComps; i++) {
        if (names[i] == "None") {
            mapping[i] = -1;
        } else if (names[i] == "Cyan") {
            newOverprintMask |= 0x01;
            mapping[i] = 0;
        } else if (names[i] == "Magenta") {
            newOverprintMask |= 0x02;
            mapping[i] = 1;
        } else if (names[i] == "Yellow") {
            newOverprintMask |= 0x04;
            mapping[i] = 2;
        } else if (names[i] == "Black") {
            newOverprintMask |= 0x08;
            mapping[i] = 3;
        } else {
            unsigned int startOverprintMask = 0x10;
            bool found = false;
            const Function *sepFunc = nullptr;
            if (nComps == 1) {
                sepFunc = func;
            } else {
                for (const GfxSeparationColorSpace *sepCS : *sepsCS) {
                    if (!sepCS->getName()->cmp(names[i])) {
                        sepFunc = sepCS->getFunc();
                        break;
                    }
                }
            }
            for (std::size_t j = 0; j < separationList->size(); j++) {
                GfxSeparationColorSpace *sepCS = (*separationList)[j];
                if (!sepCS->getName()->cmp(names[i])) {
                    if (sepFunc != nullptr && sepCS->getFunc()->hasDifferentResultSet(sepFunc)) {
                        error(errSyntaxWarning, -1, "Different functions found for '{0:s}', convert immediately", names[i].c_str());
                        gfree(mapping);
                        mapping = nullptr;
                        overprintMask = 0xffffffff;
                        return;
                    }
                    mapping[i] = j + 4;
                    newOverprintMask |= startOverprintMask;
                    found = true;
                    break;
                }
                startOverprintMask <<= 1;
            }
            if (!found) {
                if ((int)separationList->size() == maxSepComps) {
                    error(errSyntaxWarning, -1, "Too many ({0:d}) spots, convert '{1:s}' immediately", maxSepComps, names[i].c_str());
                    gfree(mapping);
                    mapping = nullptr;
                    overprintMask = 0xffffffff;
                    return;
                }
                mapping[i] = separationList->size() + 4;
                newOverprintMask |= startOverprintMask;
                if (nComps == 1) {
                    separationList->push_back(new GfxSeparationColorSpace(new GooString(names[i]), alt->copy(), func->copy()));
                } else {
                    for (const GfxSeparationColorSpace *sepCS : *sepsCS) {
                        if (!sepCS->getName()->cmp(names[i])) {
                            found = true;
                            separationList->push_back((GfxSeparationColorSpace *)sepCS->copy());
                            break;
                        }
                    }
                    if (!found) {
                        error(errSyntaxWarning, -1, "DeviceN has no suitable colorant");
                        gfree(mapping);
                        mapping = nullptr;
                        overprintMask = 0xffffffff;
                        return;
                    }
                }
            }
        }
    }
    overprintMask = newOverprintMask;
}

//------------------------------------------------------------------------
// GfxPatternColorSpace
//------------------------------------------------------------------------

GfxPatternColorSpace::GfxPatternColorSpace(GfxColorSpace *underA)
{
    under = underA;
}

GfxPatternColorSpace::~GfxPatternColorSpace()
{
    if (under) {
        delete under;
    }
}

GfxColorSpace *GfxPatternColorSpace::copy() const
{
    return new GfxPatternColorSpace(under ? under->copy() : nullptr);
}

GfxColorSpace *GfxPatternColorSpace::parse(GfxResources *res, Array *arr, OutputDev *out, GfxState *state, int recursion)
{
    GfxPatternColorSpace *cs;
    GfxColorSpace *underA;
    Object obj1;

    if (arr->getLength() != 1 && arr->getLength() != 2) {
        error(errSyntaxWarning, -1, "Bad Pattern color space");
        return nullptr;
    }
    underA = nullptr;
    if (arr->getLength() == 2) {
        obj1 = arr->get(1);
        if (!(underA = GfxColorSpace::parse(res, &obj1, out, state, recursion + 1))) {
            error(errSyntaxWarning, -1, "Bad Pattern color space (underlying color space)");
            return nullptr;
        }
    }
    cs = new GfxPatternColorSpace(underA);
    return cs;
}

void GfxPatternColorSpace::getGray(const GfxColor *color, GfxGray *gray) const
{
    *gray = 0;
}

void GfxPatternColorSpace::getRGB(const GfxColor *color, GfxRGB *rgb) const
{
    rgb->r = rgb->g = rgb->b = 0;
}

void GfxPatternColorSpace::getCMYK(const GfxColor *color, GfxCMYK *cmyk) const
{
    cmyk->c = cmyk->m = cmyk->y = 0;
    cmyk->k = 1;
}

void GfxPatternColorSpace::getDeviceN(const GfxColor *color, GfxColor *deviceN) const
{
    clearGfxColor(deviceN);
    deviceN->c[3] = 1;
}

void GfxPatternColorSpace::getDefaultColor(GfxColor *color) const
{
    color->c[0] = 0;
}

//------------------------------------------------------------------------
// Pattern
//------------------------------------------------------------------------

GfxPattern::GfxPattern(int typeA, int patternRefNumA) : type(typeA), patternRefNum(patternRefNumA) { }

GfxPattern::~GfxPattern() { }

GfxPattern *GfxPattern::parse(GfxResources *res, Object *obj, OutputDev *out, GfxState *state, int patternRefNum)
{
    GfxPattern *pattern;
    Object obj1;

    if (obj->isDict()) {
        obj1 = obj->dictLookup("PatternType");
    } else if (obj->isStream()) {
        obj1 = obj->streamGetDict()->lookup("PatternType");
    } else {
        return nullptr;
    }
    pattern = nullptr;
    if (obj1.isInt() && obj1.getInt() == 1) {
        pattern = GfxTilingPattern::parse(obj, patternRefNum);
    } else if (obj1.isInt() && obj1.getInt() == 2) {
        pattern = GfxShadingPattern::parse(res, obj, out, state, patternRefNum);
    }
    return pattern;
}

//------------------------------------------------------------------------
// GfxTilingPattern
//------------------------------------------------------------------------

GfxTilingPattern *GfxTilingPattern::parse(Object *patObj, int patternRefNum)
{
    Dict *dict;
    int paintTypeA, tilingTypeA;
    double bboxA[4], matrixA[6];
    double xStepA, yStepA;
    Object resDictA;
    Object obj1;
    int i;

    if (!patObj->isStream()) {
        return nullptr;
    }
    dict = patObj->streamGetDict();

    obj1 = dict->lookup("PaintType");
    if (obj1.isInt()) {
        paintTypeA = obj1.getInt();
    } else {
        paintTypeA = 1;
        error(errSyntaxWarning, -1, "Invalid or missing PaintType in pattern");
    }
    obj1 = dict->lookup("TilingType");
    if (obj1.isInt()) {
        tilingTypeA = obj1.getInt();
    } else {
        tilingTypeA = 1;
        error(errSyntaxWarning, -1, "Invalid or missing TilingType in pattern");
    }
    bboxA[0] = bboxA[1] = 0;
    bboxA[2] = bboxA[3] = 1;
    obj1 = dict->lookup("BBox");
    if (obj1.isArray() && obj1.arrayGetLength() == 4) {
        for (i = 0; i < 4; ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isNum()) {
                bboxA[i] = obj2.getNum();
            }
        }
    } else {
        error(errSyntaxWarning, -1, "Invalid or missing BBox in pattern");
    }
    obj1 = dict->lookup("XStep");
    if (obj1.isNum()) {
        xStepA = obj1.getNum();
    } else {
        xStepA = 1;
        error(errSyntaxWarning, -1, "Invalid or missing XStep in pattern");
    }
    obj1 = dict->lookup("YStep");
    if (obj1.isNum()) {
        yStepA = obj1.getNum();
    } else {
        yStepA = 1;
        error(errSyntaxWarning, -1, "Invalid or missing YStep in pattern");
    }
    resDictA = dict->lookup("Resources");
    if (!resDictA.isDict()) {
        error(errSyntaxWarning, -1, "Invalid or missing Resources in pattern");
    }
    matrixA[0] = 1;
    matrixA[1] = 0;
    matrixA[2] = 0;
    matrixA[3] = 1;
    matrixA[4] = 0;
    matrixA[5] = 0;
    obj1 = dict->lookup("Matrix");
    if (obj1.isArray() && obj1.arrayGetLength() == 6) {
        for (i = 0; i < 6; ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isNum()) {
                matrixA[i] = obj2.getNum();
            }
        }
    }

    return new GfxTilingPattern(paintTypeA, tilingTypeA, bboxA, xStepA, yStepA, &resDictA, matrixA, patObj, patternRefNum);
}

GfxTilingPattern::GfxTilingPattern(int paintTypeA, int tilingTypeA, const double *bboxA, double xStepA, double yStepA, const Object *resDictA, const double *matrixA, const Object *contentStreamA, int patternRefNumA)
    : GfxPattern(1, patternRefNumA)
{
    int i;

    paintType = paintTypeA;
    tilingType = tilingTypeA;
    for (i = 0; i < 4; ++i) {
        bbox[i] = bboxA[i];
    }
    xStep = xStepA;
    yStep = yStepA;
    resDict = resDictA->copy();
    for (i = 0; i < 6; ++i) {
        matrix[i] = matrixA[i];
    }
    contentStream = contentStreamA->copy();
}

GfxTilingPattern::~GfxTilingPattern() { }

GfxPattern *GfxTilingPattern::copy() const
{
    return new GfxTilingPattern(paintType, tilingType, bbox, xStep, yStep, &resDict, matrix, &contentStream, getPatternRefNum());
}

//------------------------------------------------------------------------
// GfxShadingPattern
//------------------------------------------------------------------------

GfxShadingPattern *GfxShadingPattern::parse(GfxResources *res, Object *patObj, OutputDev *out, GfxState *state, int patternRefNum)
{
    Dict *dict;
    GfxShading *shadingA;
    double matrixA[6];
    Object obj1;
    int i;

    if (!patObj->isDict()) {
        return nullptr;
    }
    dict = patObj->getDict();

    obj1 = dict->lookup("Shading");
    shadingA = GfxShading::parse(res, &obj1, out, state);
    if (!shadingA) {
        return nullptr;
    }

    matrixA[0] = 1;
    matrixA[1] = 0;
    matrixA[2] = 0;
    matrixA[3] = 1;
    matrixA[4] = 0;
    matrixA[5] = 0;
    obj1 = dict->lookup("Matrix");
    if (obj1.isArray() && obj1.arrayGetLength() == 6) {
        for (i = 0; i < 6; ++i) {
            Object obj2 = obj1.arrayGet(i);
            if (obj2.isNum()) {
                matrixA[i] = obj2.getNum();
            }
        }
    }

    return new GfxShadingPattern(shadingA, matrixA, patternRefNum);
}

GfxShadingPattern::GfxShadingPattern(GfxShading *shadingA, const double *matrixA, int patternRefNumA) : GfxPattern(2, patternRefNumA)
{
    int i;

    shading = shadingA;
    for (i = 0; i < 6; ++i) {
        matrix[i] = matrixA[i];
    }
}

GfxShadingPattern::~GfxShadingPattern()
{
    delete shading;
}

GfxPattern *GfxShadingPattern::copy() const
{
    return new GfxShadingPattern(shading->copy(), matrix, getPatternRefNum());
}

//------------------------------------------------------------------------
// GfxShading
//------------------------------------------------------------------------

GfxShading::GfxShading(int typeA)
{
    type = typeA;
    colorSpace = nullptr;
}

GfxShading::GfxShading(const GfxShading *shading)
{
    int i;

    type = shading->type;
    colorSpace = shading->colorSpace->copy();
    for (i = 0; i < gfxColorMaxComps; ++i) {
        background.c[i] = shading->background.c[i];
    }
    hasBackground = shading->hasBackground;
    bbox_xMin = shading->bbox_xMin;
    bbox_yMin = shading->bbox_yMin;
    bbox_xMax = shading->bbox_xMax;
    bbox_yMax = shading->bbox_yMax;
    hasBBox = shading->hasBBox;
}

GfxShading::~GfxShading()
{
    if (colorSpace) {
        delete colorSpace;
    }
}

GfxShading *GfxShading::parse(GfxResources *res, Object *obj, OutputDev *out, GfxState *state)
{
    GfxShading *shading;
    Dict *dict;
    int typeA;
    Object obj1;

    if (obj->isDict()) {
        dict = obj->getDict();
    } else if (obj->isStream()) {
        dict = obj->streamGetDict();
    } else {
        return nullptr;
    }

    obj1 = dict->lookup("ShadingType");
    if (!obj1.isInt()) {
        error(errSyntaxWarning, -1, "Invalid ShadingType in shading dictionary");
        return nullptr;
    }
    typeA = obj1.getInt();

    switch (typeA) {
    case 1:
        shading = GfxFunctionShading::parse(res, dict, out, state);
        break;
    case 2:
        shading = GfxAxialShading::parse(res, dict, out, state);
        break;
    case 3:
        shading = GfxRadialShading::parse(res, dict, out, state);
        break;
    case 4:
        if (obj->isStream()) {
            shading = GfxGouraudTriangleShading::parse(res, 4, dict, obj->getStream(), out, state);
        } else {
            error(errSyntaxWarning, -1, "Invalid Type 4 shading object");
            goto err1;
        }
        break;
    case 5:
        if (obj->isStream()) {
            shading = GfxGouraudTriangleShading::parse(res, 5, dict, obj->getStream(), out, state);
        } else {
            error(errSyntaxWarning, -1, "Invalid Type 5 shading object");
            goto err1;
        }
        break;
    case 6:
        if (obj->isStream()) {
            shading = GfxPatchMeshShading::parse(res, 6, dict, obj->getStream(), out, state);
        } else {
            error(errSyntaxWarning, -1, "Invalid Type 6 shading object");
            goto err1;
        }
        break;
    case 7:
        if (obj->isStream()) {
            shading = GfxPatchMeshShading::parse(res, 7, dict, obj->getStream(), out, state);
        } else {
            error(errSyntaxWarning, -1, "Invalid Type 7 shading object");
            goto err1;
        }
        break;
    default:
        error(errSyntaxWarning, -1, "Unimplemented shading type {0:d}", typeA);
        goto err1;
    }

    return shading;

err1:
    return nullptr;
}

bool GfxShading::init(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    Object obj1;
    int i;

    obj1 = dict->lookup("ColorSpace");
    if (!(colorSpace = GfxColorSpace::parse(res, &obj1, out, state))) {
        error(errSyntaxWarning, -1, "Bad color space in shading dictionary");
        return false;
    }

    for (i = 0; i < gfxColorMaxComps; ++i) {
        background.c[i] = 0;
    }
    hasBackground = false;
    obj1 = dict->lookup("Background");
    if (obj1.isArray()) {
        if (obj1.arrayGetLength() == colorSpace->getNComps()) {
            hasBackground = true;
            for (i = 0; i < colorSpace->getNComps(); ++i) {
                Object obj2 = obj1.arrayGet(i);
                background.c[i] = dblToCol(obj2.getNum(&hasBackground));
            }
            if (!hasBackground) {
                error(errSyntaxWarning, -1, "Bad Background in shading dictionary");
            }
        } else {
            error(errSyntaxWarning, -1, "Bad Background in shading dictionary");
        }
    }

    bbox_xMin = bbox_yMin = bbox_xMax = bbox_yMax = 0;
    hasBBox = false;
    obj1 = dict->lookup("BBox");
    if (obj1.isArray()) {
        if (obj1.arrayGetLength() == 4) {
            hasBBox = true;
            bbox_xMin = obj1.arrayGet(0).getNum(&hasBBox);
            bbox_yMin = obj1.arrayGet(1).getNum(&hasBBox);
            bbox_xMax = obj1.arrayGet(2).getNum(&hasBBox);
            bbox_yMax = obj1.arrayGet(3).getNum(&hasBBox);
            if (!hasBBox) {
                error(errSyntaxWarning, -1, "Bad BBox in shading dictionary (Values not numbers)");
            }
        } else {
            error(errSyntaxWarning, -1, "Bad BBox in shading dictionary");
        }
    }

    return true;
}

//------------------------------------------------------------------------
// GfxFunctionShading
//------------------------------------------------------------------------

GfxFunctionShading::GfxFunctionShading(double x0A, double y0A, double x1A, double y1A, const double *matrixA, std::vector<std::unique_ptr<Function>> &&funcsA) : GfxShading(1), funcs(std::move(funcsA))
{
    x0 = x0A;
    y0 = y0A;
    x1 = x1A;
    y1 = y1A;
    for (int i = 0; i < 6; ++i) {
        matrix[i] = matrixA[i];
    }
}

GfxFunctionShading::GfxFunctionShading(const GfxFunctionShading *shading) : GfxShading(shading)
{
    x0 = shading->x0;
    y0 = shading->y0;
    x1 = shading->x1;
    y1 = shading->y1;
    for (int i = 0; i < 6; ++i) {
        matrix[i] = shading->matrix[i];
    }
    for (const auto &f : shading->funcs) {
        funcs.emplace_back(f->copy());
    }
}

GfxFunctionShading::~GfxFunctionShading() { }

GfxFunctionShading *GfxFunctionShading::parse(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    GfxFunctionShading *shading;
    double x0A, y0A, x1A, y1A;
    double matrixA[6];
    std::vector<std::unique_ptr<Function>> funcsA;
    Object obj1;
    int i;

    x0A = y0A = 0;
    x1A = y1A = 1;
    obj1 = dict->lookup("Domain");
    if (obj1.isArray() && obj1.arrayGetLength() == 4) {
        bool decodeOk = true;
        x0A = obj1.arrayGet(0).getNum(&decodeOk);
        x1A = obj1.arrayGet(1).getNum(&decodeOk);
        y0A = obj1.arrayGet(2).getNum(&decodeOk);
        y1A = obj1.arrayGet(3).getNum(&decodeOk);

        if (!decodeOk) {
            error(errSyntaxWarning, -1, "Invalid Domain array in function shading dictionary");
            return nullptr;
        }
    }

    matrixA[0] = 1;
    matrixA[1] = 0;
    matrixA[2] = 0;
    matrixA[3] = 1;
    matrixA[4] = 0;
    matrixA[5] = 0;
    obj1 = dict->lookup("Matrix");
    if (obj1.isArray() && obj1.arrayGetLength() == 6) {
        bool decodeOk = true;
        matrixA[0] = obj1.arrayGet(0).getNum(&decodeOk);
        matrixA[1] = obj1.arrayGet(1).getNum(&decodeOk);
        matrixA[2] = obj1.arrayGet(2).getNum(&decodeOk);
        matrixA[3] = obj1.arrayGet(3).getNum(&decodeOk);
        matrixA[4] = obj1.arrayGet(4).getNum(&decodeOk);
        matrixA[5] = obj1.arrayGet(5).getNum(&decodeOk);

        if (!decodeOk) {
            error(errSyntaxWarning, -1, "Invalid Matrix array in function shading dictionary");
            return nullptr;
        }
    }

    obj1 = dict->lookup("Function");
    if (obj1.isArray()) {
        const int nFuncsA = obj1.arrayGetLength();
        if (nFuncsA > gfxColorMaxComps || nFuncsA <= 0) {
            error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
            return nullptr;
        }
        for (i = 0; i < nFuncsA; ++i) {
            Object obj2 = obj1.arrayGet(i);
            Function *f = Function::parse(&obj2);
            if (!f) {
                return nullptr;
            }
            funcsA.emplace_back(f);
        }
    } else {
        Function *f = Function::parse(&obj1);
        if (!f) {
            return nullptr;
        }
        funcsA.emplace_back(f);
    }

    shading = new GfxFunctionShading(x0A, y0A, x1A, y1A, matrixA, std::move(funcsA));
    if (!shading->init(res, dict, out, state)) {
        delete shading;
        return nullptr;
    }
    return shading;
}

bool GfxFunctionShading::init(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    const bool parentInit = GfxShading::init(res, dict, out, state);
    if (!parentInit) {
        return false;
    }

    // funcs needs to be one of the two:
    //  * One function 2-in -> nComps-out
    //  * nComps functions 2-in -> 1-out
    const int nComps = colorSpace->getNComps();
    const int nFuncs = funcs.size();
    if (nFuncs == 1) {
        if (funcs[0]->getInputSize() != 2) {
            error(errSyntaxWarning, -1, "GfxFunctionShading: function with input size != 2");
            return false;
        }
        if (funcs[0]->getOutputSize() != nComps) {
            error(errSyntaxWarning, -1, "GfxFunctionShading: function with wrong output size");
            return false;
        }
    } else if (nFuncs == nComps) {
        for (const std::unique_ptr<Function> &f : funcs) {
            if (f->getInputSize() != 2) {
                error(errSyntaxWarning, -1, "GfxFunctionShading: function with input size != 2");
                return false;
            }
            if (f->getOutputSize() != 1) {
                error(errSyntaxWarning, -1, "GfxFunctionShading: function with wrong output size");
                return false;
            }
        }
    } else {
        return false;
    }

    return true;
}

GfxShading *GfxFunctionShading::copy() const
{
    return new GfxFunctionShading(this);
}

void GfxFunctionShading::getColor(double x, double y, GfxColor *color) const
{
    double in[2], out[gfxColorMaxComps];

    // NB: there can be one function with n outputs or n functions with
    // one output each (where n = number of color components)
    for (double &i : out) {
        i = 0;
    }
    in[0] = x;
    in[1] = y;
    for (int i = 0; i < getNFuncs(); ++i) {
        funcs[i]->transform(in, &out[i]);
    }
    for (int i = 0; i < gfxColorMaxComps; ++i) {
        color->c[i] = dblToCol(out[i]);
    }
}

//------------------------------------------------------------------------
// GfxUnivariateShading
//------------------------------------------------------------------------

GfxUnivariateShading::GfxUnivariateShading(int typeA, double t0A, double t1A, std::vector<std::unique_ptr<Function>> &&funcsA, bool extend0A, bool extend1A) : GfxShading(typeA), funcs(std::move(funcsA))
{
    t0 = t0A;
    t1 = t1A;
    extend0 = extend0A;
    extend1 = extend1A;

    cacheSize = 0;
    lastMatch = 0;
    cacheBounds = nullptr;
    cacheCoeff = nullptr;
    cacheValues = nullptr;
}

GfxUnivariateShading::GfxUnivariateShading(const GfxUnivariateShading *shading) : GfxShading(shading)
{
    t0 = shading->t0;
    t1 = shading->t1;
    for (const auto &f : shading->funcs) {
        funcs.emplace_back(f->copy());
    }
    extend0 = shading->extend0;
    extend1 = shading->extend1;

    cacheSize = 0;
    lastMatch = 0;
    cacheBounds = nullptr;
    cacheCoeff = nullptr;
    cacheValues = nullptr;
}

GfxUnivariateShading::~GfxUnivariateShading()
{
    gfree(cacheBounds);
}

int GfxUnivariateShading::getColor(double t, GfxColor *color)
{
    double out[gfxColorMaxComps];

    // NB: there can be one function with n outputs or n functions with
    // one output each (where n = number of color components)
    const int nComps = getNFuncs() * funcs[0]->getOutputSize();

    if (cacheSize > 0) {
        double x, ix, *l, *u, *upper;

        if (cacheBounds[lastMatch - 1] >= t) {
            upper = std::lower_bound(cacheBounds, cacheBounds + lastMatch - 1, t);
            lastMatch = static_cast<int>(upper - cacheBounds);
            lastMatch = std::min<int>(std::max<int>(1, lastMatch), cacheSize - 1);
        } else if (cacheBounds[lastMatch] < t) {
            upper = std::lower_bound(cacheBounds + lastMatch + 1, cacheBounds + cacheSize, t);
            lastMatch = static_cast<int>(upper - cacheBounds);
            lastMatch = std::min<int>(std::max<int>(1, lastMatch), cacheSize - 1);
        }

        x = (t - cacheBounds[lastMatch - 1]) * cacheCoeff[lastMatch];
        ix = 1.0 - x;
        u = cacheValues + lastMatch * nComps;
        l = u - nComps;

        for (int i = 0; i < nComps; ++i) {
            out[i] = ix * l[i] + x * u[i];
        }
    } else {
        for (int i = 0; i < nComps; ++i) {
            out[i] = 0;
        }
        for (int i = 0; i < getNFuncs(); ++i) {
            funcs[i]->transform(&t, &out[i]);
        }
    }

    for (int i = 0; i < nComps; ++i) {
        color->c[i] = dblToCol(out[i]);
    }
    return nComps;
}

void GfxUnivariateShading::setupCache(const Matrix *ctm, double xMin, double yMin, double xMax, double yMax)
{
    double sMin, sMax, tMin, tMax, upperBound;
    int i, j, nComps, maxSize;

    gfree(cacheBounds);
    cacheBounds = nullptr;
    cacheSize = 0;

    if (unlikely(getNFuncs() < 1)) {
        return;
    }

    // NB: there can be one function with n outputs or n functions with
    // one output each (where n = number of color components)
    nComps = getNFuncs() * funcs[0]->getOutputSize();

    getParameterRange(&sMin, &sMax, xMin, yMin, xMax, yMax);
    upperBound = ctm->norm() * getDistance(sMin, sMax);
    maxSize = static_cast<int>(ceil(upperBound));
    maxSize = std::max<int>(maxSize, 2);

    {
        double x[4], y[4];

        ctm->transform(xMin, yMin, &x[0], &y[0]);
        ctm->transform(xMax, yMin, &x[1], &y[1]);
        ctm->transform(xMin, yMax, &x[2], &y[2]);
        ctm->transform(xMax, yMax, &x[3], &y[3]);

        xMin = xMax = x[0];
        yMin = yMax = y[0];
        for (i = 1; i < 4; i++) {
            xMin = std::min<double>(xMin, x[i]);
            yMin = std::min<double>(yMin, y[i]);
            xMax = std::max<double>(xMax, x[i]);
            yMax = std::max<double>(yMax, y[i]);
        }
    }

    if (maxSize > (xMax - xMin) * (yMax - yMin)) {
        return;
    }

    if (t0 < t1) {
        tMin = t0 + sMin * (t1 - t0);
        tMax = t0 + sMax * (t1 - t0);
    } else {
        tMin = t0 + sMax * (t1 - t0);
        tMax = t0 + sMin * (t1 - t0);
    }

    cacheBounds = (double *)gmallocn_checkoverflow(maxSize, sizeof(double) * (nComps + 2));
    if (unlikely(!cacheBounds)) {
        return;
    }
    cacheCoeff = cacheBounds + maxSize;
    cacheValues = cacheCoeff + maxSize;

    if (cacheSize != 0) {
        for (j = 0; j < cacheSize; ++j) {
            cacheCoeff[j] = 1 / (cacheBounds[j + 1] - cacheBounds[j]);
        }
    } else if (tMax != tMin) {
        double step = (tMax - tMin) / (maxSize - 1);
        double coeff = (maxSize - 1) / (tMax - tMin);

        cacheSize = maxSize;

        for (j = 0; j < cacheSize; ++j) {
            cacheBounds[j] = tMin + j * step;
            cacheCoeff[j] = coeff;

            for (i = 0; i < nComps; ++i) {
                cacheValues[j * nComps + i] = 0;
            }
            for (i = 0; i < getNFuncs(); ++i) {
                funcs[i]->transform(&cacheBounds[j], &cacheValues[j * nComps + i]);
            }
        }
    }

    lastMatch = 1;
}

bool GfxUnivariateShading::init(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    const bool parentInit = GfxShading::init(res, dict, out, state);
    if (!parentInit) {
        return false;
    }

    // funcs needs to be one of the two:
    //  * One function 1-in -> nComps-out
    //  * nComps functions 1-in -> 1-out
    const int nComps = colorSpace->getNComps();
    const int nFuncs = funcs.size();
    if (nFuncs == 1) {
        if (funcs[0]->getInputSize() != 1) {
            error(errSyntaxWarning, -1, "GfxUnivariateShading: function with input size != 2");
            return false;
        }
        if (funcs[0]->getOutputSize() != nComps) {
            error(errSyntaxWarning, -1, "GfxUnivariateShading: function with wrong output size");
            return false;
        }
    } else if (nFuncs == nComps) {
        for (const std::unique_ptr<Function> &f : funcs) {
            if (f->getInputSize() != 1) {
                error(errSyntaxWarning, -1, "GfxUnivariateShading: function with input size != 2");
                return false;
            }
            if (f->getOutputSize() != 1) {
                error(errSyntaxWarning, -1, "GfxUnivariateShading: function with wrong output size");
                return false;
            }
        }
    } else {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------
// GfxAxialShading
//------------------------------------------------------------------------

GfxAxialShading::GfxAxialShading(double x0A, double y0A, double x1A, double y1A, double t0A, double t1A, std::vector<std::unique_ptr<Function>> &&funcsA, bool extend0A, bool extend1A)
    : GfxUnivariateShading(2, t0A, t1A, std::move(funcsA), extend0A, extend1A)
{
    x0 = x0A;
    y0 = y0A;
    x1 = x1A;
    y1 = y1A;
}

GfxAxialShading::GfxAxialShading(const GfxAxialShading *shading) : GfxUnivariateShading(shading)
{
    x0 = shading->x0;
    y0 = shading->y0;
    x1 = shading->x1;
    y1 = shading->y1;
}

GfxAxialShading::~GfxAxialShading() { }

GfxAxialShading *GfxAxialShading::parse(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    GfxAxialShading *shading;
    double x0A, y0A, x1A, y1A;
    double t0A, t1A;
    std::vector<std::unique_ptr<Function>> funcsA;
    bool extend0A, extend1A;
    Object obj1;

    x0A = y0A = x1A = y1A = 0;
    obj1 = dict->lookup("Coords");
    if (obj1.isArray() && obj1.arrayGetLength() == 4) {
        x0A = obj1.arrayGet(0).getNumWithDefaultValue(0);
        y0A = obj1.arrayGet(1).getNumWithDefaultValue(0);
        x1A = obj1.arrayGet(2).getNumWithDefaultValue(0);
        y1A = obj1.arrayGet(3).getNumWithDefaultValue(0);
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid Coords in shading dictionary");
        return nullptr;
    }

    t0A = 0;
    t1A = 1;
    obj1 = dict->lookup("Domain");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        t0A = obj1.arrayGet(0).getNumWithDefaultValue(0);
        t1A = obj1.arrayGet(1).getNumWithDefaultValue(1);
    }

    obj1 = dict->lookup("Function");
    if (obj1.isArray()) {
        const int nFuncsA = obj1.arrayGetLength();
        if (nFuncsA > gfxColorMaxComps || nFuncsA == 0) {
            error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
            return nullptr;
        }
        for (int i = 0; i < nFuncsA; ++i) {
            Object obj2 = obj1.arrayGet(i);
            Function *f = Function::parse(&obj2);
            if (!f) {
                return nullptr;
            }
            funcsA.emplace_back(f);
        }
    } else {
        Function *f = Function::parse(&obj1);
        if (!f) {
            return nullptr;
        }
        funcsA.emplace_back(f);
    }

    extend0A = extend1A = false;
    obj1 = dict->lookup("Extend");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        Object obj2 = obj1.arrayGet(0);
        if (obj2.isBool()) {
            extend0A = obj2.getBool();
        } else {
            error(errSyntaxWarning, -1, "Invalid axial shading extend (0)");
        }
        obj2 = obj1.arrayGet(1);
        if (obj2.isBool()) {
            extend1A = obj2.getBool();
        } else {
            error(errSyntaxWarning, -1, "Invalid axial shading extend (1)");
        }
    }

    shading = new GfxAxialShading(x0A, y0A, x1A, y1A, t0A, t1A, std::move(funcsA), extend0A, extend1A);
    if (!shading->init(res, dict, out, state)) {
        delete shading;
        shading = nullptr;
    }
    return shading;
}

GfxShading *GfxAxialShading::copy() const
{
    return new GfxAxialShading(this);
}

double GfxAxialShading::getDistance(double sMin, double sMax) const
{
    double xMin, yMin, xMax, yMax;

    xMin = x0 + sMin * (x1 - x0);
    yMin = y0 + sMin * (y1 - y0);
    xMax = x0 + sMax * (x1 - x0);
    yMax = y0 + sMax * (y1 - y0);

    return hypot(xMax - xMin, yMax - yMin);
}

void GfxAxialShading::getParameterRange(double *lower, double *upper, double xMin, double yMin, double xMax, double yMax)
{
    double pdx, pdy, invsqnorm, tdx, tdy, t, range[2];

    // Linear gradients are orthogonal to the line passing through their
    // extremes. Because of convexity, the parameter range can be
    // computed as the convex hull (one the real line) of the parameter
    // values of the 4 corners of the box.
    //
    // The parameter value t for a point (x,y) can be computed as:
    //
    //   t = (p2 - p1) . (x,y) / |p2 - p1|^2
    //
    // t0  is the t value for the top left corner
    // tdx is the difference between left and right corners
    // tdy is the difference between top and bottom corners

    pdx = x1 - x0;
    pdy = y1 - y0;
    const double invsqnorm_denominator = (pdx * pdx + pdy * pdy);
    if (unlikely(invsqnorm_denominator == 0)) {
        *lower = 0;
        *upper = 0;
        return;
    }
    invsqnorm = 1.0 / invsqnorm_denominator;
    pdx *= invsqnorm;
    pdy *= invsqnorm;

    t = (xMin - x0) * pdx + (yMin - y0) * pdy;
    tdx = (xMax - xMin) * pdx;
    tdy = (yMax - yMin) * pdy;

    // Because of the linearity of the t value, tdx can simply be added
    // the t0 to move along the top edge. After this, *lower and *upper
    // represent the parameter range for the top edge, so extending it
    // to include the whole box simply requires adding tdy to the
    // correct extreme.

    range[0] = range[1] = t;
    if (tdx < 0) {
        range[0] += tdx;
    } else {
        range[1] += tdx;
    }

    if (tdy < 0) {
        range[0] += tdy;
    } else {
        range[1] += tdy;
    }

    *lower = std::max<double>(0., std::min<double>(1., range[0]));
    *upper = std::max<double>(0., std::min<double>(1., range[1]));
}

//------------------------------------------------------------------------
// GfxRadialShading
//------------------------------------------------------------------------

#ifndef RADIAL_EPSILON
#    define RADIAL_EPSILON (1. / 1024 / 1024)
#endif

GfxRadialShading::GfxRadialShading(double x0A, double y0A, double r0A, double x1A, double y1A, double r1A, double t0A, double t1A, std::vector<std::unique_ptr<Function>> &&funcsA, bool extend0A, bool extend1A)
    : GfxUnivariateShading(3, t0A, t1A, std::move(funcsA), extend0A, extend1A)
{
    x0 = x0A;
    y0 = y0A;
    r0 = r0A;
    x1 = x1A;
    y1 = y1A;
    r1 = r1A;
}

GfxRadialShading::GfxRadialShading(const GfxRadialShading *shading) : GfxUnivariateShading(shading)
{
    x0 = shading->x0;
    y0 = shading->y0;
    r0 = shading->r0;
    x1 = shading->x1;
    y1 = shading->y1;
    r1 = shading->r1;
}

GfxRadialShading::~GfxRadialShading() { }

GfxRadialShading *GfxRadialShading::parse(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    GfxRadialShading *shading;
    double x0A, y0A, r0A, x1A, y1A, r1A;
    double t0A, t1A;
    std::vector<std::unique_ptr<Function>> funcsA;
    bool extend0A, extend1A;
    Object obj1;
    int i;

    x0A = y0A = r0A = x1A = y1A = r1A = 0;
    obj1 = dict->lookup("Coords");
    if (obj1.isArray() && obj1.arrayGetLength() == 6) {
        x0A = obj1.arrayGet(0).getNumWithDefaultValue(0);
        y0A = obj1.arrayGet(1).getNumWithDefaultValue(0);
        r0A = obj1.arrayGet(2).getNumWithDefaultValue(0);
        x1A = obj1.arrayGet(3).getNumWithDefaultValue(0);
        y1A = obj1.arrayGet(4).getNumWithDefaultValue(0);
        r1A = obj1.arrayGet(5).getNumWithDefaultValue(0);
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid Coords in shading dictionary");
        return nullptr;
    }

    t0A = 0;
    t1A = 1;
    obj1 = dict->lookup("Domain");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        t0A = obj1.arrayGet(0).getNumWithDefaultValue(0);
        t1A = obj1.arrayGet(1).getNumWithDefaultValue(1);
    }

    obj1 = dict->lookup("Function");
    if (obj1.isArray()) {
        const int nFuncsA = obj1.arrayGetLength();
        if (nFuncsA > gfxColorMaxComps) {
            error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
            return nullptr;
        }
        for (i = 0; i < nFuncsA; ++i) {
            Object obj2 = obj1.arrayGet(i);
            Function *f = Function::parse(&obj2);
            if (!f) {
                return nullptr;
            }
            funcsA.emplace_back(f);
        }
    } else {
        Function *f = Function::parse(&obj1);
        if (!f) {
            return nullptr;
        }
        funcsA.emplace_back(f);
    }

    extend0A = extend1A = false;
    obj1 = dict->lookup("Extend");
    if (obj1.isArray() && obj1.arrayGetLength() == 2) {
        extend0A = obj1.arrayGet(0).getBoolWithDefaultValue(false);
        extend1A = obj1.arrayGet(1).getBoolWithDefaultValue(false);
    }

    shading = new GfxRadialShading(x0A, y0A, r0A, x1A, y1A, r1A, t0A, t1A, std::move(funcsA), extend0A, extend1A);
    if (!shading->init(res, dict, out, state)) {
        delete shading;
        return nullptr;
    }
    return shading;
}

GfxShading *GfxRadialShading::copy() const
{
    return new GfxRadialShading(this);
}

double GfxRadialShading::getDistance(double sMin, double sMax) const
{
    double xMin, yMin, rMin, xMax, yMax, rMax;

    xMin = x0 + sMin * (x1 - x0);
    yMin = y0 + sMin * (y1 - y0);
    rMin = r0 + sMin * (r1 - r0);

    xMax = x0 + sMax * (x1 - x0);
    yMax = y0 + sMax * (y1 - y0);
    rMax = r0 + sMax * (r1 - r0);

    return hypot(xMax - xMin, yMax - yMin) + fabs(rMax - rMin);
}

// extend range, adapted from cairo, radialExtendRange
static bool radialExtendRange(double range[2], double value, bool valid)
{
    if (!valid) {
        range[0] = range[1] = value;
    } else if (value < range[0]) {
        range[0] = value;
    } else if (value > range[1]) {
        range[1] = value;
    }

    return true;
}

inline void radialEdge(double num, double den, double delta, double lower, double upper, double dr, double mindr, bool &valid, double *range)
{
    if (fabs(den) >= RADIAL_EPSILON) {
        double t_edge, v;
        t_edge = (num) / (den);
        v = t_edge * (delta);
        if (t_edge * dr >= mindr && (lower) <= v && v <= (upper)) {
            valid = radialExtendRange(range, t_edge, valid);
        }
    }
}

inline void radialCorner1(double x, double y, double &b, double dx, double dy, double cr, double dr, double mindr, bool &valid, double *range)
{
    b = (x)*dx + (y)*dy + cr * dr;
    if (fabs(b) >= RADIAL_EPSILON) {
        double t_corner;
        double x2 = (x) * (x);
        double y2 = (y) * (y);
        double cr2 = (cr) * (cr);
        double c = x2 + y2 - cr2;

        t_corner = 0.5 * c / b;
        if (t_corner * dr >= mindr) {
            valid = radialExtendRange(range, t_corner, valid);
        }
    }
}

inline void radialCorner2(double x, double y, double a, double &b, double &c, double &d, double dx, double dy, double cr, double inva, double dr, double mindr, bool &valid, double *range)
{
    b = (x)*dx + (y)*dy + cr * dr;
    c = (x) * (x) + (y) * (y)-cr * cr;
    d = b * b - a * c;
    if (d >= 0) {
        double t_corner;

        d = sqrt(d);
        t_corner = (b + d) * inva;
        if (t_corner * dr >= mindr) {
            valid = radialExtendRange(range, t_corner, valid);
        }
        t_corner = (b - d) * inva;
        if (t_corner * dr >= mindr) {
            valid = radialExtendRange(range, t_corner, valid);
        }
    }
}
void GfxRadialShading::getParameterRange(double *lower, double *upper, double xMin, double yMin, double xMax, double yMax)
{
    double cx, cy, cr, dx, dy, dr;
    double a, x_focus, y_focus;
    double mindr, minx, miny, maxx, maxy;
    double range[2];
    bool valid;

    // A radial pattern is considered degenerate if it can be
    // represented as a solid or clear pattern.  This corresponds to one
    // of the two cases:
    //
    // 1) The radii are both very small:
    //      |dr| < FLT_EPSILON && min (r0, r1) < FLT_EPSILON
    //
    // 2) The two circles have about the same radius and are very
    //    close to each other (approximately a cylinder gradient that
    //    doesn't move with the parameter):
    //      |dr| < FLT_EPSILON && max (|dx|, |dy|) < 2 * FLT_EPSILON

    if (xMin >= xMax || yMin >= yMax || (fabs(r0 - r1) < RADIAL_EPSILON && (std::min<double>(r0, r1) < RADIAL_EPSILON || std::max<double>(fabs(x0 - x1), fabs(y0 - y1)) < 2 * RADIAL_EPSILON))) {
        *lower = *upper = 0;
        return;
    }

    range[0] = range[1] = 0;
    valid = false;

    x_focus = y_focus = 0; // silence gcc

    cx = x0;
    cy = y0;
    cr = r0;
    dx = x1 - cx;
    dy = y1 - cy;
    dr = r1 - cr;

    // translate by -(cx, cy) to simplify computations
    xMin -= cx;
    yMin -= cy;
    xMax -= cx;
    yMax -= cy;

    // enlarge boundaries slightly to avoid rounding problems in the
    // parameter range computation
    xMin -= RADIAL_EPSILON;
    yMin -= RADIAL_EPSILON;
    xMax += RADIAL_EPSILON;
    yMax += RADIAL_EPSILON;

    // enlarge boundaries even more to avoid rounding problems when
    // testing if a point belongs to the box
    minx = xMin - RADIAL_EPSILON;
    miny = yMin - RADIAL_EPSILON;
    maxx = xMax + RADIAL_EPSILON;
    maxy = yMax + RADIAL_EPSILON;

    // we dont' allow negative radiuses, so we will be checking that
    // t*dr >= mindr to consider t valid
    mindr = -(cr + RADIAL_EPSILON);

    // After the previous transformations, the start circle is centered
    // in the origin and has radius cr. A 1-unit change in the t
    // parameter corresponds to dx,dy,dr changes in the x,y,r of the
    // circle (center coordinates, radius).
    //
    // To compute the minimum range needed to correctly draw the
    // pattern, we start with an empty range and extend it to include
    // the circles touching the bounding box or within it.

    // Focus, the point where the circle has radius == 0.
    //
    // r = cr + t * dr = 0
    // t = -cr / dr
    //
    // If the radius is constant (dr == 0) there is no focus (the
    // gradient represents a cylinder instead of a cone).
    if (fabs(dr) >= RADIAL_EPSILON) {
        double t_focus;

        t_focus = -cr / dr;
        x_focus = t_focus * dx;
        y_focus = t_focus * dy;
        if (minx <= x_focus && x_focus <= maxx && miny <= y_focus && y_focus <= maxy) {
            valid = radialExtendRange(range, t_focus, valid);
        }
    }

    // Circles externally tangent to box edges.
    //
    // All circles have center in (dx, dy) * t
    //
    // If the circle is tangent to the line defined by the edge of the
    // box, then at least one of the following holds true:
    //
    //   (dx*t) + (cr + dr*t) == x0 (left   edge)
    //   (dx*t) - (cr + dr*t) == x1 (right  edge)
    //   (dy*t) + (cr + dr*t) == y0 (top    edge)
    //   (dy*t) - (cr + dr*t) == y1 (bottom edge)
    //
    // The solution is only valid if the tangent point is actually on
    // the edge, i.e. if its y coordinate is in [y0,y1] for left/right
    // edges and if its x coordinate is in [x0,x1] for top/bottom edges.
    //
    // For the first equation:
    //
    //   (dx + dr) * t = x0 - cr
    //   t = (x0 - cr) / (dx + dr)
    //   y = dy * t
    //
    // in the code this becomes:
    //
    //   t_edge = (num) / (den)
    //   v = (delta) * t_edge
    //
    // If the denominator in t is 0, the pattern is tangent to a line
    // parallel to the edge under examination. The corner-case where the
    // boundary line is the same as the edge is handled by the focus
    // point case and/or by the a==0 case.

    // circles tangent (externally) to left/right/top/bottom edge
    radialEdge(xMin - cr, dx + dr, dy, miny, maxy, dr, mindr, valid, range);
    radialEdge(xMax + cr, dx - dr, dy, miny, maxy, dr, mindr, valid, range);
    radialEdge(yMin - cr, dy + dr, dx, minx, maxx, dr, mindr, valid, range);
    radialEdge(yMax + cr, dy - dr, dx, minx, maxx, dr, mindr, valid, range);

    // Circles passing through a corner.
    //
    // A circle passing through the point (x,y) satisfies:
    //
    // (x-t*dx)^2 + (y-t*dy)^2 == (cr + t*dr)^2
    //
    // If we set:
    //   a = dx^2 + dy^2 - dr^2
    //   b = x*dx + y*dy + cr*dr
    //   c = x^2 + y^2 - cr^2
    // we have:
    //   a*t^2 - 2*b*t + c == 0

    a = dx * dx + dy * dy - dr * dr;
    if (fabs(a) < RADIAL_EPSILON * RADIAL_EPSILON) {
        double b;

        // Ensure that gradients with both a and dr small are
        // considered degenerate.
        // The floating point version of the degeneracy test implemented
        // in _radial_pattern_is_degenerate() is:
        //
        //  1) The circles are practically the same size:
        //     |dr| < RADIAL_EPSILON
        //  AND
        //  2a) The circles are both very small:
        //      min (r0, r1) < RADIAL_EPSILON
        //   OR
        //  2b) The circles are very close to each other:
        //      max (|dx|, |dy|) < 2 * RADIAL_EPSILON
        //
        // Assuming that the gradient is not degenerate, we want to
        // show that |a| < RADIAL_EPSILON^2 implies |dr| >= RADIAL_EPSILON.
        //
        // If the gradient is not degenerate yet it has |dr| <
        // RADIAL_EPSILON, (2b) is false, thus:
        //
        //   max (|dx|, |dy|) >= 2*RADIAL_EPSILON
        // which implies:
        //   4*RADIAL_EPSILON^2 <= max (|dx|, |dy|)^2 <= dx^2 + dy^2
        //
        // From the definition of a, we get:
        //   a = dx^2 + dy^2 - dr^2 < RADIAL_EPSILON^2
        //   dx^2 + dy^2 - RADIAL_EPSILON^2 < dr^2
        //   3*RADIAL_EPSILON^2 < dr^2
        //
        // which is inconsistent with the hypotheses, thus |dr| <
        // RADIAL_EPSILON is false or the gradient is degenerate.

        assert(fabs(dr) >= RADIAL_EPSILON);

        // If a == 0, all the circles are tangent to a line in the
        // focus point. If this line is within the box extents, we
        // should add the circle with infinite radius, but this would
        // make the range unbounded. We will be limiting the range to
        // [0,1] anyway, so we simply add the biggest legitimate
        // circle (it happens for 0 or for 1).
        if (dr < 0) {
            valid = radialExtendRange(range, 0, valid);
        } else {
            valid = radialExtendRange(range, 1, valid);
        }

        // Nondegenerate, nonlimit circles passing through the corners.
        //
        // a == 0 && a*t^2 - 2*b*t + c == 0
        //
        // t = c / (2*b)
        //
        // The b == 0 case has just been handled, so we only have to
        // compute this if b != 0.

        // circles touching each corner
        radialCorner1(xMin, yMin, b, dx, dy, cr, dr, mindr, valid, range);
        radialCorner1(xMin, yMax, b, dx, dy, cr, dr, mindr, valid, range);
        radialCorner1(xMax, yMin, b, dx, dy, cr, dr, mindr, valid, range);
        radialCorner1(xMax, yMax, b, dx, dy, cr, dr, mindr, valid, range);
    } else {
        double inva, b, c, d;

        inva = 1 / a;

        // Nondegenerate, nonlimit circles passing through the corners.
        //
        // a != 0 && a*t^2 - 2*b*t + c == 0
        //
        // t = (b +- sqrt (b*b - a*c)) / a
        //
        // If the argument of sqrt() is negative, then no circle
        // passes through the corner.

        // circles touching each corner
        radialCorner2(xMin, yMin, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
        radialCorner2(xMin, yMax, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
        radialCorner2(xMax, yMin, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
        radialCorner2(xMax, yMax, a, b, c, d, dx, dy, cr, inva, dr, mindr, valid, range);
    }

    *lower = std::max<double>(0., std::min<double>(1., range[0]));
    *upper = std::max<double>(0., std::min<double>(1., range[1]));
}

//------------------------------------------------------------------------
// GfxShadingBitBuf
//------------------------------------------------------------------------

class GfxShadingBitBuf
{
public:
    explicit GfxShadingBitBuf(Stream *strA);
    ~GfxShadingBitBuf();
    GfxShadingBitBuf(const GfxShadingBitBuf &) = delete;
    GfxShadingBitBuf &operator=(const GfxShadingBitBuf &) = delete;
    bool getBits(int n, unsigned int *val);
    void flushBits();

private:
    Stream *str;
    int bitBuf;
    int nBits;
};

GfxShadingBitBuf::GfxShadingBitBuf(Stream *strA)
{
    str = strA;
    str->reset();
    bitBuf = 0;
    nBits = 0;
}

GfxShadingBitBuf::~GfxShadingBitBuf()
{
    str->close();
}

bool GfxShadingBitBuf::getBits(int n, unsigned int *val)
{
    unsigned int x;

    if (nBits >= n) {
        x = (bitBuf >> (nBits - n)) & ((1 << n) - 1);
        nBits -= n;
    } else {
        x = 0;
        if (nBits > 0) {
            x = bitBuf & ((1 << nBits) - 1);
            n -= nBits;
            nBits = 0;
        }
        while (n > 0) {
            if ((bitBuf = str->getChar()) == EOF) {
                nBits = 0;
                return false;
            }
            if (n >= 8) {
                x = (x << 8) | bitBuf;
                n -= 8;
            } else {
                x = (x << n) | (bitBuf >> (8 - n));
                nBits = 8 - n;
                n = 0;
            }
        }
    }
    *val = x;
    return true;
}

void GfxShadingBitBuf::flushBits()
{
    bitBuf = 0;
    nBits = 0;
}

//------------------------------------------------------------------------
// GfxGouraudTriangleShading
//------------------------------------------------------------------------

GfxGouraudTriangleShading::GfxGouraudTriangleShading(int typeA, GfxGouraudVertex *verticesA, int nVerticesA, int (*trianglesA)[3], int nTrianglesA, std::vector<std::unique_ptr<Function>> &&funcsA)
    : GfxShading(typeA), funcs(std::move(funcsA))
{
    vertices = verticesA;
    nVertices = nVerticesA;
    triangles = trianglesA;
    nTriangles = nTrianglesA;
}

GfxGouraudTriangleShading::GfxGouraudTriangleShading(const GfxGouraudTriangleShading *shading) : GfxShading(shading)
{
    nVertices = shading->nVertices;
    vertices = (GfxGouraudVertex *)gmallocn(nVertices, sizeof(GfxGouraudVertex));
    memcpy(vertices, shading->vertices, nVertices * sizeof(GfxGouraudVertex));
    nTriangles = shading->nTriangles;
    triangles = (int(*)[3])gmallocn(nTriangles * 3, sizeof(int));
    memcpy(triangles, shading->triangles, nTriangles * 3 * sizeof(int));
    for (const auto &f : shading->funcs) {
        funcs.emplace_back(f->copy());
    }
}

GfxGouraudTriangleShading::~GfxGouraudTriangleShading()
{
    gfree(vertices);
    gfree(triangles);
}

GfxGouraudTriangleShading *GfxGouraudTriangleShading::parse(GfxResources *res, int typeA, Dict *dict, Stream *str, OutputDev *out, GfxState *gfxState)
{
    GfxGouraudTriangleShading *shading;
    std::vector<std::unique_ptr<Function>> funcsA;
    int coordBits, compBits, flagBits, vertsPerRow, nRows;
    double xMin, xMax, yMin, yMax;
    double cMin[gfxColorMaxComps], cMax[gfxColorMaxComps];
    double xMul, yMul;
    double cMul[gfxColorMaxComps];
    GfxGouraudVertex *verticesA;
    int(*trianglesA)[3];
    int nComps, nVerticesA, nTrianglesA, vertSize, triSize;
    unsigned int x, y, flag;
    unsigned int c[gfxColorMaxComps];
    GfxShadingBitBuf *bitBuf;
    Object obj1;
    int i, j, k, state;

    obj1 = dict->lookup("BitsPerCoordinate");
    if (obj1.isInt()) {
        coordBits = obj1.getInt();
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid BitsPerCoordinate in shading dictionary");
        return nullptr;
    }
    if (unlikely(coordBits <= 0)) {
        error(errSyntaxWarning, -1, "Invalid BitsPerCoordinate in shading dictionary");
        return nullptr;
    }
    obj1 = dict->lookup("BitsPerComponent");
    if (obj1.isInt()) {
        compBits = obj1.getInt();
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid BitsPerComponent in shading dictionary");
        return nullptr;
    }
    if (unlikely(compBits <= 0 || compBits > 31)) {
        error(errSyntaxWarning, -1, "Invalid BitsPerComponent in shading dictionary");
        return nullptr;
    }
    flagBits = vertsPerRow = 0; // make gcc happy
    if (typeA == 4) {
        obj1 = dict->lookup("BitsPerFlag");
        if (obj1.isInt()) {
            flagBits = obj1.getInt();
        } else {
            error(errSyntaxWarning, -1, "Missing or invalid BitsPerFlag in shading dictionary");
            return nullptr;
        }
    } else {
        obj1 = dict->lookup("VerticesPerRow");
        if (obj1.isInt()) {
            vertsPerRow = obj1.getInt();
        } else {
            error(errSyntaxWarning, -1, "Missing or invalid VerticesPerRow in shading dictionary");
            return nullptr;
        }
    }
    obj1 = dict->lookup("Decode");
    if (obj1.isArray() && obj1.arrayGetLength() >= 6) {
        bool decodeOk = true;
        xMin = obj1.arrayGet(0).getNum(&decodeOk);
        xMax = obj1.arrayGet(1).getNum(&decodeOk);
        xMul = (xMax - xMin) / (pow(2.0, coordBits) - 1);
        yMin = obj1.arrayGet(2).getNum(&decodeOk);
        yMax = obj1.arrayGet(3).getNum(&decodeOk);
        yMul = (yMax - yMin) / (pow(2.0, coordBits) - 1);
        for (i = 0; 5 + 2 * i < obj1.arrayGetLength() && i < gfxColorMaxComps; ++i) {
            cMin[i] = obj1.arrayGet(4 + 2 * i).getNum(&decodeOk);
            cMax[i] = obj1.arrayGet(5 + 2 * i).getNum(&decodeOk);
            cMul[i] = (cMax[i] - cMin[i]) / (double)((1u << compBits) - 1);
        }
        nComps = i;

        if (!decodeOk) {
            error(errSyntaxWarning, -1, "Missing or invalid Decode array in shading dictionary");
            return nullptr;
        }
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid Decode array in shading dictionary");
        return nullptr;
    }

    obj1 = dict->lookup("Function");
    if (!obj1.isNull()) {
        if (obj1.isArray()) {
            const int nFuncsA = obj1.arrayGetLength();
            if (nFuncsA > gfxColorMaxComps) {
                error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
                return nullptr;
            }
            for (i = 0; i < nFuncsA; ++i) {
                Object obj2 = obj1.arrayGet(i);
                Function *f = Function::parse(&obj2);
                if (!f) {
                    return nullptr;
                }
                funcsA.emplace_back(f);
            }
        } else {
            Function *f = Function::parse(&obj1);
            if (!f) {
                return nullptr;
            }
            funcsA.emplace_back(f);
        }
    }

    nVerticesA = nTrianglesA = 0;
    verticesA = nullptr;
    trianglesA = nullptr;
    vertSize = triSize = 0;
    state = 0;
    flag = 0; // make gcc happy
    bitBuf = new GfxShadingBitBuf(str);
    while (true) {
        if (typeA == 4) {
            if (!bitBuf->getBits(flagBits, &flag)) {
                break;
            }
        }
        if (!bitBuf->getBits(coordBits, &x) || !bitBuf->getBits(coordBits, &y)) {
            break;
        }
        for (i = 0; i < nComps; ++i) {
            if (!bitBuf->getBits(compBits, &c[i])) {
                break;
            }
        }
        if (i < nComps) {
            break;
        }
        if (nVerticesA == vertSize) {
            int oldVertSize = vertSize;
            vertSize = (vertSize == 0) ? 16 : 2 * vertSize;
            verticesA = (GfxGouraudVertex *)greallocn_checkoverflow(verticesA, vertSize, sizeof(GfxGouraudVertex));
            if (unlikely(!verticesA)) {
                error(errSyntaxWarning, -1, "GfxGouraudTriangleShading::parse: vertices size overflow");
                gfree(trianglesA);
                delete bitBuf;
                return nullptr;
            }
            memset(verticesA + oldVertSize, 0, (vertSize - oldVertSize) * sizeof(GfxGouraudVertex));
        }
        verticesA[nVerticesA].x = xMin + xMul * (double)x;
        verticesA[nVerticesA].y = yMin + yMul * (double)y;
        for (i = 0; i < nComps; ++i) {
            verticesA[nVerticesA].color.c[i] = dblToCol(cMin[i] + cMul[i] * (double)c[i]);
        }
        ++nVerticesA;
        bitBuf->flushBits();
        if (typeA == 4) {
            if (state == 0 || state == 1) {
                ++state;
            } else if (state == 2 || flag > 0) {
                if (nTrianglesA == triSize) {
                    triSize = (triSize == 0) ? 16 : 2 * triSize;
                    trianglesA = (int(*)[3])greallocn(trianglesA, triSize * 3, sizeof(int));
                }
                if (state == 2) {
                    trianglesA[nTrianglesA][0] = nVerticesA - 3;
                    trianglesA[nTrianglesA][1] = nVerticesA - 2;
                    trianglesA[nTrianglesA][2] = nVerticesA - 1;
                    ++state;
                } else if (flag == 1) {
                    trianglesA[nTrianglesA][0] = trianglesA[nTrianglesA - 1][1];
                    trianglesA[nTrianglesA][1] = trianglesA[nTrianglesA - 1][2];
                    trianglesA[nTrianglesA][2] = nVerticesA - 1;
                } else { // flag == 2
                    trianglesA[nTrianglesA][0] = trianglesA[nTrianglesA - 1][0];
                    trianglesA[nTrianglesA][1] = trianglesA[nTrianglesA - 1][2];
                    trianglesA[nTrianglesA][2] = nVerticesA - 1;
                }
                ++nTrianglesA;
            } else { // state == 3 && flag == 0
                state = 1;
            }
        }
    }
    delete bitBuf;
    if (typeA == 5 && nVerticesA > 0 && vertsPerRow > 0) {
        nRows = nVerticesA / vertsPerRow;
        nTrianglesA = (nRows - 1) * 2 * (vertsPerRow - 1);
        trianglesA = (int(*)[3])gmallocn_checkoverflow(nTrianglesA * 3, sizeof(int));
        if (unlikely(!trianglesA)) {
            gfree(verticesA);
            return nullptr;
        }
        k = 0;
        for (i = 0; i < nRows - 1; ++i) {
            for (j = 0; j < vertsPerRow - 1; ++j) {
                trianglesA[k][0] = i * vertsPerRow + j;
                trianglesA[k][1] = i * vertsPerRow + j + 1;
                trianglesA[k][2] = (i + 1) * vertsPerRow + j;
                ++k;
                trianglesA[k][0] = i * vertsPerRow + j + 1;
                trianglesA[k][1] = (i + 1) * vertsPerRow + j;
                trianglesA[k][2] = (i + 1) * vertsPerRow + j + 1;
                ++k;
            }
        }
    }

    shading = new GfxGouraudTriangleShading(typeA, verticesA, nVerticesA, trianglesA, nTrianglesA, std::move(funcsA));
    if (!shading->init(res, dict, out, gfxState)) {
        delete shading;
        return nullptr;
    }
    return shading;
}

bool GfxGouraudTriangleShading::init(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    const bool parentInit = GfxShading::init(res, dict, out, state);
    if (!parentInit) {
        return false;
    }

    // funcs needs to be one of the three:
    //  * One function 1-in -> nComps-out
    //  * nComps functions 1-in -> 1-out
    //  * empty
    const int nComps = colorSpace->getNComps();
    const int nFuncs = funcs.size();
    if (nFuncs == 1) {
        if (funcs[0]->getInputSize() != 1) {
            error(errSyntaxWarning, -1, "GfxGouraudTriangleShading: function with input size != 2");
            return false;
        }
        if (funcs[0]->getOutputSize() != nComps) {
            error(errSyntaxWarning, -1, "GfxGouraudTriangleShading: function with wrong output size");
            return false;
        }
    } else if (nFuncs == nComps) {
        for (const std::unique_ptr<Function> &f : funcs) {
            if (f->getInputSize() != 1) {
                error(errSyntaxWarning, -1, "GfxGouraudTriangleShading: function with input size != 2");
                return false;
            }
            if (f->getOutputSize() != 1) {
                error(errSyntaxWarning, -1, "GfxGouraudTriangleShading: function with wrong output size");
                return false;
            }
        }
    } else if (nFuncs != 0) {
        return false;
    }

    return true;
}

GfxShading *GfxGouraudTriangleShading::copy() const
{
    return new GfxGouraudTriangleShading(this);
}

void GfxGouraudTriangleShading::getTriangle(int i, double *x0, double *y0, GfxColor *color0, double *x1, double *y1, GfxColor *color1, double *x2, double *y2, GfxColor *color2)
{
    int v;

    assert(!isParameterized());

    v = triangles[i][0];
    *x0 = vertices[v].x;
    *y0 = vertices[v].y;
    *color0 = vertices[v].color;
    v = triangles[i][1];
    *x1 = vertices[v].x;
    *y1 = vertices[v].y;
    *color1 = vertices[v].color;
    v = triangles[i][2];
    *x2 = vertices[v].x;
    *y2 = vertices[v].y;
    *color2 = vertices[v].color;
}

void GfxGouraudTriangleShading::getParameterizedColor(double t, GfxColor *color) const
{
    double out[gfxColorMaxComps];

    for (unsigned int j = 0; j < funcs.size(); ++j) {
        funcs[j]->transform(&t, &out[j]);
    }
    for (int j = 0; j < gfxColorMaxComps; ++j) {
        color->c[j] = dblToCol(out[j]);
    }
}

void GfxGouraudTriangleShading::getTriangle(int i, double *x0, double *y0, double *color0, double *x1, double *y1, double *color1, double *x2, double *y2, double *color2)
{
    int v;

    assert(isParameterized());

    v = triangles[i][0];
    if (likely(v >= 0 && v < nVertices)) {
        *x0 = vertices[v].x;
        *y0 = vertices[v].y;
        *color0 = colToDbl(vertices[v].color.c[0]);
    }
    v = triangles[i][1];
    if (likely(v >= 0 && v < nVertices)) {
        *x1 = vertices[v].x;
        *y1 = vertices[v].y;
        *color1 = colToDbl(vertices[v].color.c[0]);
    }
    v = triangles[i][2];
    if (likely(v >= 0 && v < nVertices)) {
        *x2 = vertices[v].x;
        *y2 = vertices[v].y;
        *color2 = colToDbl(vertices[v].color.c[0]);
    }
}

//------------------------------------------------------------------------
// GfxPatchMeshShading
//------------------------------------------------------------------------

GfxPatchMeshShading::GfxPatchMeshShading(int typeA, GfxPatch *patchesA, int nPatchesA, std::vector<std::unique_ptr<Function>> &&funcsA) : GfxShading(typeA), funcs(std::move(funcsA))
{
    patches = patchesA;
    nPatches = nPatchesA;
}

GfxPatchMeshShading::GfxPatchMeshShading(const GfxPatchMeshShading *shading) : GfxShading(shading)
{
    nPatches = shading->nPatches;
    patches = (GfxPatch *)gmallocn(nPatches, sizeof(GfxPatch));
    memcpy(patches, shading->patches, nPatches * sizeof(GfxPatch));
    for (const auto &f : shading->funcs) {
        funcs.emplace_back(f->copy());
    }
}

GfxPatchMeshShading::~GfxPatchMeshShading()
{
    gfree(patches);
}

GfxPatchMeshShading *GfxPatchMeshShading::parse(GfxResources *res, int typeA, Dict *dict, Stream *str, OutputDev *out, GfxState *state)
{
    GfxPatchMeshShading *shading;
    std::vector<std::unique_ptr<Function>> funcsA;
    int coordBits, compBits, flagBits;
    double xMin, xMax, yMin, yMax;
    double cMin[gfxColorMaxComps], cMax[gfxColorMaxComps];
    double xMul, yMul;
    double cMul[gfxColorMaxComps];
    GfxPatch *patchesA, *p;
    int nComps, nPatchesA, patchesSize, nPts, nColors;
    unsigned int flag;
    double x[16], y[16];
    unsigned int xi, yi;
    double c[4][gfxColorMaxComps];
    unsigned int ci;
    Object obj1;
    int i, j;

    obj1 = dict->lookup("BitsPerCoordinate");
    if (obj1.isInt()) {
        coordBits = obj1.getInt();
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid BitsPerCoordinate in shading dictionary");
        return nullptr;
    }
    if (unlikely(coordBits <= 0)) {
        error(errSyntaxWarning, -1, "Invalid BitsPerCoordinate in shading dictionary");
        return nullptr;
    }
    obj1 = dict->lookup("BitsPerComponent");
    if (obj1.isInt()) {
        compBits = obj1.getInt();
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid BitsPerComponent in shading dictionary");
        return nullptr;
    }
    if (unlikely(compBits <= 0 || compBits > 31)) {
        error(errSyntaxWarning, -1, "Invalid BitsPerComponent in shading dictionary");
        return nullptr;
    }
    obj1 = dict->lookup("BitsPerFlag");
    if (obj1.isInt()) {
        flagBits = obj1.getInt();
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid BitsPerFlag in shading dictionary");
        return nullptr;
    }
    obj1 = dict->lookup("Decode");
    if (obj1.isArray() && obj1.arrayGetLength() >= 6) {
        bool decodeOk = true;
        xMin = obj1.arrayGet(0).getNum(&decodeOk);
        xMax = obj1.arrayGet(1).getNum(&decodeOk);
        xMul = (xMax - xMin) / (pow(2.0, coordBits) - 1);
        yMin = obj1.arrayGet(2).getNum(&decodeOk);
        yMax = obj1.arrayGet(3).getNum(&decodeOk);
        yMul = (yMax - yMin) / (pow(2.0, coordBits) - 1);
        for (i = 0; 5 + 2 * i < obj1.arrayGetLength() && i < gfxColorMaxComps; ++i) {
            cMin[i] = obj1.arrayGet(4 + 2 * i).getNum(&decodeOk);
            cMax[i] = obj1.arrayGet(5 + 2 * i).getNum(&decodeOk);
            cMul[i] = (cMax[i] - cMin[i]) / (double)((1u << compBits) - 1);
        }
        nComps = i;

        if (!decodeOk) {
            error(errSyntaxWarning, -1, "Missing or invalid Decode array in shading dictionary");
            return nullptr;
        }
    } else {
        error(errSyntaxWarning, -1, "Missing or invalid Decode array in shading dictionary");
        return nullptr;
    }

    obj1 = dict->lookup("Function");
    if (!obj1.isNull()) {
        if (obj1.isArray()) {
            const int nFuncsA = obj1.arrayGetLength();
            if (nFuncsA > gfxColorMaxComps) {
                error(errSyntaxWarning, -1, "Invalid Function array in shading dictionary");
                return nullptr;
            }
            for (i = 0; i < nFuncsA; ++i) {
                Object obj2 = obj1.arrayGet(i);
                Function *f = Function::parse(&obj2);
                if (!f) {
                    return nullptr;
                }
                funcsA.emplace_back(f);
            }
        } else {
            Function *f = Function::parse(&obj1);
            if (!f) {
                return nullptr;
            }
            funcsA.emplace_back(f);
        }
    }

    nPatchesA = 0;
    patchesA = nullptr;
    patchesSize = 0;
    auto bitBuf = std::make_unique<GfxShadingBitBuf>(str);
    while (true) {
        if (!bitBuf->getBits(flagBits, &flag)) {
            break;
        }
        if (typeA == 6) {
            switch (flag) {
            case 0:
                nPts = 12;
                nColors = 4;
                break;
            case 1:
            case 2:
            case 3:
            default:
                nPts = 8;
                nColors = 2;
                break;
            }
        } else {
            switch (flag) {
            case 0:
                nPts = 16;
                nColors = 4;
                break;
            case 1:
            case 2:
            case 3:
            default:
                nPts = 12;
                nColors = 2;
                break;
            }
        }
        for (i = 0; i < nPts; ++i) {
            if (!bitBuf->getBits(coordBits, &xi) || !bitBuf->getBits(coordBits, &yi)) {
                break;
            }
            x[i] = xMin + xMul * (double)xi;
            y[i] = yMin + yMul * (double)yi;
        }
        if (i < nPts) {
            break;
        }
        for (i = 0; i < nColors; ++i) {
            for (j = 0; j < nComps; ++j) {
                if (!bitBuf->getBits(compBits, &ci)) {
                    break;
                }
                c[i][j] = cMin[j] + cMul[j] * (double)ci;
                if (funcsA.empty()) {
                    // ... and colorspace values can also be stored into doubles.
                    // They will be casted later.
                    c[i][j] = dblToCol(c[i][j]);
                }
            }
            if (j < nComps) {
                break;
            }
        }
        if (i < nColors) {
            break;
        }
        if (nPatchesA == patchesSize) {
            int oldPatchesSize = patchesSize;
            patchesSize = (patchesSize == 0) ? 16 : 2 * patchesSize;
            patchesA = (GfxPatch *)greallocn_checkoverflow(patchesA, patchesSize, sizeof(GfxPatch));
            if (unlikely(!patchesA)) {
                return nullptr;
            }
            memset(patchesA + oldPatchesSize, 0, (patchesSize - oldPatchesSize) * sizeof(GfxPatch));
        }
        p = &patchesA[nPatchesA];
        if (typeA == 6) {
            switch (flag) {
            case 0:
                p->x[0][0] = x[0];
                p->y[0][0] = y[0];
                p->x[0][1] = x[1];
                p->y[0][1] = y[1];
                p->x[0][2] = x[2];
                p->y[0][2] = y[2];
                p->x[0][3] = x[3];
                p->y[0][3] = y[3];
                p->x[1][3] = x[4];
                p->y[1][3] = y[4];
                p->x[2][3] = x[5];
                p->y[2][3] = y[5];
                p->x[3][3] = x[6];
                p->y[3][3] = y[6];
                p->x[3][2] = x[7];
                p->y[3][2] = y[7];
                p->x[3][1] = x[8];
                p->y[3][1] = y[8];
                p->x[3][0] = x[9];
                p->y[3][0] = y[9];
                p->x[2][0] = x[10];
                p->y[2][0] = y[10];
                p->x[1][0] = x[11];
                p->y[1][0] = y[11];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = c[0][j];
                    p->color[0][1].c[j] = c[1][j];
                    p->color[1][1].c[j] = c[2][j];
                    p->color[1][0].c[j] = c[3][j];
                }
                break;
            case 1:
                if (nPatchesA == 0) {
                    gfree(patchesA);
                    return nullptr;
                }
                p->x[0][0] = patchesA[nPatchesA - 1].x[0][3];
                p->y[0][0] = patchesA[nPatchesA - 1].y[0][3];
                p->x[0][1] = patchesA[nPatchesA - 1].x[1][3];
                p->y[0][1] = patchesA[nPatchesA - 1].y[1][3];
                p->x[0][2] = patchesA[nPatchesA - 1].x[2][3];
                p->y[0][2] = patchesA[nPatchesA - 1].y[2][3];
                p->x[0][3] = patchesA[nPatchesA - 1].x[3][3];
                p->y[0][3] = patchesA[nPatchesA - 1].y[3][3];
                p->x[1][3] = x[0];
                p->y[1][3] = y[0];
                p->x[2][3] = x[1];
                p->y[2][3] = y[1];
                p->x[3][3] = x[2];
                p->y[3][3] = y[2];
                p->x[3][2] = x[3];
                p->y[3][2] = y[3];
                p->x[3][1] = x[4];
                p->y[3][1] = y[4];
                p->x[3][0] = x[5];
                p->y[3][0] = y[5];
                p->x[2][0] = x[6];
                p->y[2][0] = y[6];
                p->x[1][0] = x[7];
                p->y[1][0] = y[7];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = patchesA[nPatchesA - 1].color[0][1].c[j];
                    p->color[0][1].c[j] = patchesA[nPatchesA - 1].color[1][1].c[j];
                    p->color[1][1].c[j] = c[0][j];
                    p->color[1][0].c[j] = c[1][j];
                }
                break;
            case 2:
                if (nPatchesA == 0) {
                    gfree(patchesA);
                    return nullptr;
                }
                p->x[0][0] = patchesA[nPatchesA - 1].x[3][3];
                p->y[0][0] = patchesA[nPatchesA - 1].y[3][3];
                p->x[0][1] = patchesA[nPatchesA - 1].x[3][2];
                p->y[0][1] = patchesA[nPatchesA - 1].y[3][2];
                p->x[0][2] = patchesA[nPatchesA - 1].x[3][1];
                p->y[0][2] = patchesA[nPatchesA - 1].y[3][1];
                p->x[0][3] = patchesA[nPatchesA - 1].x[3][0];
                p->y[0][3] = patchesA[nPatchesA - 1].y[3][0];
                p->x[1][3] = x[0];
                p->y[1][3] = y[0];
                p->x[2][3] = x[1];
                p->y[2][3] = y[1];
                p->x[3][3] = x[2];
                p->y[3][3] = y[2];
                p->x[3][2] = x[3];
                p->y[3][2] = y[3];
                p->x[3][1] = x[4];
                p->y[3][1] = y[4];
                p->x[3][0] = x[5];
                p->y[3][0] = y[5];
                p->x[2][0] = x[6];
                p->y[2][0] = y[6];
                p->x[1][0] = x[7];
                p->y[1][0] = y[7];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = patchesA[nPatchesA - 1].color[1][1].c[j];
                    p->color[0][1].c[j] = patchesA[nPatchesA - 1].color[1][0].c[j];
                    p->color[1][1].c[j] = c[0][j];
                    p->color[1][0].c[j] = c[1][j];
                }
                break;
            case 3:
                if (nPatchesA == 0) {
                    gfree(patchesA);
                    return nullptr;
                }
                p->x[0][0] = patchesA[nPatchesA - 1].x[3][0];
                p->y[0][0] = patchesA[nPatchesA - 1].y[3][0];
                p->x[0][1] = patchesA[nPatchesA - 1].x[2][0];
                p->y[0][1] = patchesA[nPatchesA - 1].y[2][0];
                p->x[0][2] = patchesA[nPatchesA - 1].x[1][0];
                p->y[0][2] = patchesA[nPatchesA - 1].y[1][0];
                p->x[0][3] = patchesA[nPatchesA - 1].x[0][0];
                p->y[0][3] = patchesA[nPatchesA - 1].y[0][0];
                p->x[1][3] = x[0];
                p->y[1][3] = y[0];
                p->x[2][3] = x[1];
                p->y[2][3] = y[1];
                p->x[3][3] = x[2];
                p->y[3][3] = y[2];
                p->x[3][2] = x[3];
                p->y[3][2] = y[3];
                p->x[3][1] = x[4];
                p->y[3][1] = y[4];
                p->x[3][0] = x[5];
                p->y[3][0] = y[5];
                p->x[2][0] = x[6];
                p->y[2][0] = y[6];
                p->x[1][0] = x[7];
                p->y[1][0] = y[7];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = patchesA[nPatchesA - 1].color[1][0].c[j];
                    p->color[0][1].c[j] = patchesA[nPatchesA - 1].color[0][0].c[j];
                    p->color[1][1].c[j] = c[0][j];
                    p->color[1][0].c[j] = c[1][j];
                }
                break;
            }
        } else {
            switch (flag) {
            case 0:
                p->x[0][0] = x[0];
                p->y[0][0] = y[0];
                p->x[0][1] = x[1];
                p->y[0][1] = y[1];
                p->x[0][2] = x[2];
                p->y[0][2] = y[2];
                p->x[0][3] = x[3];
                p->y[0][3] = y[3];
                p->x[1][3] = x[4];
                p->y[1][3] = y[4];
                p->x[2][3] = x[5];
                p->y[2][3] = y[5];
                p->x[3][3] = x[6];
                p->y[3][3] = y[6];
                p->x[3][2] = x[7];
                p->y[3][2] = y[7];
                p->x[3][1] = x[8];
                p->y[3][1] = y[8];
                p->x[3][0] = x[9];
                p->y[3][0] = y[9];
                p->x[2][0] = x[10];
                p->y[2][0] = y[10];
                p->x[1][0] = x[11];
                p->y[1][0] = y[11];
                p->x[1][1] = x[12];
                p->y[1][1] = y[12];
                p->x[1][2] = x[13];
                p->y[1][2] = y[13];
                p->x[2][2] = x[14];
                p->y[2][2] = y[14];
                p->x[2][1] = x[15];
                p->y[2][1] = y[15];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = c[0][j];
                    p->color[0][1].c[j] = c[1][j];
                    p->color[1][1].c[j] = c[2][j];
                    p->color[1][0].c[j] = c[3][j];
                }
                break;
            case 1:
                if (nPatchesA == 0) {
                    gfree(patchesA);
                    return nullptr;
                }
                p->x[0][0] = patchesA[nPatchesA - 1].x[0][3];
                p->y[0][0] = patchesA[nPatchesA - 1].y[0][3];
                p->x[0][1] = patchesA[nPatchesA - 1].x[1][3];
                p->y[0][1] = patchesA[nPatchesA - 1].y[1][3];
                p->x[0][2] = patchesA[nPatchesA - 1].x[2][3];
                p->y[0][2] = patchesA[nPatchesA - 1].y[2][3];
                p->x[0][3] = patchesA[nPatchesA - 1].x[3][3];
                p->y[0][3] = patchesA[nPatchesA - 1].y[3][3];
                p->x[1][3] = x[0];
                p->y[1][3] = y[0];
                p->x[2][3] = x[1];
                p->y[2][3] = y[1];
                p->x[3][3] = x[2];
                p->y[3][3] = y[2];
                p->x[3][2] = x[3];
                p->y[3][2] = y[3];
                p->x[3][1] = x[4];
                p->y[3][1] = y[4];
                p->x[3][0] = x[5];
                p->y[3][0] = y[5];
                p->x[2][0] = x[6];
                p->y[2][0] = y[6];
                p->x[1][0] = x[7];
                p->y[1][0] = y[7];
                p->x[1][1] = x[8];
                p->y[1][1] = y[8];
                p->x[1][2] = x[9];
                p->y[1][2] = y[9];
                p->x[2][2] = x[10];
                p->y[2][2] = y[10];
                p->x[2][1] = x[11];
                p->y[2][1] = y[11];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = patchesA[nPatchesA - 1].color[0][1].c[j];
                    p->color[0][1].c[j] = patchesA[nPatchesA - 1].color[1][1].c[j];
                    p->color[1][1].c[j] = c[0][j];
                    p->color[1][0].c[j] = c[1][j];
                }
                break;
            case 2:
                if (nPatchesA == 0) {
                    gfree(patchesA);
                    return nullptr;
                }
                p->x[0][0] = patchesA[nPatchesA - 1].x[3][3];
                p->y[0][0] = patchesA[nPatchesA - 1].y[3][3];
                p->x[0][1] = patchesA[nPatchesA - 1].x[3][2];
                p->y[0][1] = patchesA[nPatchesA - 1].y[3][2];
                p->x[0][2] = patchesA[nPatchesA - 1].x[3][1];
                p->y[0][2] = patchesA[nPatchesA - 1].y[3][1];
                p->x[0][3] = patchesA[nPatchesA - 1].x[3][0];
                p->y[0][3] = patchesA[nPatchesA - 1].y[3][0];
                p->x[1][3] = x[0];
                p->y[1][3] = y[0];
                p->x[2][3] = x[1];
                p->y[2][3] = y[1];
                p->x[3][3] = x[2];
                p->y[3][3] = y[2];
                p->x[3][2] = x[3];
                p->y[3][2] = y[3];
                p->x[3][1] = x[4];
                p->y[3][1] = y[4];
                p->x[3][0] = x[5];
                p->y[3][0] = y[5];
                p->x[2][0] = x[6];
                p->y[2][0] = y[6];
                p->x[1][0] = x[7];
                p->y[1][0] = y[7];
                p->x[1][1] = x[8];
                p->y[1][1] = y[8];
                p->x[1][2] = x[9];
                p->y[1][2] = y[9];
                p->x[2][2] = x[10];
                p->y[2][2] = y[10];
                p->x[2][1] = x[11];
                p->y[2][1] = y[11];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = patchesA[nPatchesA - 1].color[1][1].c[j];
                    p->color[0][1].c[j] = patchesA[nPatchesA - 1].color[1][0].c[j];
                    p->color[1][1].c[j] = c[0][j];
                    p->color[1][0].c[j] = c[1][j];
                }
                break;
            case 3:
                if (nPatchesA == 0) {
                    gfree(patchesA);
                    return nullptr;
                }
                p->x[0][0] = patchesA[nPatchesA - 1].x[3][0];
                p->y[0][0] = patchesA[nPatchesA - 1].y[3][0];
                p->x[0][1] = patchesA[nPatchesA - 1].x[2][0];
                p->y[0][1] = patchesA[nPatchesA - 1].y[2][0];
                p->x[0][2] = patchesA[nPatchesA - 1].x[1][0];
                p->y[0][2] = patchesA[nPatchesA - 1].y[1][0];
                p->x[0][3] = patchesA[nPatchesA - 1].x[0][0];
                p->y[0][3] = patchesA[nPatchesA - 1].y[0][0];
                p->x[1][3] = x[0];
                p->y[1][3] = y[0];
                p->x[2][3] = x[1];
                p->y[2][3] = y[1];
                p->x[3][3] = x[2];
                p->y[3][3] = y[2];
                p->x[3][2] = x[3];
                p->y[3][2] = y[3];
                p->x[3][1] = x[4];
                p->y[3][1] = y[4];
                p->x[3][0] = x[5];
                p->y[3][0] = y[5];
                p->x[2][0] = x[6];
                p->y[2][0] = y[6];
                p->x[1][0] = x[7];
                p->y[1][0] = y[7];
                p->x[1][1] = x[8];
                p->y[1][1] = y[8];
                p->x[1][2] = x[9];
                p->y[1][2] = y[9];
                p->x[2][2] = x[10];
                p->y[2][2] = y[10];
                p->x[2][1] = x[11];
                p->y[2][1] = y[11];
                for (j = 0; j < nComps; ++j) {
                    p->color[0][0].c[j] = patchesA[nPatchesA - 1].color[1][0].c[j];
                    p->color[0][1].c[j] = patchesA[nPatchesA - 1].color[0][0].c[j];
                    p->color[1][1].c[j] = c[0][j];
                    p->color[1][0].c[j] = c[1][j];
                }
                break;
            }
        }
        ++nPatchesA;
        bitBuf->flushBits();
    }

    if (typeA == 6) {
        for (i = 0; i < nPatchesA; ++i) {
            p = &patchesA[i];
            p->x[1][1] = (-4 * p->x[0][0] + 6 * (p->x[0][1] + p->x[1][0]) - 2 * (p->x[0][3] + p->x[3][0]) + 3 * (p->x[3][1] + p->x[1][3]) - p->x[3][3]) / 9;
            p->y[1][1] = (-4 * p->y[0][0] + 6 * (p->y[0][1] + p->y[1][0]) - 2 * (p->y[0][3] + p->y[3][0]) + 3 * (p->y[3][1] + p->y[1][3]) - p->y[3][3]) / 9;
            p->x[1][2] = (-4 * p->x[0][3] + 6 * (p->x[0][2] + p->x[1][3]) - 2 * (p->x[0][0] + p->x[3][3]) + 3 * (p->x[3][2] + p->x[1][0]) - p->x[3][0]) / 9;
            p->y[1][2] = (-4 * p->y[0][3] + 6 * (p->y[0][2] + p->y[1][3]) - 2 * (p->y[0][0] + p->y[3][3]) + 3 * (p->y[3][2] + p->y[1][0]) - p->y[3][0]) / 9;
            p->x[2][1] = (-4 * p->x[3][0] + 6 * (p->x[3][1] + p->x[2][0]) - 2 * (p->x[3][3] + p->x[0][0]) + 3 * (p->x[0][1] + p->x[2][3]) - p->x[0][3]) / 9;
            p->y[2][1] = (-4 * p->y[3][0] + 6 * (p->y[3][1] + p->y[2][0]) - 2 * (p->y[3][3] + p->y[0][0]) + 3 * (p->y[0][1] + p->y[2][3]) - p->y[0][3]) / 9;
            p->x[2][2] = (-4 * p->x[3][3] + 6 * (p->x[3][2] + p->x[2][3]) - 2 * (p->x[3][0] + p->x[0][3]) + 3 * (p->x[0][2] + p->x[2][0]) - p->x[0][0]) / 9;
            p->y[2][2] = (-4 * p->y[3][3] + 6 * (p->y[3][2] + p->y[2][3]) - 2 * (p->y[3][0] + p->y[0][3]) + 3 * (p->y[0][2] + p->y[2][0]) - p->y[0][0]) / 9;
        }
    }

    shading = new GfxPatchMeshShading(typeA, patchesA, nPatchesA, std::move(funcsA));
    if (!shading->init(res, dict, out, state)) {
        delete shading;
        return nullptr;
    }
    return shading;
}

bool GfxPatchMeshShading::init(GfxResources *res, Dict *dict, OutputDev *out, GfxState *state)
{
    const bool parentInit = GfxShading::init(res, dict, out, state);
    if (!parentInit) {
        return false;
    }

    // funcs needs to be one of the three:
    //  * One function 1-in -> nComps-out
    //  * nComps functions 1-in -> 1-out
    //  * empty
    const int nComps = colorSpace->getNComps();
    const int nFuncs = funcs.size();
    if (nFuncs == 1) {
        if (funcs[0]->getInputSize() != 1) {
            error(errSyntaxWarning, -1, "GfxPatchMeshShading: function with input size != 2");
            return false;
        }
        if (funcs[0]->getOutputSize() != nComps) {
            error(errSyntaxWarning, -1, "GfxPatchMeshShading: function with wrong output size");
            return false;
        }
    } else if (nFuncs == nComps) {
        for (const std::unique_ptr<Function> &f : funcs) {
            if (f->getInputSize() != 1) {
                error(errSyntaxWarning, -1, "GfxPatchMeshShading: function with input size != 2");
                return false;
            }
            if (f->getOutputSize() != 1) {
                error(errSyntaxWarning, -1, "GfxPatchMeshShading: function with wrong output size");
                return false;
            }
        }
    } else if (nFuncs != 0) {
        return false;
    }

    return true;
}

void GfxPatchMeshShading::getParameterizedColor(double t, GfxColor *color) const
{
    double out[gfxColorMaxComps] = {};

    for (unsigned int j = 0; j < funcs.size(); ++j) {
        funcs[j]->transform(&t, &out[j]);
    }
    for (int j = 0; j < gfxColorMaxComps; ++j) {
        color->c[j] = dblToCol(out[j]);
    }
}

GfxShading *GfxPatchMeshShading::copy() const
{
    return new GfxPatchMeshShading(this);
}

//------------------------------------------------------------------------
// GfxImageColorMap
//------------------------------------------------------------------------

GfxImageColorMap::GfxImageColorMap(int bitsA, Object *decode, GfxColorSpace *colorSpaceA)
{
    GfxIndexedColorSpace *indexedCS;
    GfxSeparationColorSpace *sepCS;
    int maxPixel, indexHigh;
    unsigned char *indexedLookup;
    const Function *sepFunc;
    double x[gfxColorMaxComps];
    double y[gfxColorMaxComps] = {};
    int i, j, k;
    double mapped;
    bool useByteLookup;

    ok = true;
    useMatte = false;

    colorSpace = colorSpaceA;

    // initialize
    for (k = 0; k < gfxColorMaxComps; ++k) {
        lookup[k] = nullptr;
        lookup2[k] = nullptr;
    }
    byte_lookup = nullptr;

    // bits per component and color space
    if (unlikely(bitsA <= 0 || bitsA > 30)) {
        goto err1;
    }

    bits = bitsA;
    maxPixel = (1 << bits) - 1;

    // this is a hack to support 16 bits images, everywhere
    // we assume a component fits in 8 bits, with this hack
    // we treat 16 bit images as 8 bit ones until it's fixed correctly.
    // The hack has another part on ImageStream::getLine
    if (maxPixel > 255) {
        maxPixel = 255;
    }

    // get decode map
    if (decode->isNull()) {
        nComps = colorSpace->getNComps();
        colorSpace->getDefaultRanges(decodeLow, decodeRange, maxPixel);
    } else if (decode->isArray()) {
        nComps = decode->arrayGetLength() / 2;
        if (nComps < colorSpace->getNComps()) {
            goto err1;
        }
        if (nComps > colorSpace->getNComps()) {
            error(errSyntaxWarning, -1, "Too many elements in Decode array");
            nComps = colorSpace->getNComps();
        }
        for (i = 0; i < nComps; ++i) {
            Object obj = decode->arrayGet(2 * i);
            if (!obj.isNum()) {
                goto err1;
            }
            decodeLow[i] = obj.getNum();
            obj = decode->arrayGet(2 * i + 1);
            if (!obj.isNum()) {
                goto err1;
            }
            decodeRange[i] = obj.getNum() - decodeLow[i];
        }
    } else {
        goto err1;
    }

    // Construct a lookup table -- this stores pre-computed decoded
    // values for each component, i.e., the result of applying the
    // decode mapping to each possible image pixel component value.
    for (k = 0; k < nComps; ++k) {
        lookup[k] = (GfxColorComp *)gmallocn(maxPixel + 1, sizeof(GfxColorComp));
        for (i = 0; i <= maxPixel; ++i) {
            lookup[k][i] = dblToCol(decodeLow[k] + (i * decodeRange[k]) / maxPixel);
        }
    }

    // Optimization: for Indexed and Separation color spaces (which have
    // only one component), we pre-compute a second lookup table with
    // color values
    colorSpace2 = nullptr;
    nComps2 = 0;
    useByteLookup = false;
    switch (colorSpace->getMode()) {
    case csIndexed:
        // Note that indexHigh may not be the same as maxPixel --
        // Distiller will remove unused palette entries, resulting in
        // indexHigh < maxPixel.
        indexedCS = (GfxIndexedColorSpace *)colorSpace;
        colorSpace2 = indexedCS->getBase();
        indexHigh = indexedCS->getIndexHigh();
        nComps2 = colorSpace2->getNComps();
        indexedLookup = indexedCS->getLookup();
        colorSpace2->getDefaultRanges(x, y, indexHigh);
        if (colorSpace2->useGetGrayLine() || colorSpace2->useGetRGBLine() || colorSpace2->useGetCMYKLine() || colorSpace2->useGetDeviceNLine()) {
            byte_lookup = (unsigned char *)gmallocn((maxPixel + 1), nComps2);
            useByteLookup = true;
        }
        for (k = 0; k < nComps2; ++k) {
            lookup2[k] = (GfxColorComp *)gmallocn(maxPixel + 1, sizeof(GfxColorComp));
            for (i = 0; i <= maxPixel; ++i) {
                j = (int)(decodeLow[0] + (i * decodeRange[0]) / maxPixel + 0.5);
                if (j < 0) {
                    j = 0;
                } else if (j > indexHigh) {
                    j = indexHigh;
                }

                mapped = x[k] + (indexedLookup[j * nComps2 + k] / 255.0) * y[k];
                lookup2[k][i] = dblToCol(mapped);
                if (useByteLookup) {
                    byte_lookup[i * nComps2 + k] = (unsigned char)(mapped * 255);
                }
            }
        }
        break;
    case csSeparation:
        sepCS = (GfxSeparationColorSpace *)colorSpace;
        colorSpace2 = sepCS->getAlt();
        nComps2 = colorSpace2->getNComps();
        sepFunc = sepCS->getFunc();
        if (colorSpace2->useGetGrayLine() || colorSpace2->useGetRGBLine() || colorSpace2->useGetCMYKLine() || colorSpace2->useGetDeviceNLine()) {
            byte_lookup = (unsigned char *)gmallocn((maxPixel + 1), nComps2);
            useByteLookup = true;
        }
        for (k = 0; k < nComps2; ++k) {
            lookup2[k] = (GfxColorComp *)gmallocn(maxPixel + 1, sizeof(GfxColorComp));
            for (i = 0; i <= maxPixel; ++i) {
                x[0] = decodeLow[0] + (i * decodeRange[0]) / maxPixel;
                sepFunc->transform(x, y);
                lookup2[k][i] = dblToCol(y[k]);
                if (useByteLookup) {
                    byte_lookup[i * nComps2 + k] = (unsigned char)(y[k] * 255);
                }
            }
        }
        break;
    default:
        if ((!decode->isNull() || maxPixel != 255) && (colorSpace->useGetGrayLine() || (colorSpace->useGetRGBLine() && !decode->isNull()) || colorSpace->useGetCMYKLine() || colorSpace->useGetDeviceNLine())) {
            byte_lookup = (unsigned char *)gmallocn((maxPixel + 1), nComps);
            useByteLookup = true;
        }
        for (k = 0; k < nComps; ++k) {
            lookup2[k] = (GfxColorComp *)gmallocn(maxPixel + 1, sizeof(GfxColorComp));
            for (i = 0; i <= maxPixel; ++i) {
                mapped = decodeLow[k] + (i * decodeRange[k]) / maxPixel;
                lookup2[k][i] = dblToCol(mapped);
                if (useByteLookup) {
                    int byte;

                    byte = (int)(mapped * 255.0 + 0.5);
                    if (byte < 0) {
                        byte = 0;
                    } else if (byte > 255) {
                        byte = 255;
                    }
                    byte_lookup[i * nComps + k] = byte;
                }
            }
        }
    }

    return;

err1:
    ok = false;
}

GfxImageColorMap::GfxImageColorMap(const GfxImageColorMap *colorMap)
{
    int n, i, k;

    colorSpace = colorMap->colorSpace->copy();
    bits = colorMap->bits;
    nComps = colorMap->nComps;
    nComps2 = colorMap->nComps2;
    useMatte = colorMap->useMatte;
    matteColor = colorMap->matteColor;
    colorSpace2 = nullptr;
    for (k = 0; k < gfxColorMaxComps; ++k) {
        lookup[k] = nullptr;
        lookup2[k] = nullptr;
    }
    byte_lookup = nullptr;
    n = 1 << bits;
    for (k = 0; k < nComps; ++k) {
        lookup[k] = (GfxColorComp *)gmallocn(n, sizeof(GfxColorComp));
        memcpy(lookup[k], colorMap->lookup[k], n * sizeof(GfxColorComp));
    }
    if (colorSpace->getMode() == csIndexed) {
        colorSpace2 = ((GfxIndexedColorSpace *)colorSpace)->getBase();
        for (k = 0; k < nComps2; ++k) {
            lookup2[k] = (GfxColorComp *)gmallocn(n, sizeof(GfxColorComp));
            memcpy(lookup2[k], colorMap->lookup2[k], n * sizeof(GfxColorComp));
        }
    } else if (colorSpace->getMode() == csSeparation) {
        colorSpace2 = ((GfxSeparationColorSpace *)colorSpace)->getAlt();
        for (k = 0; k < nComps2; ++k) {
            lookup2[k] = (GfxColorComp *)gmallocn(n, sizeof(GfxColorComp));
            memcpy(lookup2[k], colorMap->lookup2[k], n * sizeof(GfxColorComp));
        }
    } else {
        for (k = 0; k < nComps; ++k) {
            lookup2[k] = (GfxColorComp *)gmallocn(n, sizeof(GfxColorComp));
            memcpy(lookup2[k], colorMap->lookup2[k], n * sizeof(GfxColorComp));
        }
    }
    if (colorMap->byte_lookup) {
        int nc = colorSpace2 ? nComps2 : nComps;

        byte_lookup = (unsigned char *)gmallocn(n, nc);
        memcpy(byte_lookup, colorMap->byte_lookup, n * nc);
    }
    for (i = 0; i < nComps; ++i) {
        decodeLow[i] = colorMap->decodeLow[i];
        decodeRange[i] = colorMap->decodeRange[i];
    }
    ok = true;
}

GfxImageColorMap::~GfxImageColorMap()
{
    int i;

    delete colorSpace;
    for (i = 0; i < gfxColorMaxComps; ++i) {
        gfree(lookup[i]);
        gfree(lookup2[i]);
    }
    gfree(byte_lookup);
}

void GfxImageColorMap::getGray(const unsigned char *x, GfxGray *gray)
{
    GfxColor color;
    int i;

    if (colorSpace2) {
        for (i = 0; i < nComps2; ++i) {
            color.c[i] = lookup2[i][x[0]];
        }
        colorSpace2->getGray(&color, gray);
    } else {
        for (i = 0; i < nComps; ++i) {
            color.c[i] = lookup2[i][x[i]];
        }
        colorSpace->getGray(&color, gray);
    }
}

void GfxImageColorMap::getRGB(const unsigned char *x, GfxRGB *rgb)
{
    GfxColor color;
    int i;

    if (colorSpace2) {
        for (i = 0; i < nComps2; ++i) {
            color.c[i] = lookup2[i][x[0]];
        }
        colorSpace2->getRGB(&color, rgb);
    } else {
        for (i = 0; i < nComps; ++i) {
            color.c[i] = lookup2[i][x[i]];
        }
        colorSpace->getRGB(&color, rgb);
    }
}

void GfxImageColorMap::getGrayLine(unsigned char *in, unsigned char *out, int length)
{
    int i, j;
    unsigned char *inp, *tmp_line;

    if ((colorSpace2 && !colorSpace2->useGetGrayLine()) || (!colorSpace2 && !colorSpace->useGetGrayLine())) {
        GfxGray gray;

        inp = in;
        for (i = 0; i < length; i++) {
            getGray(inp, &gray);
            out[i] = colToByte(gray);
            inp += nComps;
        }
        return;
    }

    switch (colorSpace->getMode()) {
    case csIndexed:
    case csSeparation:
        tmp_line = (unsigned char *)gmallocn(length, nComps2);
        for (i = 0; i < length; i++) {
            for (j = 0; j < nComps2; j++) {
                unsigned char c = in[i];
                if (byte_lookup) {
                    c = byte_lookup[c * nComps2 + j];
                }
                tmp_line[i * nComps2 + j] = c;
            }
        }
        colorSpace2->getGrayLine(tmp_line, out, length);
        gfree(tmp_line);
        break;

    default:
        if (byte_lookup) {
            inp = in;
            for (j = 0; j < length; j++) {
                for (i = 0; i < nComps; i++) {
                    *inp = byte_lookup[*inp * nComps + i];
                    inp++;
                }
            }
        }
        colorSpace->getGrayLine(in, out, length);
        break;
    }
}

void GfxImageColorMap::getRGBLine(unsigned char *in, unsigned int *out, int length)
{
    int i, j;
    unsigned char *inp, *tmp_line;

    if (!useRGBLine()) {
        GfxRGB rgb;

        inp = in;
        for (i = 0; i < length; i++) {
            getRGB(inp, &rgb);
            out[i] = ((int)colToByte(rgb.r) << 16) | ((int)colToByte(rgb.g) << 8) | ((int)colToByte(rgb.b) << 0);
            inp += nComps;
        }
        return;
    }

    switch (colorSpace->getMode()) {
    case csIndexed:
    case csSeparation:
        tmp_line = (unsigned char *)gmallocn(length, nComps2);
        for (i = 0; i < length; i++) {
            for (j = 0; j < nComps2; j++) {
                unsigned char c = in[i];
                if (byte_lookup) {
                    c = byte_lookup[c * nComps2 + j];
                }
                tmp_line[i * nComps2 + j] = c;
            }
        }
        colorSpace2->getRGBLine(tmp_line, out, length);
        gfree(tmp_line);
        break;

    default:
        if (byte_lookup) {
            inp = in;
            for (j = 0; j < length; j++) {
                for (i = 0; i < nComps; i++) {
                    *inp = byte_lookup[*inp * nComps + i];
                    inp++;
                }
            }
        }
        colorSpace->getRGBLine(in, out, length);
        break;
    }
}

void GfxImageColorMap::getRGBLine(unsigned char *in, unsigned char *out, int length)
{
    int i, j;
    unsigned char *inp, *tmp_line;

    if (!useRGBLine()) {
        GfxRGB rgb;

        inp = in;
        for (i = 0; i < length; i++) {
            getRGB(inp, &rgb);
            *out++ = colToByte(rgb.r);
            *out++ = colToByte(rgb.g);
            *out++ = colToByte(rgb.b);
            inp += nComps;
        }
        return;
    }

    switch (colorSpace->getMode()) {
    case csIndexed:
    case csSeparation:
        tmp_line = (unsigned char *)gmallocn(length, nComps2);
        for (i = 0; i < length; i++) {
            for (j = 0; j < nComps2; j++) {
                unsigned char c = in[i];
                if (byte_lookup) {
                    c = byte_lookup[c * nComps2 + j];
                }
                tmp_line[i * nComps2 + j] = c;
            }
        }
        colorSpace2->getRGBLine(tmp_line, out, length);
        gfree(tmp_line);
        break;

    default:
        if (byte_lookup) {
            inp = in;
            for (j = 0; j < length; j++) {
                for (i = 0; i < nComps; i++) {
                    *inp = byte_lookup[*inp * nComps + i];
                    inp++;
                }
            }
        }
        colorSpace->getRGBLine(in, out, length);
        break;
    }
}

void GfxImageColorMap::getRGBXLine(unsigned char *in, unsigned char *out, int length)
{
    int i, j;
    unsigned char *inp, *tmp_line;

    if (!useRGBLine()) {
        GfxRGB rgb;

        inp = in;
        for (i = 0; i < length; i++) {
            getRGB(inp, &rgb);
            *out++ = colToByte(rgb.r);
            *out++ = colToByte(rgb.g);
            *out++ = colToByte(rgb.b);
            *out++ = 255;
            inp += nComps;
        }
        return;
    }

    switch (colorSpace->getMode()) {
    case csIndexed:
    case csSeparation:
        tmp_line = (unsigned char *)gmallocn(length, nComps2);
        for (i = 0; i < length; i++) {
            for (j = 0; j < nComps2; j++) {
                unsigned char c = in[i];
                if (byte_lookup) {
                    c = byte_lookup[c * nComps2 + j];
                }
                tmp_line[i * nComps2 + j] = c;
            }
        }
        colorSpace2->getRGBXLine(tmp_line, out, length);
        gfree(tmp_line);
        break;

    default:
        if (byte_lookup) {
            inp = in;
            for (j = 0; j < length; j++) {
                for (i = 0; i < nComps; i++) {
                    *inp = byte_lookup[*inp * nComps + i];
                    inp++;
                }
            }
        }
        colorSpace->getRGBXLine(in, out, length);
        break;
    }
}

void GfxImageColorMap::getCMYKLine(unsigned char *in, unsigned char *out, int length)
{
    int i, j;
    unsigned char *inp, *tmp_line;

    if (!useCMYKLine()) {
        GfxCMYK cmyk;

        inp = in;
        for (i = 0; i < length; i++) {
            getCMYK(inp, &cmyk);
            *out++ = colToByte(cmyk.c);
            *out++ = colToByte(cmyk.m);
            *out++ = colToByte(cmyk.y);
            *out++ = colToByte(cmyk.k);
            inp += nComps;
        }
        return;
    }

    switch (colorSpace->getMode()) {
    case csIndexed:
    case csSeparation:
        tmp_line = (unsigned char *)gmallocn(length, nComps2);
        for (i = 0; i < length; i++) {
            for (j = 0; j < nComps2; j++) {
                unsigned char c = in[i];
                if (byte_lookup) {
                    c = byte_lookup[c * nComps2 + j];
                }
                tmp_line[i * nComps2 + j] = c;
            }
        }
        colorSpace2->getCMYKLine(tmp_line, out, length);
        gfree(tmp_line);
        break;

    default:
        if (byte_lookup) {
            inp = in;
            for (j = 0; j < length; j++) {
                for (i = 0; i < nComps; i++) {
                    *inp = byte_lookup[*inp * nComps + i];
                    inp++;
                }
            }
        }
        colorSpace->getCMYKLine(in, out, length);
        break;
    }
}

void GfxImageColorMap::getDeviceNLine(unsigned char *in, unsigned char *out, int length)
{
    unsigned char *inp, *tmp_line;

    if (!useDeviceNLine()) {
        GfxColor deviceN;

        inp = in;
        for (int i = 0; i < length; i++) {
            getDeviceN(inp, &deviceN);
            for (int j = 0; j < SPOT_NCOMPS + 4; j++) {
                *out++ = deviceN.c[j];
            }
            inp += nComps;
        }
        return;
    }

    switch (colorSpace->getMode()) {
    case csIndexed:
    case csSeparation:
        tmp_line = (unsigned char *)gmallocn(length, nComps2);
        for (int i = 0; i < length; i++) {
            for (int j = 0; j < nComps2; j++) {
                unsigned char c = in[i];
                if (byte_lookup) {
                    c = byte_lookup[c * nComps2 + j];
                }
                tmp_line[i * nComps2 + j] = c;
            }
        }
        colorSpace2->getDeviceNLine(tmp_line, out, length);
        gfree(tmp_line);
        break;

    default:
        if (byte_lookup) {
            inp = in;
            for (int j = 0; j < length; j++) {
                for (int i = 0; i < nComps; i++) {
                    *inp = byte_lookup[*inp * nComps + i];
                    inp++;
                }
            }
        }
        colorSpace->getDeviceNLine(in, out, length);
        break;
    }
}

void GfxImageColorMap::getCMYK(const unsigned char *x, GfxCMYK *cmyk)
{
    GfxColor color;
    int i;

    if (colorSpace2) {
        for (i = 0; i < nComps2; ++i) {
            color.c[i] = lookup2[i][x[0]];
        }
        colorSpace2->getCMYK(&color, cmyk);
    } else {
        for (i = 0; i < nComps; ++i) {
            color.c[i] = lookup[i][x[i]];
        }
        colorSpace->getCMYK(&color, cmyk);
    }
}

void GfxImageColorMap::getDeviceN(const unsigned char *x, GfxColor *deviceN)
{
    GfxColor color;
    int i;

    if (colorSpace2 && (colorSpace->getMapping() == nullptr || colorSpace->getMapping()[0] == -1)) {
        for (i = 0; i < nComps2; ++i) {
            color.c[i] = lookup2[i][x[0]];
        }
        colorSpace2->getDeviceN(&color, deviceN);
    } else {
        for (i = 0; i < nComps; ++i) {
            color.c[i] = lookup[i][x[i]];
        }
        colorSpace->getDeviceN(&color, deviceN);
    }
}

void GfxImageColorMap::getColor(const unsigned char *x, GfxColor *color)
{
    int maxPixel, i;

    maxPixel = (1 << bits) - 1;
    for (i = 0; i < nComps; ++i) {
        color->c[i] = dblToCol(decodeLow[i] + (x[i] * decodeRange[i]) / maxPixel);
    }
}

//------------------------------------------------------------------------
// GfxSubpath and GfxPath
//------------------------------------------------------------------------

GfxSubpath::GfxSubpath(double x1, double y1)
{
    size = 16;
    x = (double *)gmallocn(size, sizeof(double));
    y = (double *)gmallocn(size, sizeof(double));
    curve = (bool *)gmallocn(size, sizeof(bool));
    n = 1;
    x[0] = x1;
    y[0] = y1;
    curve[0] = false;
    closed = false;
}

GfxSubpath::~GfxSubpath()
{
    gfree(x);
    gfree(y);
    gfree(curve);
}

// Used for copy().
GfxSubpath::GfxSubpath(const GfxSubpath *subpath)
{
    size = subpath->size;
    n = subpath->n;
    x = (double *)gmallocn(size, sizeof(double));
    y = (double *)gmallocn(size, sizeof(double));
    curve = (bool *)gmallocn(size, sizeof(bool));
    memcpy(x, subpath->x, n * sizeof(double));
    memcpy(y, subpath->y, n * sizeof(double));
    memcpy(curve, subpath->curve, n * sizeof(bool));
    closed = subpath->closed;
}

void GfxSubpath::lineTo(double x1, double y1)
{
    if (n >= size) {
        size *= 2;
        x = (double *)greallocn(x, size, sizeof(double));
        y = (double *)greallocn(y, size, sizeof(double));
        curve = (bool *)greallocn(curve, size, sizeof(bool));
    }
    x[n] = x1;
    y[n] = y1;
    curve[n] = false;
    ++n;
}

void GfxSubpath::curveTo(double x1, double y1, double x2, double y2, double x3, double y3)
{
    if (n + 3 > size) {
        size *= 2;
        x = (double *)greallocn(x, size, sizeof(double));
        y = (double *)greallocn(y, size, sizeof(double));
        curve = (bool *)greallocn(curve, size, sizeof(bool));
    }
    x[n] = x1;
    y[n] = y1;
    x[n + 1] = x2;
    y[n + 1] = y2;
    x[n + 2] = x3;
    y[n + 2] = y3;
    curve[n] = curve[n + 1] = true;
    curve[n + 2] = false;
    n += 3;
}

void GfxSubpath::close()
{
    if (x[n - 1] != x[0] || y[n - 1] != y[0]) {
        lineTo(x[0], y[0]);
    }
    closed = true;
}

void GfxSubpath::offset(double dx, double dy)
{
    int i;

    for (i = 0; i < n; ++i) {
        x[i] += dx;
        y[i] += dy;
    }
}

GfxPath::GfxPath()
{
    justMoved = false;
    size = 16;
    n = 0;
    firstX = firstY = 0;
    subpaths = (GfxSubpath **)gmallocn(size, sizeof(GfxSubpath *));
}

GfxPath::~GfxPath()
{
    int i;

    for (i = 0; i < n; ++i) {
        delete subpaths[i];
    }
    gfree(subpaths);
}

// Used for copy().
GfxPath::GfxPath(bool justMoved1, double firstX1, double firstY1, GfxSubpath **subpaths1, int n1, int size1)
{
    int i;

    justMoved = justMoved1;
    firstX = firstX1;
    firstY = firstY1;
    size = size1;
    n = n1;
    subpaths = (GfxSubpath **)gmallocn(size, sizeof(GfxSubpath *));
    for (i = 0; i < n; ++i) {
        subpaths[i] = subpaths1[i]->copy();
    }
}

void GfxPath::moveTo(double x, double y)
{
    justMoved = true;
    firstX = x;
    firstY = y;
}

void GfxPath::lineTo(double x, double y)
{
    if (justMoved || (n > 0 && subpaths[n - 1]->isClosed())) {
        if (n >= size) {
            size *= 2;
            subpaths = (GfxSubpath **)greallocn(subpaths, size, sizeof(GfxSubpath *));
        }
        if (justMoved) {
            subpaths[n] = new GfxSubpath(firstX, firstY);
        } else {
            subpaths[n] = new GfxSubpath(subpaths[n - 1]->getLastX(), subpaths[n - 1]->getLastY());
        }
        ++n;
        justMoved = false;
    }
    subpaths[n - 1]->lineTo(x, y);
}

void GfxPath::curveTo(double x1, double y1, double x2, double y2, double x3, double y3)
{
    if (justMoved || (n > 0 && subpaths[n - 1]->isClosed())) {
        if (n >= size) {
            size *= 2;
            subpaths = (GfxSubpath **)greallocn(subpaths, size, sizeof(GfxSubpath *));
        }
        if (justMoved) {
            subpaths[n] = new GfxSubpath(firstX, firstY);
        } else {
            subpaths[n] = new GfxSubpath(subpaths[n - 1]->getLastX(), subpaths[n - 1]->getLastY());
        }
        ++n;
        justMoved = false;
    }
    subpaths[n - 1]->curveTo(x1, y1, x2, y2, x3, y3);
}

void GfxPath::close()
{
    // this is necessary to handle the pathological case of
    // moveto/closepath/clip, which defines an empty clipping region
    if (justMoved) {
        if (n >= size) {
            size *= 2;
            subpaths = (GfxSubpath **)greallocn(subpaths, size, sizeof(GfxSubpath *));
        }
        subpaths[n] = new GfxSubpath(firstX, firstY);
        ++n;
        justMoved = false;
    }
    subpaths[n - 1]->close();
}

void GfxPath::append(GfxPath *path)
{
    int i;

    if (n + path->n > size) {
        size = n + path->n;
        subpaths = (GfxSubpath **)greallocn(subpaths, size, sizeof(GfxSubpath *));
    }
    for (i = 0; i < path->n; ++i) {
        subpaths[n++] = path->subpaths[i]->copy();
    }
    justMoved = false;
}

void GfxPath::offset(double dx, double dy)
{
    int i;

    for (i = 0; i < n; ++i) {
        subpaths[i]->offset(dx, dy);
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
GfxState::ReusablePathIterator::ReusablePathIterator(GfxPath *pathA) : path(pathA), subPathOff(0), coordOff(0), numCoords(0), curSubPath(nullptr)
{
    if (path->getNumSubpaths()) {
        curSubPath = path->getSubpath(subPathOff);
        numCoords = curSubPath->getNumPoints();
    }
}

bool GfxState::ReusablePathIterator::isEnd() const
{
    return coordOff >= numCoords;
}

void GfxState::ReusablePathIterator::next()
{
    ++coordOff;
    if (coordOff == numCoords) {
        ++subPathOff;
        if (subPathOff < path->getNumSubpaths()) {
            coordOff = 0;
            curSubPath = path->getSubpath(subPathOff);
            numCoords = curSubPath->getNumPoints();
        }
    }
}

void GfxState::ReusablePathIterator::setCoord(double x, double y)
{
    curSubPath->setX(coordOff, x);
    curSubPath->setY(coordOff, y);
}

void GfxState::ReusablePathIterator::reset()
{
    coordOff = 0;
    subPathOff = 0;
    curSubPath = path->getSubpath(0);
    numCoords = curSubPath->getNumPoints();
}

GfxState::GfxState(double hDPIA, double vDPIA, const PDFRectangle *pageBox, int rotateA, bool upsideDown)
{
    double kx, ky;

    hDPI = hDPIA;
    vDPI = vDPIA;
    rotate = rotateA;
    px1 = pageBox->x1;
    py1 = pageBox->y1;
    px2 = pageBox->x2;
    py2 = pageBox->y2;
    kx = hDPI / 72.0;
    ky = vDPI / 72.0;
    if (rotate == 90) {
        ctm[0] = 0;
        ctm[1] = upsideDown ? ky : -ky;
        ctm[2] = kx;
        ctm[3] = 0;
        ctm[4] = -kx * py1;
        ctm[5] = ky * (upsideDown ? -px1 : px2);
        pageWidth = kx * (py2 - py1);
        pageHeight = ky * (px2 - px1);
    } else if (rotate == 180) {
        ctm[0] = -kx;
        ctm[1] = 0;
        ctm[2] = 0;
        ctm[3] = upsideDown ? ky : -ky;
        ctm[4] = kx * px2;
        ctm[5] = ky * (upsideDown ? -py1 : py2);
        pageWidth = kx * (px2 - px1);
        pageHeight = ky * (py2 - py1);
    } else if (rotate == 270) {
        ctm[0] = 0;
        ctm[1] = upsideDown ? -ky : ky;
        ctm[2] = -kx;
        ctm[3] = 0;
        ctm[4] = kx * py2;
        ctm[5] = ky * (upsideDown ? px2 : -px1);
        pageWidth = kx * (py2 - py1);
        pageHeight = ky * (px2 - px1);
    } else {
        ctm[0] = kx;
        ctm[1] = 0;
        ctm[2] = 0;
        ctm[3] = upsideDown ? -ky : ky;
        ctm[4] = -kx * px1;
        ctm[5] = ky * (upsideDown ? py2 : -py1);
        pageWidth = kx * (px2 - px1);
        pageHeight = ky * (py2 - py1);
    }

    fillColorSpace = new GfxDeviceGrayColorSpace();
    strokeColorSpace = new GfxDeviceGrayColorSpace();
    fillColor.c[0] = 0;
    strokeColor.c[0] = 0;
    fillPattern = nullptr;
    strokePattern = nullptr;
    blendMode = gfxBlendNormal;
    fillOpacity = 1;
    strokeOpacity = 1;
    fillOverprint = false;
    strokeOverprint = false;
    overprintMode = 0;
    transfer[0] = transfer[1] = transfer[2] = transfer[3] = nullptr;

    lineWidth = 1;
    lineDashStart = 0;
    flatness = 1;
    lineJoin = 0;
    lineCap = 0;
    miterLimit = 10;
    strokeAdjust = false;
    alphaIsShape = false;
    textKnockout = false;

    font = nullptr;
    fontSize = 0;
    textMat[0] = 1;
    textMat[1] = 0;
    textMat[2] = 0;
    textMat[3] = 1;
    textMat[4] = 0;
    textMat[5] = 0;
    charSpace = 0;
    wordSpace = 0;
    horizScaling = 1;
    leading = 0;
    rise = 0;
    render = 0;

    path = new GfxPath();
    curX = curY = 0;
    lineX = lineY = 0;

    clipXMin = 0;
    clipYMin = 0;
    clipXMax = pageWidth;
    clipYMax = pageHeight;

    renderingIntent[0] = 0;

    saved = nullptr;

    defaultGrayColorSpace = nullptr;
    defaultRGBColorSpace = nullptr;
    defaultCMYKColorSpace = nullptr;
#ifdef USE_CMS
    XYZ2DisplayTransformRelCol = nullptr;
    XYZ2DisplayTransformAbsCol = nullptr;
    XYZ2DisplayTransformSat = nullptr;
    XYZ2DisplayTransformPerc = nullptr;
    localDisplayProfile = nullptr;

    if (!sRGBProfile) {
        // This is probably the one of the first invocations of lcms2, so we set the error handler
        cmsSetLogErrorHandler(CMSError);

        sRGBProfile = make_GfxLCMSProfilePtr(cmsCreate_sRGBProfile());
    }

    if (!XYZProfile) {
        XYZProfile = make_GfxLCMSProfilePtr(cmsCreateXYZProfile());
    }
#endif
}

GfxState::~GfxState()
{
    int i;

    if (fillColorSpace) {
        delete fillColorSpace;
    }
    if (strokeColorSpace) {
        delete strokeColorSpace;
    }
    if (fillPattern) {
        delete fillPattern;
    }
    if (strokePattern) {
        delete strokePattern;
    }
    for (i = 0; i < 4; ++i) {
        if (transfer[i]) {
            delete transfer[i];
        }
    }
    if (path) {
        // this gets set to NULL by restore()
        delete path;
    }

    delete defaultGrayColorSpace;
    delete defaultRGBColorSpace;
    delete defaultCMYKColorSpace;
}

// Used for copy();
GfxState::GfxState(const GfxState *state, bool copyPath)
{
    int i;

    hDPI = state->hDPI;
    vDPI = state->vDPI;
    memcpy(ctm, state->ctm, sizeof(ctm));
    px1 = state->px1;
    py1 = state->py1;
    px2 = state->px2;
    py2 = state->py2;
    pageWidth = state->pageWidth;
    pageHeight = state->pageHeight;
    rotate = state->rotate;

    fillColorSpace = state->fillColorSpace;
    if (fillColorSpace) {
        fillColorSpace = state->fillColorSpace->copy();
    }
    strokeColorSpace = state->strokeColorSpace;
    if (strokeColorSpace) {
        strokeColorSpace = state->strokeColorSpace->copy();
    }
    fillColor = state->fillColor;
    strokeColor = state->strokeColor;

    fillPattern = state->fillPattern;
    if (fillPattern) {
        fillPattern = state->fillPattern->copy();
    }
    strokePattern = state->strokePattern;
    if (strokePattern) {
        strokePattern = state->strokePattern->copy();
    }
    blendMode = state->blendMode;
    fillOpacity = state->fillOpacity;
    strokeOpacity = state->strokeOpacity;
    fillOverprint = state->fillOverprint;
    strokeOverprint = state->strokeOverprint;
    overprintMode = state->overprintMode;
    for (i = 0; i < 4; ++i) {
        transfer[i] = state->transfer[i];
        if (transfer[i]) {
            transfer[i] = state->transfer[i]->copy();
        }
    }
    lineWidth = state->lineWidth;
    lineDash = state->lineDash;
    lineDashStart = state->lineDashStart;
    flatness = state->flatness;
    lineJoin = state->lineJoin;
    lineCap = state->lineCap;
    miterLimit = state->miterLimit;
    strokeAdjust = state->strokeAdjust;
    alphaIsShape = state->alphaIsShape;
    textKnockout = state->textKnockout;

    font = state->font;
    fontSize = state->fontSize;
    memcpy(textMat, state->textMat, sizeof(textMat));
    charSpace = state->charSpace;
    wordSpace = state->wordSpace;
    horizScaling = state->horizScaling;
    leading = state->leading;
    rise = state->rise;
    render = state->render;

    path = state->path;
    if (copyPath) {
        path = state->path->copy();
    }
    curX = state->curX;
    curY = state->curY;
    lineX = state->lineX;
    lineY = state->lineY;

    clipXMin = state->clipXMin;
    clipYMin = state->clipYMin;
    clipXMax = state->clipXMax;
    clipYMax = state->clipYMax;
    memcpy(renderingIntent, state->renderingIntent, sizeof(renderingIntent));

    saved = nullptr;
#ifdef USE_CMS
    localDisplayProfile = state->localDisplayProfile;
    XYZ2DisplayTransformRelCol = state->XYZ2DisplayTransformRelCol;
    XYZ2DisplayTransformAbsCol = state->XYZ2DisplayTransformAbsCol;
    XYZ2DisplayTransformSat = state->XYZ2DisplayTransformSat;
    XYZ2DisplayTransformPerc = state->XYZ2DisplayTransformPerc;
#endif

    if (state->defaultGrayColorSpace) {
        defaultGrayColorSpace = state->defaultGrayColorSpace->copy();
    } else {
        defaultGrayColorSpace = nullptr;
    }
    if (state->defaultRGBColorSpace) {
        defaultRGBColorSpace = state->defaultRGBColorSpace->copy();
    } else {
        defaultRGBColorSpace = nullptr;
    }
    if (state->defaultCMYKColorSpace) {
        defaultCMYKColorSpace = state->defaultCMYKColorSpace->copy();
    } else {
        defaultCMYKColorSpace = nullptr;
    }
}

#ifdef USE_CMS

GfxLCMSProfilePtr GfxState::sRGBProfile = nullptr;
GfxLCMSProfilePtr GfxState::XYZProfile = nullptr;

void GfxState::setDisplayProfile(const GfxLCMSProfilePtr &localDisplayProfileA)
{
    localDisplayProfile = localDisplayProfileA;
    if (localDisplayProfile) {
        cmsHTRANSFORM transform;
        unsigned int nChannels;
        unsigned int localDisplayPixelType;

        localDisplayPixelType = getCMSColorSpaceType(cmsGetColorSpace(localDisplayProfile.get()));
        nChannels = getCMSNChannels(cmsGetColorSpace(localDisplayProfile.get()));
        // create transform from XYZ
        if ((transform = cmsCreateTransform(XYZProfile.get(), TYPE_XYZ_DBL, localDisplayProfile.get(), COLORSPACE_SH(localDisplayPixelType) | CHANNELS_SH(nChannels) | BYTES_SH(1), INTENT_RELATIVE_COLORIMETRIC, LCMS_FLAGS)) == nullptr) {
            error(errSyntaxWarning, -1, "Can't create Lab transform");
        } else {
            XYZ2DisplayTransformRelCol = std::make_shared<GfxColorTransform>(transform, INTENT_RELATIVE_COLORIMETRIC, PT_XYZ, localDisplayPixelType);
        }

        if ((transform = cmsCreateTransform(XYZProfile.get(), TYPE_XYZ_DBL, localDisplayProfile.get(), COLORSPACE_SH(localDisplayPixelType) | CHANNELS_SH(nChannels) | BYTES_SH(1), INTENT_ABSOLUTE_COLORIMETRIC, LCMS_FLAGS)) == nullptr) {
            error(errSyntaxWarning, -1, "Can't create Lab transform");
        } else {
            XYZ2DisplayTransformAbsCol = std::make_shared<GfxColorTransform>(transform, INTENT_ABSOLUTE_COLORIMETRIC, PT_XYZ, localDisplayPixelType);
        }

        if ((transform = cmsCreateTransform(XYZProfile.get(), TYPE_XYZ_DBL, localDisplayProfile.get(), COLORSPACE_SH(localDisplayPixelType) | CHANNELS_SH(nChannels) | BYTES_SH(1), INTENT_SATURATION, LCMS_FLAGS)) == nullptr) {
            error(errSyntaxWarning, -1, "Can't create Lab transform");
        } else {
            XYZ2DisplayTransformSat = std::make_shared<GfxColorTransform>(transform, INTENT_SATURATION, PT_XYZ, localDisplayPixelType);
        }

        if ((transform = cmsCreateTransform(XYZProfile.get(), TYPE_XYZ_DBL, localDisplayProfile.get(), COLORSPACE_SH(localDisplayPixelType) | CHANNELS_SH(nChannels) | BYTES_SH(1), INTENT_PERCEPTUAL, LCMS_FLAGS)) == nullptr) {
            error(errSyntaxWarning, -1, "Can't create Lab transform");
        } else {
            XYZ2DisplayTransformPerc = std::make_shared<GfxColorTransform>(transform, INTENT_PERCEPTUAL, PT_XYZ, localDisplayPixelType);
        }
    }
}

std::shared_ptr<GfxColorTransform> GfxState::getXYZ2DisplayTransform()
{
    auto transform = XYZ2DisplayTransformRelCol;
    if (strcmp(renderingIntent, "AbsoluteColorimetric") == 0) {
        transform = XYZ2DisplayTransformAbsCol;
    } else if (strcmp(renderingIntent, "Saturation") == 0) {
        transform = XYZ2DisplayTransformSat;
    } else if (strcmp(renderingIntent, "Perceptual") == 0) {
        transform = XYZ2DisplayTransformPerc;
    }
    return transform;
}

int GfxState::getCmsRenderingIntent()
{
    const char *intent = getRenderingIntent();
    int cmsIntent = INTENT_RELATIVE_COLORIMETRIC;
    if (intent) {
        if (strcmp(intent, "AbsoluteColorimetric") == 0) {
            cmsIntent = INTENT_ABSOLUTE_COLORIMETRIC;
        } else if (strcmp(intent, "Saturation") == 0) {
            cmsIntent = INTENT_SATURATION;
        } else if (strcmp(intent, "Perceptual") == 0) {
            cmsIntent = INTENT_PERCEPTUAL;
        }
    }
    return cmsIntent;
}

#endif

void GfxState::setPath(GfxPath *pathA)
{
    delete path;
    path = pathA;
}

void GfxState::getUserClipBBox(double *xMin, double *yMin, double *xMax, double *yMax) const
{
    double ictm[6];
    double xMin1, yMin1, xMax1, yMax1, tx, ty;

    // invert the CTM
    const double det_denominator = (ctm[0] * ctm[3] - ctm[1] * ctm[2]);
    if (unlikely(det_denominator == 0)) {
        *xMin = 0;
        *yMin = 0;
        *xMax = 0;
        *yMax = 0;
        return;
    }
    const double det = 1 / det_denominator;
    ictm[0] = ctm[3] * det;
    ictm[1] = -ctm[1] * det;
    ictm[2] = -ctm[2] * det;
    ictm[3] = ctm[0] * det;
    ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
    ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;

    // transform all four corners of the clip bbox; find the min and max
    // x and y values
    xMin1 = xMax1 = clipXMin * ictm[0] + clipYMin * ictm[2] + ictm[4];
    yMin1 = yMax1 = clipXMin * ictm[1] + clipYMin * ictm[3] + ictm[5];
    tx = clipXMin * ictm[0] + clipYMax * ictm[2] + ictm[4];
    ty = clipXMin * ictm[1] + clipYMax * ictm[3] + ictm[5];
    if (tx < xMin1) {
        xMin1 = tx;
    } else if (tx > xMax1) {
        xMax1 = tx;
    }
    if (ty < yMin1) {
        yMin1 = ty;
    } else if (ty > yMax1) {
        yMax1 = ty;
    }
    tx = clipXMax * ictm[0] + clipYMin * ictm[2] + ictm[4];
    ty = clipXMax * ictm[1] + clipYMin * ictm[3] + ictm[5];
    if (tx < xMin1) {
        xMin1 = tx;
    } else if (tx > xMax1) {
        xMax1 = tx;
    }
    if (ty < yMin1) {
        yMin1 = ty;
    } else if (ty > yMax1) {
        yMax1 = ty;
    }
    tx = clipXMax * ictm[0] + clipYMax * ictm[2] + ictm[4];
    ty = clipXMax * ictm[1] + clipYMax * ictm[3] + ictm[5];
    if (tx < xMin1) {
        xMin1 = tx;
    } else if (tx > xMax1) {
        xMax1 = tx;
    }
    if (ty < yMin1) {
        yMin1 = ty;
    } else if (ty > yMax1) {
        yMax1 = ty;
    }

    *xMin = xMin1;
    *yMin = yMin1;
    *xMax = xMax1;
    *yMax = yMax1;
}

double GfxState::transformWidth(double w) const
{
    double x, y;

    x = ctm[0] + ctm[2];
    y = ctm[1] + ctm[3];
    return w * sqrt(0.5 * (x * x + y * y));
}

double GfxState::getTransformedFontSize() const
{
    double x1, y1, x2, y2;

    x1 = textMat[2] * fontSize;
    y1 = textMat[3] * fontSize;
    x2 = ctm[0] * x1 + ctm[2] * y1;
    y2 = ctm[1] * x1 + ctm[3] * y1;
    return sqrt(x2 * x2 + y2 * y2);
}

void GfxState::getFontTransMat(double *m11, double *m12, double *m21, double *m22) const
{
    *m11 = (textMat[0] * ctm[0] + textMat[1] * ctm[2]) * fontSize;
    *m12 = (textMat[0] * ctm[1] + textMat[1] * ctm[3]) * fontSize;
    *m21 = (textMat[2] * ctm[0] + textMat[3] * ctm[2]) * fontSize;
    *m22 = (textMat[2] * ctm[1] + textMat[3] * ctm[3]) * fontSize;
}

void GfxState::setCTM(double a, double b, double c, double d, double e, double f)
{
    ctm[0] = a;
    ctm[1] = b;
    ctm[2] = c;
    ctm[3] = d;
    ctm[4] = e;
    ctm[5] = f;
}

void GfxState::concatCTM(double a, double b, double c, double d, double e, double f)
{
    double a1 = ctm[0];
    double b1 = ctm[1];
    double c1 = ctm[2];
    double d1 = ctm[3];

    ctm[0] = a * a1 + b * c1;
    ctm[1] = a * b1 + b * d1;
    ctm[2] = c * a1 + d * c1;
    ctm[3] = c * b1 + d * d1;
    ctm[4] = e * a1 + f * c1 + ctm[4];
    ctm[5] = e * b1 + f * d1 + ctm[5];
}

void GfxState::shiftCTMAndClip(double tx, double ty)
{
    ctm[4] += tx;
    ctm[5] += ty;
    clipXMin += tx;
    clipYMin += ty;
    clipXMax += tx;
    clipYMax += ty;
}

void GfxState::setFillColorSpace(GfxColorSpace *colorSpace)
{
    if (fillColorSpace) {
        delete fillColorSpace;
    }
    fillColorSpace = colorSpace;
}

void GfxState::setStrokeColorSpace(GfxColorSpace *colorSpace)
{
    if (strokeColorSpace) {
        delete strokeColorSpace;
    }
    strokeColorSpace = colorSpace;
}

void GfxState::setFillPattern(GfxPattern *pattern)
{
    if (fillPattern) {
        delete fillPattern;
    }
    fillPattern = pattern;
}

void GfxState::setStrokePattern(GfxPattern *pattern)
{
    if (strokePattern) {
        delete strokePattern;
    }
    strokePattern = pattern;
}

void GfxState::setFont(std::shared_ptr<GfxFont> fontA, double fontSizeA)
{
    font = std::move(fontA);
    fontSize = fontSizeA;
}

void GfxState::setTransfer(Function **funcs)
{
    int i;

    for (i = 0; i < 4; ++i) {
        if (transfer[i]) {
            delete transfer[i];
        }
        transfer[i] = funcs[i];
    }
}

void GfxState::setLineDash(std::vector<double> &&dash, double start)
{
    lineDash = dash;
    lineDashStart = start;
}

void GfxState::clearPath()
{
    delete path;
    path = new GfxPath();
}

void GfxState::clip()
{
    double xMin, yMin, xMax, yMax, x, y;
    GfxSubpath *subpath;
    int i, j;

    xMin = xMax = yMin = yMax = 0; // make gcc happy
    for (i = 0; i < path->getNumSubpaths(); ++i) {
        subpath = path->getSubpath(i);
        for (j = 0; j < subpath->getNumPoints(); ++j) {
            transform(subpath->getX(j), subpath->getY(j), &x, &y);
            if (i == 0 && j == 0) {
                xMin = xMax = x;
                yMin = yMax = y;
            } else {
                if (x < xMin) {
                    xMin = x;
                } else if (x > xMax) {
                    xMax = x;
                }
                if (y < yMin) {
                    yMin = y;
                } else if (y > yMax) {
                    yMax = y;
                }
            }
        }
    }
    if (xMin > clipXMin) {
        clipXMin = xMin;
    }
    if (yMin > clipYMin) {
        clipYMin = yMin;
    }
    if (xMax < clipXMax) {
        clipXMax = xMax;
    }
    if (yMax < clipYMax) {
        clipYMax = yMax;
    }
}

void GfxState::clipToStrokePath()
{
    double xMin, yMin, xMax, yMax, x, y, t0, t1;
    GfxSubpath *subpath;
    int i, j;

    xMin = xMax = yMin = yMax = 0; // make gcc happy
    for (i = 0; i < path->getNumSubpaths(); ++i) {
        subpath = path->getSubpath(i);
        for (j = 0; j < subpath->getNumPoints(); ++j) {
            transform(subpath->getX(j), subpath->getY(j), &x, &y);
            if (i == 0 && j == 0) {
                xMin = xMax = x;
                yMin = yMax = y;
            } else {
                if (x < xMin) {
                    xMin = x;
                } else if (x > xMax) {
                    xMax = x;
                }
                if (y < yMin) {
                    yMin = y;
                } else if (y > yMax) {
                    yMax = y;
                }
            }
        }
    }

    // allow for the line width
    //~ miter joins can extend farther than this
    t0 = fabs(ctm[0]);
    t1 = fabs(ctm[2]);
    if (t0 > t1) {
        xMin -= 0.5 * lineWidth * t0;
        xMax += 0.5 * lineWidth * t0;
    } else {
        xMin -= 0.5 * lineWidth * t1;
        xMax += 0.5 * lineWidth * t1;
    }
    t0 = fabs(ctm[0]);
    t1 = fabs(ctm[3]);
    if (t0 > t1) {
        yMin -= 0.5 * lineWidth * t0;
        yMax += 0.5 * lineWidth * t0;
    } else {
        yMin -= 0.5 * lineWidth * t1;
        yMax += 0.5 * lineWidth * t1;
    }

    if (xMin > clipXMin) {
        clipXMin = xMin;
    }
    if (yMin > clipYMin) {
        clipYMin = yMin;
    }
    if (xMax < clipXMax) {
        clipXMax = xMax;
    }
    if (yMax < clipYMax) {
        clipYMax = yMax;
    }
}

void GfxState::clipToRect(double xMin, double yMin, double xMax, double yMax)
{
    double x, y, xMin1, yMin1, xMax1, yMax1;

    transform(xMin, yMin, &x, &y);
    xMin1 = xMax1 = x;
    yMin1 = yMax1 = y;
    transform(xMax, yMin, &x, &y);
    if (x < xMin1) {
        xMin1 = x;
    } else if (x > xMax1) {
        xMax1 = x;
    }
    if (y < yMin1) {
        yMin1 = y;
    } else if (y > yMax1) {
        yMax1 = y;
    }
    transform(xMax, yMax, &x, &y);
    if (x < xMin1) {
        xMin1 = x;
    } else if (x > xMax1) {
        xMax1 = x;
    }
    if (y < yMin1) {
        yMin1 = y;
    } else if (y > yMax1) {
        yMax1 = y;
    }
    transform(xMin, yMax, &x, &y);
    if (x < xMin1) {
        xMin1 = x;
    } else if (x > xMax1) {
        xMax1 = x;
    }
    if (y < yMin1) {
        yMin1 = y;
    } else if (y > yMax1) {
        yMax1 = y;
    }

    if (xMin1 > clipXMin) {
        clipXMin = xMin1;
    }
    if (yMin1 > clipYMin) {
        clipYMin = yMin1;
    }
    if (xMax1 < clipXMax) {
        clipXMax = xMax1;
    }
    if (yMax1 < clipYMax) {
        clipYMax = yMax1;
    }
}

void GfxState::textShift(double tx, double ty)
{
    double dx, dy;

    textTransformDelta(tx, ty, &dx, &dy);
    curX += dx;
    curY += dy;
}

void GfxState::shift(double dx, double dy)
{
    curX += dx;
    curY += dy;
}

GfxState *GfxState::save()
{
    GfxState *newState;

    newState = copy();
    newState->saved = this;
    return newState;
}

GfxState *GfxState::restore()
{
    GfxState *oldState;

    if (saved) {
        oldState = saved;

        // these attributes aren't saved/restored by the q/Q operators
        oldState->path = path;
        oldState->curX = curX;
        oldState->curY = curY;
        oldState->lineX = lineX;
        oldState->lineY = lineY;

        path = nullptr;
        saved = nullptr;
        delete this;

    } else {
        oldState = this;
    }

    return oldState;
}

bool GfxState::parseBlendMode(Object *obj, GfxBlendMode *mode)
{
    int i, j;

    if (obj->isName()) {
        for (i = 0; i < nGfxBlendModeNames; ++i) {
            if (!strcmp(obj->getName(), gfxBlendModeNames[i].name)) {
                *mode = gfxBlendModeNames[i].mode;
                return true;
            }
        }
        return false;
    } else if (obj->isArray()) {
        for (i = 0; i < obj->arrayGetLength(); ++i) {
            Object obj2 = obj->arrayGet(i);
            if (!obj2.isName()) {
                return false;
            }
            for (j = 0; j < nGfxBlendModeNames; ++j) {
                if (!strcmp(obj2.getName(), gfxBlendModeNames[j].name)) {
                    *mode = gfxBlendModeNames[j].mode;
                    return true;
                }
            }
        }
        *mode = gfxBlendNormal;
        return true;
    } else {
        return false;
    }
}
