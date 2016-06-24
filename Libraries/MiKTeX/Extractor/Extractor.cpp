/* Extractor.cpp:

   Copyright (C) 2001-2016 Christian Schenk

   This file is part of MiKTeX Extractor.

   MiKTeX Extractor is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Extractor is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Extractor; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"

#include "internal.h"

#include "CabExtractor.h"
#include "TarBzip2Extractor.h"
#include "TarLzmaExtractor.h"

#if defined (MIKTEX_WINDOWS) && defined(ENABLE_WINDOWS_CAB_EXTRACTOR)
#  include "win/winCabExtractor.h"
#endif

using namespace MiKTeX::Extractor;
using namespace std;

Extractor::~Extractor()
{
}

unique_ptr<Extractor> Extractor::CreateExtractor(ArchiveFileType archiveFileType)
{
  switch (archiveFileType)
  {
  case ArchiveFileType::MSCab:
#if defined (MIKTEX_WINDOWS) && defined(ENABLE_WINDOWS_CAB_EXTRACTOR)
    static bool USE_WINDOWS_CABEXTRACTOR = false;
    if (USE_WINDOWS_CABEXTRACTOR)
    {
      return new winCabExtractor;
    }
#endif
    return make_unique<CabExtractor>();
  case ArchiveFileType::TarBzip2:
    return make_unique<TarBzip2Extractor>();
  case ArchiveFileType::TarLzma:
  case ArchiveFileType::TarXz:
    return make_unique<TarLzmaExtractor>();
  case ArchiveFileType::Tar:
    return make_unique<TarExtractor>();
  default:
    UNIMPLEMENTED();
  }
}
