/* xetex-miktex.cpp:
   
   Copyright (C) 2007-2017 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "miktex-first.h"

#include <poppler-config.h>
#include <hb-icu.h>
#include <graphite2/Font.h>
#include <jpeglib.h>
#include <png.h>
#include <TECkit_Engine.h>
#include <zlib.h>

#include "xetex-miktex.h"

using namespace MiKTeX::Core;

// special case: Web2C likes to add 1 to the nameoffile base address
inline XETEXPROGCLASS::utf8code* GetNameOfFileForWeb2C()
{
  return &XETEXPROG.nameoffile[-1];
}

XETEXPROGCLASS::utf8code* nameoffile = GetNameOfFileForWeb2C();

void XETEXAPPCLASS::GetLibraryVersions(std::vector<LibraryVersion>& versions) const
{
  ETeXApp::GetLibraryVersions(versions);

  versions.push_back(LibraryVersion("zlib", ZLIB_VERSION, zlib_version));
  versions.push_back(LibraryVersion("libpng", PNG_LIBPNG_VER_STRING, png_libpng_ver));
  versions.push_back(LibraryVersion("poppler", POPPLER_VERSION, ""));
#if defined(JPEG_LIB_VERSION_MAJOR)
  VersionNumber jpegVersion(JPEG_LIB_VERSION_MAJOR, JPEG_LIB_VERSION_MINOR, 0, 0);
#else
  VersionNumber jpegVersion(JPEG_LIB_VERSION, 0, 0, 0);
#endif
  versions.push_back(LibraryVersion("jpeg", &jpegVersion, nullptr));

  int fc_version = FcGetVersion();
  VersionNumber fcVersion(FC_VERSION / 10000, (FC_VERSION % 10000) / 100, FC_VERSION % 100, 0);
  VersionNumber fcRunVersion(fc_version / 10000, (fc_version % 10000) / 100, fc_version % 100, 0);
  versions.push_back(LibraryVersion("fontconfig", &fcVersion, &fcRunVersion));

  FT_Int ftMajor, ftMinor, ftPatch;
  if (gFreeTypeLibrary == nullptr && FT_Init_FreeType(&gFreeTypeLibrary) != 0)
  {
    fprintf(stderr, "FreeType2 initialization failed!\n");
    throw 9;
  }
  FT_Library_Version(gFreeTypeLibrary, &ftMajor, &ftMinor, &ftPatch);
  VersionNumber ftVersion(FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH, 0);
  VersionNumber ftRunVersion(ftMajor, ftMinor, ftPatch, 0);
  versions.push_back(LibraryVersion("freetype2", &ftVersion, &ftRunVersion));

  int grMajor, grMinor, grBugfix;
  gr_engine_version(&grMajor, &grMinor, &grBugfix);
  VersionNumber grVersion(GR2_VERSION_MAJOR, GR2_VERSION_MINOR, GR2_VERSION_BUGFIX, 0);
  VersionNumber grRunVersion(grMajor, grMinor, grBugfix, 0);
  versions.push_back(LibraryVersion("graphite2", &grVersion, &grRunVersion));

  UVersionInfo icuVersion;
  char icu_version[U_MAX_VERSION_STRING_LENGTH] = "";
  u_getVersion(icuVersion);
  u_versionToString(icuVersion, icu_version);
  versions.push_back(LibraryVersion("icu", U_ICU_VERSION, icu_version));

  versions.push_back(LibraryVersion("harfbuzz", HB_VERSION_STRING, hb_version_string()));

  unsigned tkver = TECkit_GetVersion();
  VersionNumber tkVersion((tkver & 0xffff0000) >> 16, tkver & 0x0000ffff, 0, 0);
  versions.push_back(LibraryVersion("teckit", nullptr, &tkVersion));
}
