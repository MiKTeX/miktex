/* miktex-pdftex.cpp:
   
   Copyright (C) 1998-2021 Christian Schenk

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

#include "miktex-pdftex.h"

#include <jpeglib.h>
#include <png.h>
#include <xpdf/config.h>
#include <zlib.h>

#include "PdfTeXResources.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Resources;

static PdfTeXResources pdfTeXResources;
ResourceRepository* PDFTEXAPPCLASS::resources = &pdfTeXResources;

PDFTEXPROGCLASS::scaled& curh = PDFTEXPROG.curh;
PDFTEXPROGCLASS::instaterecord& curinput = PDFTEXPROG.curinput;
PDFTEXPROGCLASS::scaled& curv = PDFTEXPROG.curv;
PDFTEXPROGCLASS::memoryword*& eqtb = PDFTEXPROG.eqtb;
C4P::C4P_integer& fixeddecimaldigits = PDFTEXPROG.fixeddecimaldigits;
C4P::C4P_integer& fixedgamma = PDFTEXPROG.fixedgamma;
C4P::C4P_integer& fixedgentounicode = PDFTEXPROG.fixedgentounicode;
C4P::C4P_integer& fixedimageapplygamma = PDFTEXPROG.fixedimageapplygamma;
C4P::C4P_integer& fixedimagegamma = PDFTEXPROG.fixedimagegamma;
C4P::C4P_boolean& fixedimagehicolor = PDFTEXPROG.fixedimagehicolor;
C4P::C4P_integer& fixedinclusioncopyfont = PDFTEXPROG.fixedinclusioncopyfont;
C4P::C4P_integer& fixedpdfdraftmode = PDFTEXPROG.fixedpdfdraftmode;
C4P::C4P_integer& fixedpdfmajorversion = PDFTEXPROG.fixedpdfmajorversion;
C4P::C4P_integer& fixedpdfminorversion = PDFTEXPROG.fixedpdfminorversion;
C4P::C4P_integer& fixedpkresolution = PDFTEXPROG.fixedpkresolution;
PDFTEXPROGCLASS::eightbits*& fontbc = PDFTEXPROG.fontbc;
PDFTEXPROGCLASS::scaled*& fontdsize = PDFTEXPROG.fontdsize;
PDFTEXPROGCLASS::eightbits*& fontec = PDFTEXPROG.fontec;
C4P::C4P_integer& fontmax = PDFTEXPROG.fontmax;
PDFTEXPROGCLASS::strnumber*& fontname = PDFTEXPROG.fontname;
PDFTEXPROGCLASS::strnumber& formatident = PDFTEXPROG.formatident;
C4P::C4P_signed8& interaction = PDFTEXPROG.interaction;
PDFTEXPROGCLASS::strnumber& jobname = PDFTEXPROG.jobname;
PDFTEXPROGCLASS::strnumber& lasttokensstring = PDFTEXPROG.lasttokensstring;
C4P::C4P_integer& objptr = PDFTEXPROG.objptr;
PDFTEXPROGCLASS::objentry*& objtab = PDFTEXPROG.objtab;
PDFTEXPROGCLASS::scaled& onehundredbp = PDFTEXPROG.onehundredbp;
PDFTEXPROGCLASS::strnumber& outputfilename = PDFTEXPROG.outputfilename;
C4P::C4P_integer& pdfboxspecart = PDFTEXPROG.pdfboxspecart;
C4P::C4P_integer& pdfboxspecbleed = PDFTEXPROG.pdfboxspecbleed;
C4P::C4P_integer& pdfboxspeccrop = PDFTEXPROG.pdfboxspeccrop;
C4P::C4P_integer& pdfboxspecmedia = PDFTEXPROG.pdfboxspecmedia;
C4P::C4P_integer& pdfboxspectrim = PDFTEXPROG.pdfboxspectrim;
PDFTEXPROGCLASS::eightbits*& pdfbuf = PDFTEXPROG.pdfbuf;
C4P::C4P_integer& pdfbufsize = PDFTEXPROG.pdfbufsize;
PDFTEXPROGCLASS::charusedarray*& pdfcharused = PDFTEXPROG.pdfcharused;
C4P::C4P_integer& pdfcurform = PDFTEXPROG.pdfcurform;
PDFTEXPROGCLASS::bytefile& pdffile = PDFTEXPROG.pdffile;
PDFTEXPROGCLASS::strnumber*& pdffontattr = PDFTEXPROG.pdffontattr;
PDFTEXPROGCLASS::fmentryptr*& pdffontmap = PDFTEXPROG.pdffontmap;
C4P::C4P_boolean*& pdffontnobuiltintounicode = PDFTEXPROG.pdffontnobuiltintounicode;
PDFTEXPROGCLASS::scaled*& pdffontsize = PDFTEXPROG.pdffontsize;
C4P::C4P_longinteger& pdfgone = PDFTEXPROG.pdfgone;
C4P::C4P_integer& pdfimageprocset = PDFTEXPROG.pdfimageprocset;
PDFTEXPROGCLASS::eightbits& pdflastbyte = PDFTEXPROG.pdflastbyte;
C4P::C4P_boolean& pdfosmode = PDFTEXPROG.pdfosmode;
C4P::C4P_integer& pdfoutputvalue = PDFTEXPROG.pdfoutputvalue;
C4P::C4P_integer& pdfpagegroupval = PDFTEXPROG.pdfpagegroupval;
C4P::C4P_integer& pdfptr = PDFTEXPROG.pdfptr;
C4P::C4P_longinteger& pdfsaveoffset = PDFTEXPROG.pdfsaveoffset;
C4P::C4P_longinteger& pdfstreamlength = PDFTEXPROG.pdfstreamlength;
PDFTEXPROGCLASS::strnumber& pdftexbanner = PDFTEXPROG.pdftexbanner;
C4P::C4P_integer& pkscalefactor = PDFTEXPROG.pkscalefactor;
PDFTEXPROGCLASS::poolpointer& poolptr = PDFTEXPROG.poolptr;
C4P::C4P_integer& poolsize = PDFTEXPROG.poolsize;
PDFTEXPROGCLASS::scaled& ruledp = PDFTEXPROG.ruledp;
PDFTEXPROGCLASS::scaled& ruleht = PDFTEXPROG.ruleht;
PDFTEXPROGCLASS::scaled& rulewd = PDFTEXPROG.rulewd;
PDFTEXPROGCLASS::packedasciicode*& strpool = PDFTEXPROG.strpool;
PDFTEXPROGCLASS::poolpointer*& strstart = PDFTEXPROG.strstart;
C4P::C4P_integer& termoffset = PDFTEXPROG.termoffset;
PDFTEXPROGCLASS::strnumber& texmflogname = PDFTEXPROG.logname;
C4P::C4P_integer& totalpages = PDFTEXPROG.totalpages;
C4P::C4P_integer*& vfefnts = PDFTEXPROG.vfefnts;
PDFTEXPROGCLASS::internalfontnumber*& vfifnts = PDFTEXPROG.vfifnts;
C4P::C4P_integer*& vfpacketbase = PDFTEXPROG.vfpacketbase;
C4P::C4P_integer& vfpacketlength = PDFTEXPROG.vfpacketlength;
PDFTEXPROGCLASS::memoryword*& zmem = PDFTEXPROG.zmem;

char* nameoffile = nullptr;

#if WITH_SYNCTEX
C4P::C4P_integer& synctexoption = PDFTEXPROG.synctexoption;
C4P::C4P_integer& synctexoffset = PDFTEXPROG.synctexoffset;
#endif

C4P::C4P_integer k;

void PDFTEXAPPCLASS::GetLibraryVersions(std::vector<LibraryVersion>& versions) const
{
  ETeXApp::GetLibraryVersions(versions);
  versions.push_back(LibraryVersion("zlib", ZLIB_VERSION, zlib_version));
  versions.push_back(LibraryVersion("libpng", PNG_LIBPNG_VER_STRING, png_libpng_ver));
  versions.push_back(LibraryVersion("xpdf", xpdfVersion, ""));
#if defined(JPEG_LIB_VERSION_MAJOR)
  VersionNumber jpegVersion(JPEG_LIB_VERSION_MAJOR, JPEG_LIB_VERSION_MINOR, 0, 0);
#else
  VersionNumber jpegVersion(JPEG_LIB_VERSION, 0, 0, 0);
#endif
  versions.push_back(LibraryVersion("jpeg", &jpegVersion, nullptr));
}
