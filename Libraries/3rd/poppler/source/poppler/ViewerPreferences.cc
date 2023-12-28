//========================================================================
//
// ViewerPreferences.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2011 Pino Toscano <pino@kde.org>
// Copyright 2017, 2020, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright 2019 Marek Kasik <mkasik@redhat.com>
//
//========================================================================

#include <config.h>

#include "ViewerPreferences.h"

#include "Object.h"
#include "Dict.h"

ViewerPreferences::ViewerPreferences(Dict *prefDict)
{
    hideToolbar = prefDict->lookup("HideToolbar").getBoolWithDefaultValue(false);

    hideMenubar = prefDict->lookup("HideMenubar").getBoolWithDefaultValue(false);

    hideWindowUI = prefDict->lookup("HideWindowUI").getBoolWithDefaultValue(false);

    fitWindow = prefDict->lookup("FitWindow").getBoolWithDefaultValue(false);

    centerWindow = prefDict->lookup("CenterWindow").getBoolWithDefaultValue(false);

    displayDocTitle = prefDict->lookup("DisplayDocTitle").getBoolWithDefaultValue(false);

    Object obj = prefDict->lookup("NonFullScreenPageMode");
    if (obj.isName()) {
        const char *mode = obj.getName();
        if (!strcmp(mode, "UseNone")) {
            nonFullScreenPageMode = nfpmUseNone;
        } else if (!strcmp(mode, "UseOutlines")) {
            nonFullScreenPageMode = nfpmUseOutlines;
        } else if (!strcmp(mode, "UseThumbs")) {
            nonFullScreenPageMode = nfpmUseThumbs;
        } else if (!strcmp(mode, "UseOC")) {
            nonFullScreenPageMode = nfpmUseOC;
        }
    }

    obj = prefDict->lookup("Direction");
    if (obj.isName()) {
        const char *dir = obj.getName();
        if (!strcmp(dir, "L2R")) {
            direction = directionL2R;
        } else if (!strcmp(dir, "R2L")) {
            direction = directionR2L;
        }
    }

    obj = prefDict->lookup("PrintScaling");
    if (obj.isName()) {
        const char *ps = obj.getName();
        if (!strcmp(ps, "None")) {
            printScaling = printScalingNone;
        } else if (!strcmp(ps, "AppDefault")) {
            printScaling = printScalingAppDefault;
        }
    }

    obj = prefDict->lookup("Duplex");
    if (obj.isName()) {
        const char *d = obj.getName();
        if (!strcmp(d, "Simplex")) {
            duplex = duplexSimplex;
        } else if (!strcmp(d, "DuplexFlipShortEdge")) {
            duplex = duplexDuplexFlipShortEdge;
        } else if (!strcmp(d, "DuplexFlipLongEdge")) {
            duplex = duplexDuplexFlipLongEdge;
        }
    }

    pickTrayByPDFSize = prefDict->lookup("PickTrayByPDFSize").getBoolWithDefaultValue(false);

    obj = prefDict->lookup("NumCopies");
    if (obj.isInt()) {
        numCopies = obj.getInt();
        if (numCopies < 2) {
            numCopies = 1;
        }
    }

    obj = prefDict->lookup("PrintPageRange");
    if (obj.isArray()) {
        Array *range = obj.getArray();
        int length = range->getLength();
        int pageNumber1, pageNumber2;

        if (length % 2 == 1) {
            length--;
        }

        for (int i = 0; i < length; i += 2) {
            Object obj2 = range->get(i);
            Object obj3 = range->get(i + 1);

            if (obj2.isInt() && (pageNumber1 = obj2.getInt()) >= 1 && obj3.isInt() && (pageNumber2 = obj3.getInt()) >= 1 && pageNumber1 < pageNumber2) {
                printPageRange.emplace_back(pageNumber1, pageNumber2);
            } else {
                printPageRange.clear();
                break;
            }
        }
    }
}

ViewerPreferences::~ViewerPreferences() { }
