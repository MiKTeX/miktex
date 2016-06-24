/* mfmodes.cpp: METAFONT modes

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Paths.h"
#include "miktex/Core/StreamReader.h"

#include "Session/SessionImpl.h"
#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

struct ModeComparer
{
  bool operator() (const MIKTEXMFMODE & lhs, const MIKTEXMFMODE & rhs) const
  {
    return StringCompare(lhs.szDescription, rhs.szDescription) < 0;
  }
};

void SessionImpl::ReadMetafontModes()
{
  PathName path;

  if (!FindFile("modes.mf", FileType::MF, path))
  {
    MIKTEX_UNEXPECTED();
  }

  StreamReader reader(path);

  bool readingModeDef = false;

  string line;

  metafontModes.reserve(200);

  MIKTEXMFMODE mfmode;

  memset(&mfmode, 0, sizeof(mfmode));

  while (reader.ReadLine(line))
  {
    if (readingModeDef)
    {
      const char * lpsz;
      if (strncmp(line.c_str(), "enddef;", 7) == 0)
      {
	metafontModes.push_back(mfmode);
	readingModeDef = false;
      }
      else if (mfmode.iHorzRes == 0 && ((lpsz = strstr(line.c_str(), "pixels_per_inch")) != nullptr))
      {
	SkipNonDigit(lpsz);
	mfmode.iHorzRes = mfmode.iVertRes = atoi(lpsz);
      }
      else if ((lpsz = strstr(line.c_str(), "aspect_ratio")) != nullptr)
      {
	SkipNonDigit(lpsz);
	mfmode.iVertRes = atoi(lpsz);
      }
    }
    else if (strncmp(line.c_str(), "mode_def", 8) == 0)
    {
      const char * lpsz = line.c_str() + 8;
      while (!IsAlpha(*lpsz) && *lpsz != '\n')
      {
	++lpsz;
      }
      if (!IsAlpha(*lpsz))
      {
	continue;
      }
      const char * lpszModeName = lpsz;
      SkipAlpha(lpsz);
      if (*lpsz == 0)
      {
	continue;
      }
      *const_cast<char *>(lpsz++) = 0;
      if (Utils::Equals(lpszModeName, "help"))
      {
	continue;
      }
      lpsz = strstr(lpsz, "%\\[");
      if (lpsz == nullptr)
      {
	continue;
      }
      lpsz += 3;
      SkipSpace(lpsz);
      if (lpsz == nullptr)
      {
	continue;
      }
      const char * printer_name = lpsz;
      readingModeDef = true;
      StringUtil::CopyString(mfmode.szMnemonic, ARRAY_SIZE(mfmode.szMnemonic), lpszModeName);
      StringUtil::CopyString(mfmode.szDescription, ARRAY_SIZE(mfmode.szDescription), printer_name);
      mfmode.iHorzRes = 0;
    }
  }

  sort(metafontModes.begin(), metafontModes.end(), ModeComparer());
}

bool SessionImpl::GetMETAFONTMode(unsigned idx, MIKTEXMFMODE * pMode)
{
  MIKTEX_ASSERT_BUFFER(pMode, sizeof(*pMode));

  if (metafontModes.size() == 0)
  {
    ReadMetafontModes();
  }

  if (idx > metafontModes.size())
  {
    INVALID_ARGUMENT("index", std::to_string(idx));
  }
  else if (idx == metafontModes.size())
  {
    return false;
  }

  *pMode = metafontModes[idx];

  return true;
}

bool SessionImpl::FindMETAFONTMode(const char * lpszMnemonic, MIKTEXMFMODE * pMode)
{
  MIKTEXMFMODE candidate;

  for (unsigned long idx = 0; GetMETAFONTMode(idx, &candidate); ++idx)
  {
    if (Utils::Equals(lpszMnemonic, candidate.szMnemonic))
    {
      *pMode = candidate;
      return true;
    }
  }

  return false;
}

bool SessionImpl::DetermineMETAFONTMode(unsigned dpi, MIKTEXMFMODE * pMode)
{
  MIKTEX_ASSERT_BUFFER(pMode, sizeof(*pMode));

  const char * lpszMode = nullptr;

  // favour well known modes
  switch (dpi)
  {
  case 85:
    lpszMode = "sun";
    break;
  case 100:
    lpszMode = "nextscrn";
    break;
  case 180:
    lpszMode = "toshiba";
    break;
  case 300:
    lpszMode = "cx";
    break;
  case 400:
    lpszMode = "nexthi";
    break;
  case 600:
    lpszMode = "ljfour";
    break;
  case 1270:
    lpszMode = "linoone";
    break;
  }

  MIKTEXMFMODE candidate;

  if (lpszMode != nullptr && FindMETAFONTMode(lpszMode, &candidate) && candidate.iHorzRes == static_cast<int>(dpi))
  {
    *pMode = candidate;
    return true;
  }

  for (unsigned long i = 0; GetMETAFONTMode(i, &candidate); ++i)
  {
    if (candidate.iHorzRes == static_cast<int>(dpi))
    {
      *pMode = candidate;
      return true;
    }
  }

  return false;
}
