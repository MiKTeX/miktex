//========================================================================
//
// sanitychecks.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2020 Philipp Knechtges <philipp-dev@knechtges.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SANITYCHECKS_H
#define SANITYCHECKS_H

#include "config.h"

#ifdef USE_CMS
#    include <lcms2.h>
#    include "GfxState.h"

/*
 * Check the supplied ICC profile for different criteria
 */
bool checkICCProfile(const GfxLCMSProfilePtr &profile, const char *filename, cmsUInt32Number UsedDirection, cmsColorSpaceSignature expectedColorSpace);

#endif

#endif
