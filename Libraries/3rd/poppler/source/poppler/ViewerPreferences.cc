//========================================================================
//
// ViewerPreferences.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2011 Pino Toscano <pino@kde.org>
// Copyright 2017 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include <config.h>

#include "ViewerPreferences.h"

#include "Object.h"
#include "Dict.h"

ViewerPreferences::ViewerPreferences(Dict *prefDict)
{
  init();

  Object obj = prefDict->lookup("HideToolbar");
  if (obj.isBool()) {
    hideToolbar = obj.getBool();
  }

  obj = prefDict->lookup("HideMenubar");
  if (obj.isBool()) {
    hideMenubar = obj.getBool();
  }

  obj = prefDict->lookup("HideWindowUI");
  if (obj.isBool()) {
    hideWindowUI = obj.getBool();
  }

  obj = prefDict->lookup("FitWindow");
  if (obj.isBool()) {
    fitWindow = obj.getBool();
  }

  obj = prefDict->lookup("CenterWindow");
  if (obj.isBool()) {
    centerWindow = obj.getBool();
  }

  obj = prefDict->lookup("DisplayDocTitle");
  if (obj.isBool()) {
    displayDocTitle = obj.getBool();
  }

  obj = prefDict->lookup("NonFullScreenPageMode");
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
}

ViewerPreferences::~ViewerPreferences()
{
}

void ViewerPreferences::init()
{
  hideToolbar = gFalse;
  hideMenubar = gFalse;
  hideWindowUI = gFalse;
  fitWindow = gFalse;
  centerWindow = gFalse;
  displayDocTitle = gFalse;
  nonFullScreenPageMode = nfpmUseNone;
  direction = directionL2R;
  printScaling = printScalingAppDefault;
  duplex = duplexNone;
}
