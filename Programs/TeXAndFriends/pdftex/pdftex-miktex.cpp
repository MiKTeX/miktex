/* pdftex-miktex.cpp:
   
   Copyright (C) 1998-2016 Christian Schenk

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

#include "pdftex-miktex.h"

#include <poppler/poppler-config.h>
#include <jpeglib.h>
#include <png.h>
#include <zlib.h>

using namespace MiKTeX::Core;

void PDFTEXCLASS::GetLibraryVersions(std::vector<LibraryVersion> & versions) const
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
}
