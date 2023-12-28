//========================================================================
//
// sanitychecks.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2020 Philipp Knechtges <philipp-dev@knechtges.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include "sanitychecks.h"

#ifdef USE_CMS
bool checkICCProfile(const GfxLCMSProfilePtr &profile, const char *filename, cmsUInt32Number UsedDirection, cmsColorSpaceSignature expectedColorSpace)
{
    if (!profile) {
        fprintf(stderr, "Could not open the ICC profile \"%s\".\n", filename);
        return false;
    }
    if (!cmsIsIntentSupported(profile.get(), INTENT_RELATIVE_COLORIMETRIC, UsedDirection) && !cmsIsIntentSupported(profile.get(), INTENT_ABSOLUTE_COLORIMETRIC, UsedDirection)
        && !cmsIsIntentSupported(profile.get(), INTENT_SATURATION, UsedDirection) && !cmsIsIntentSupported(profile.get(), INTENT_PERCEPTUAL, LCMS_USED_AS_OUTPUT)) {
        if (UsedDirection == LCMS_USED_AS_OUTPUT) {
            fprintf(stderr, "ICC profile \"%s\" is not an output profile.\n", filename);
        } else if (UsedDirection == LCMS_USED_AS_INPUT) {
            fprintf(stderr, "ICC profile \"%s\" is not an input profile.\n", filename);
        } else {
            fprintf(stderr, "ICC profile \"%s\" is not suitable.\n", filename);
        }
        return false;
    }
    auto profilecolorspace = cmsGetColorSpace(profile.get());
    if (profilecolorspace != expectedColorSpace) {
        if (expectedColorSpace == cmsSigCmykData) {
            fprintf(stderr, "Supplied ICC profile \"%s\" is not a CMYK profile.\n", filename);
        } else if (expectedColorSpace == cmsSigGrayData) {
            fprintf(stderr, "Supplied ICC profile \"%s\" is not a monochrome profile.\n", filename);
        } else if (expectedColorSpace == cmsSigRgbData) {
            fprintf(stderr, "Supplied ICC profile \"%s\" is not a RGB profile.\n", filename);
        }
        return false;
    }
    return true;
}
#endif
