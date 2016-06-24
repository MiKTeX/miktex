/* YapConfig.cpp: Yap configuration settings

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "yap.h"

unique_ptr<YapConfig> g_pYapConfig;

#define DEFAULT_DVIPAGE_MODE DviPageMode::Pk
#define DEFAULT_DVIPAGE_MODE_STRING \
  EnumToString<DviPageMode>(DviPageMode::Pk, dviPageModes)

#define DEFAULT_UNIT Unit::BigPoints
#define DEFAULT_UNIT_STRING \
  EnumToString<Unit>(Unit::BigPoints, units)

template<class Enum> struct EnumAndString
{
  Enum num;
  const char * lpsz;
};

namespace
{
  const EnumAndString<DviPageMode> dviPageModes[] = {
    { DviPageMode::Pk, "pk" },
#if MIKTEX_SERIES_INT <= 209
    { DviPageMode::Pk, "auto" },
#endif
    { DviPageMode::Dvips, "dvips" },
    { DviPageMode::None, 0 },
  };

  const EnumAndString<Unit> units[] = {
    { Unit::BigPoints, "bp" },
    { Unit::Centimeters, "cm" },
    { Unit::Inches, "in" },
    { Unit::Millimeters, "mm" },
    { Unit::Picas, "pc" },
    { Unit::None, 0 },
  };


  const char * const DEFAULT_SECURE_COMMANDS = "gunzip,zcat,bunzip2,bzcat";

  const char * const DEFAULT_GAMMA_VALUES = "0.01 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0 1.25 1.5 1.75 2.0 3.0 4.0 5.0 7.5 10.0 100.0";
};

template<class Enum> const char * EnumToString(Enum num, const EnumAndString<Enum> * pMap)
{
  for (size_t idx = 0; pMap[idx].lpsz != nullptr; ++idx)
  {
    if (pMap[idx].num == num)
    {
      return pMap[idx].lpsz;
    }
  }
  MIKTEX_UNEXPECTED();
}

template<class Enum> Enum StringToEnum(const char * lpsz, const EnumAndString<Enum> * pMap)
{
  for (size_t idx = 0; pMap[idx].lpsz != nullptr; ++idx)
  {
    if (Utils::EqualsIgnoreCase(lpsz, pMap[idx].lpsz))
    {
      return pMap[idx].num;
    }
  }
  MIKTEX_UNEXPECTED();
}

bool SetMetafontMode(const char * lpszMnemonic, unsigned long & metafontModeIdx)
{
  shared_ptr<Session> session = Session::Get();
  MIKTEXMFMODE metafontMode;
  for (unsigned long idx = 0; session->GetMETAFONTMode(idx, &metafontMode); ++idx)
  {
    if (Utils::Equals(lpszMnemonic, metafontMode.szMnemonic))
    {
      metafontModeIdx = idx;
      return true;
    }
  }
  return false;
}

bool YapConfig::SetDisplayMetafontMode(const char * lpszMnemonic)
{
  return SetMetafontMode(lpszMnemonic, displayMetafontMode);
}

bool YapConfig::SetPrinterMetafontMode(const char * lpszMnemonic)
{
  return SetMetafontMode(lpszMnemonic, printerMetafontMode);
}

static string DefaultInverseSearchTemplate()
{
  shared_ptr<Session> session = Session::Get();
  string inverseSearchTemplate;
  PathName texworks;
  if (session->FindFile(MIKTEX_TEXWORKS_EXE, FileType::EXE, texworks))
  {
    inverseSearchTemplate = Q_(texworks);
    inverseSearchTemplate += " -p=%l \"%f\"";
  }
  else
  {
    inverseSearchTemplate = "notepad \"%f\"";
  }
  return inverseSearchTemplate;
} void
YapConfig::Load()
{
  PathName fileName = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  fileName /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  fileName /= MIKTEX_YAP_INI_FILENAME;
  if (File::Exists(fileName))
  {
    pCfg->Read(fileName);
  }

  //
  // [Settings]
  //
  showSplashWindow = GetValue("Settings", "Show Splash Window", true);
  checkFileTypeAssociations = GetValue("Settings", "Check Associations", true);
  inverseSearchCommandLine = GetValue("Settings", "Editor", DefaultInverseSearchTemplate());
  lastTool = GetValue("Settings", "Last Tool", 0);
  maintainHorizontalPosition = GetValue("Settings", "Maintain Horizontal Position", false);
  maintainVerticalPosition = GetValue("Settings", "Maintain Vertical Position", false);

#if defined(MIKTEX_DEBUG)
  showBoundingBoxes = GetValue("Settings", "Show Bounding Boxes", false);
#endif
  showSourceSpecials = GetValue("Settings", "Show Source Specials", false);

  //
  // [screen]
  //
  continuousView = GetValue("screen", "ContinuousView", true);
#if WIN95NOCONTVIEW
  if (continuousView && win95)
  {
    continuousView = false;
  }
#endif
  doublePage = GetValue("screen", "doublepage", false);
  gamma = GetValue("screen", "gamma", 1.0);
  if (gamma <= 0.0)
  {
    gamma = 1.0;
  }
  gammaValues = GetValue("screen", "gammavalues", string(DEFAULT_GAMMA_VALUES));
  string displayMode = GetValue("screen", "mode", string("ljfour"));
  if (!SetDisplayMetafontMode(displayMode.c_str()))
  {
    MIKTEX_UNEXPECTED();
  }
  displayShrinkFactor = GetValue("screen", "shrinkfactor", 6);
  if (displayShrinkFactor == 0)
  {
    displayShrinkFactor = 1;
  }
  renderGraphicsInBackground = GetValue("screen", "graphicsinbackground", DEFAULT_bRenderGraphicsInBackground);
  dviPageMode = StringToEnum<DviPageMode>(GetValue("screen", "dvipagemode", string(DEFAULT_DVIPAGE_MODE_STRING)).c_str(), dviPageModes);
  unit = StringToEnum<Unit>(GetValue("screen", "unit", string(DEFAULT_UNIT_STRING)).c_str(), units);

  //
  // [printer]
  //
  string printerMode = GetValue("printer", "mode", string("ljfour"));
  if (!SetPrinterMetafontMode(printerMode.c_str()))
  {
    MIKTEX_UNEXPECTED();
  }
  pageXShift = GetValue("printer", "nPageXShift", 0);
  pageYShift = GetValue("printer", "nPageYShift", 0);

  //
  // [DVIPS]
  //
  dvipsExtraOptions = GetValue("DVIPS", "Extra Options", string(""));

  //
  // [Magnifying Glass]
  //
  magGlassHidesCursor = GetValue("Magnifying Glass", "Hide Cursor", false);
  magGlassOpacity = GetValue("Magnifying Glass", "Opacity", 70);

  //
  // [Small Magnifying Glass]
  //
  magGlassSmallHeight = GetValue("Small Magnifying Glass", "Height", 150);
  magGlassSmallShrinkFactor = GetValue("Small Magnifying Glass", "Shrink Factor", 1);
  if (magGlassSmallShrinkFactor == 0)
  {
    magGlassSmallShrinkFactor = 1;
  }
  magGlassSmallWidth = GetValue("Small Magnifying Glass", "Width", 200);

  //
  // [Medium Magnifying Glass]
  //
  magGlassMediumHeight = GetValue("Medium Magnifying Glass", "Height", 250);
  magGlassMediumShrinkFactor = GetValue("Medium Magnifying Glass", "Shrink Factor", 1);
  if (magGlassMediumShrinkFactor == 0)
  {
    magGlassMediumShrinkFactor = 1;
  }
  magGlassMediumWidth = GetValue("Medium Magnifying Glass", "Width", 400);

  //
  // [Large Magnifying Glass]
  //
  magGlassLargeHeight = GetValue("Large Magnifying Glass", "Height", 500);
  magGlassLargeShrinkFactor = GetValue("Large Magnifying Glass", "Shrink Factor", 1);
  if (magGlassLargeShrinkFactor == 0)
  {
    magGlassLargeShrinkFactor = 1;
  }
  magGlassLargeWidth = GetValue("Large Magnifying Glass", "Width", 700);

  //
  // [Security]
  //
  enableShellCommands = GetValue("Security", "EnableEmbeddedCommands", SEC_ASK_USER);
  secureCommands = GetValue("Security", "SecureCommands", string(DEFAULT_SECURE_COMMANDS));
}

void YapConfig::Save()
{
  MIKTEXMFMODE mfmode;

  //
  // [Settings]
  //
  UpdateValue("Settings", "Show Splash Window", showSplashWindow, true);
  UpdateValue("Settings", "Check Associations", checkFileTypeAssociations, true);
  UpdateValue("Settings", "Editor", inverseSearchCommandLine, DefaultInverseSearchTemplate());
  UpdateValue("Settings", "Last Tool", lastTool, 0);
  UpdateValue("Settings", "Maintain Horizontal Position", maintainHorizontalPosition, false);
  UpdateValue("Settings", "Maintain Vertical Position", maintainVerticalPosition, false);
#if defined(MIKTEX_DEBUG)
  UpdateValue("Settings","Show Bounding Boxes",showBoundingBoxes,false);
#endif
  UpdateValue("Settings","Show Source Specials",showSourceSpecials,false);

  //
  // [screen]
  //
  UpdateValue("screen", "ContinuousView", continuousView, true);
  UpdateValue("screen", "DoublePage", doublePage, false);
  UpdateValue("screen", "gamma" , gamma, 1.0);
  UpdateValue("screen", "gammavalues", gammaValues, string(DEFAULT_GAMMA_VALUES));
  if (session->GetMETAFONTMode(displayMetafontMode, &mfmode))
  {
    UpdateValue("screen", "mode", string(mfmode.szMnemonic), string("ljfour"));
  }
  UpdateValue("screen", "shrinkfactor", displayShrinkFactor,6);
  UpdateValue("screen", "graphicsinbackground", renderGraphicsInBackground, DEFAULT_bRenderGraphicsInBackground);
  UpdateValue("screen", "dvipagemode", string(EnumToString<DviPageMode>(dviPageMode, dviPageModes)), string(DEFAULT_DVIPAGE_MODE_STRING));
  UpdateValue("screen", "unit",string(EnumToString<Unit>(unit, units)), string(DEFAULT_UNIT_STRING));

  //
  // [printer]
  //
  if (session->GetMETAFONTMode(printerMetafontMode, &mfmode))
  {
    UpdateValue("printer", "mode", string(mfmode.szMnemonic), string("ljfour"));
  }
  UpdateValue("printer", "nPageXShift", pageXShift, 0);
  UpdateValue("printer", "nPageYShift", pageYShift, 0);

  //
  // [DVIPS]
  //
  UpdateValue("DVIPS", "Extra Options", dvipsExtraOptions, string(""));

  //
  // [Magnifying Glass]
  //
  UpdateValue("Magnifying Glass", "Hide Cursor", magGlassHidesCursor, false);
  UpdateValue("Magnifying Glass", "Opacity", magGlassOpacity, 70);

  //
  // [Small Magnifying Glass]
  //
  UpdateValue("Small Magnifying Glass", "Height", magGlassSmallHeight, 150);
  UpdateValue("Small Magnifying Glass", "Shrink Factor", magGlassSmallShrinkFactor, 1);
  UpdateValue("Small Magnifying Glass", "Width", magGlassSmallWidth, 200);

  //
  // [Medium Magnifying Glass]
  //
  UpdateValue("Medium Magnifying Glass", "Height", magGlassMediumHeight, 250);
  UpdateValue("Medium Magnifying Glass", "Shrink Factor", magGlassMediumShrinkFactor, 1);
  UpdateValue("Medium Magnifying Glass", "Width", magGlassMediumWidth, 400);

  //
  // [Large Magnifying Glass]
  //
  UpdateValue("Large Magnifying Glass", "Height", magGlassLargeHeight, 500);
  UpdateValue("Large Magnifying Glass", "Shrink Factor", magGlassLargeShrinkFactor, 1);
  UpdateValue("Large Magnifying Glass", "Width", magGlassLargeWidth, 700);

  //
  // [Security]
  //
  UpdateValue("Security", "EnableEmbeddedCommands", enableShellCommands, int(SEC_ASK_USER));
  UpdateValue("Security", "SecureCommands", secureCommands, string(DEFAULT_SECURE_COMMANDS));

  PathName fileName = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  fileName /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  fileName /= MIKTEX_YAP_INI_FILENAME;
  bool firstTime = !File::Exists(fileName);
  pCfg->Write(fileName);
  if (firstTime)
  {
    Fndb::Add(fileName);
  }
}

YapConfig::YapConfig() :
  dviPageMode(DEFAULT_DVIPAGE_MODE),
  unit(Unit::None),
  win95((GetVersion() & 0x80000000) != 0),
  showSplashWindow(true),
  pCfg(Cfg::Create())
{
  Load();
}

YapConfig::~YapConfig()
{
#if 0
  try
  {
    Save();
  }
  catch (const exception &)
  {
  }
#endif
}
