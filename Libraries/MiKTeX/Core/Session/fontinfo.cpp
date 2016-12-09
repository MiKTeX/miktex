/* fontinfo.cpp: font information

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

/* Algorithms are borrowed from the web2c mktex* shell scripts. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/PathNameParser.h"
#include "miktex/Core/Paths.h"
#include "miktex/Core/StreamReader.h"

#include "Session/SessionImpl.h"
#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

const char * WHITESPACE = " \t\r\n";

const char * const MAP_SEARCH_PATH = MAKE_SEARCH_PATH("fontname");

MIKTEXSTATICFUNC(int) IsPrefixOf(const string & s1, const string & s2)
{
  size_t l1 = s1.length();
  return l1 <= s2.length() && strncmp(s1.c_str(), s2.c_str(), l1) == 0;
}

MIKTEXSTATICFUNC(bool) SessionImpl::FindInTypefaceMap(const string & fontName, string & typeface)
{
  const size_t FONT_ABBREV_LENGTH = 2;

  if (fontName.length() <= FONT_ABBREV_LENGTH)
  {
    return false;
  }

  // "ptmr8r" => "tm"
  string fontAbbrev = fontName.substr(1, FONT_ABBREV_LENGTH);

  PathName typefaceMap;
  if (!FindFile("typeface.map", MAP_SEARCH_PATH, typefaceMap))
  {
    MIKTEX_UNEXPECTED();
  }

  StreamReader reader(typefaceMap);

  string line;
  while (reader.ReadLine(line))
  {
    Tokenizer tok(line.c_str(), WHITESPACE);
    if (tok.GetCurrent() == nullptr || fontAbbrev != tok.GetCurrent())
    {
      continue;
    }
    ++tok;
    if (tok.GetCurrent() == nullptr)
    {
      continue;
    }
    typeface = tok.GetCurrent();
    trace_fonts->WriteFormattedLine("core", T_("found %s in typeface.map"), Q_(typeface));
    return true;
  }

  return false;
}

bool SessionImpl::FindInSupplierMap(const string & fontName, string & supplier, string & typeface)
{
  const size_t SUPPLIER_ABBREV_LENGTH = 1;

  if (fontName.length() < SUPPLIER_ABBREV_LENGTH)
  {
    return false;
  }

  // "ptmr8r" => "p"
  string supplierAbbrev = fontName.substr(0, SUPPLIER_ABBREV_LENGTH);

  PathName supplierMap;
  if (!FindFile("supplier.map", MAP_SEARCH_PATH, supplierMap))
  {
    MIKTEX_UNEXPECTED();
  }

  StreamReader reader(supplierMap);

  string line;
  bool found = false;
  while (!found && reader.ReadLine(line))
  {
    Tokenizer tok(line.c_str(), WHITESPACE);
    if (tok.GetCurrent() == nullptr ||  supplierAbbrev != tok.GetCurrent())
    {
      continue;
    }
    ++tok;
    if (tok.GetCurrent() == nullptr)
    {
      continue;
    }
    supplier = tok.GetCurrent();
    trace_fonts->WriteFormattedLine("core", T_("found %s in supplier.map"), Q_(supplier));
    found = true;
  }

  return found && FindInTypefaceMap(fontName, typeface);
}

char GetLastChar(const string & s)
{
  size_t len = s.length();
  return len < 2 ? 0 : s[len - 1];
}

bool SessionImpl::FindInSpecialMap(const string & fontName, string & supplier, string & typeface)
{
  PathName specialMap;
  if (!FindFile("special.map", MAP_SEARCH_PATH, specialMap))
  {
    MIKTEX_UNEXPECTED();
  }

  StreamReader reader(specialMap);

  string line;
  bool found = false;
  while (!found && reader.ReadLine(line))
  {
    Tokenizer tok(line.c_str(), WHITESPACE);
    if (tok.GetCurrent() == nullptr
      || !(fontName == tok.GetCurrent()
        || (IsPrefixOf(tok.GetCurrent(), fontName)
          && (IsDigit(GetLastChar(fontName)))
          && (!IsDigit(GetLastChar(tok.GetCurrent()))))))
    {
      continue;
    }
    ++tok;
    if (tok.GetCurrent() == nullptr)
    {
      continue;
    }
    supplier = tok.GetCurrent();
    ++tok;
    if (tok.GetCurrent() == nullptr)
    {
      continue;
    }
    typeface = tok.GetCurrent();
    trace_fonts->WriteFormattedLine("core", T_("found %s/%s in special.map"), Q_(supplier), Q_(typeface));
    return true;
  }

  return false;
}

bool SessionImpl::InternalGetFontInfo(const string & fontName, string & supplier, string & typeface)
{
  return FindInSpecialMap(fontName, supplier, typeface) || FindInSupplierMap(fontName, supplier, typeface);
}

void SessionImpl::SplitFontPath(const PathName & fontPath, string * fontType, string * supplier, string * typeface, string * fontName, string * pointSize)
{
  if (supplier != nullptr || typeface != nullptr)
  {
    if (fontType != nullptr)
    {
      *fontType = "";
    }
    if (supplier != nullptr)
    {
      *supplier = "";
    }
    if (typeface != nullptr)
    {
      *typeface = "";
    }
    if (Utils::IsAbsolutePath(fontPath.GetData()))
    {
      PathName root;
      PathName pathRel;
      if (SplitTEXMFPath(fontPath, root, pathRel) != INVALID_ROOT_INDEX)
      {
        const char * d1, * d2;
        PathNameParser tok(pathRel.GetData());
        if (((d1 = tok.GetCurrent()) != nullptr) && PathName::Compare(d1, "fonts") == 0 && ((d2 = ++tok) != nullptr))
        {
          if (fontType != nullptr)
          {
            *fontType = d2;
          }
          const char * d3, * d4;
          if (((d3 = ++tok) != nullptr) && ((d4 = ++tok) != nullptr))
          {
            if (supplier != nullptr)
            {
              *supplier = d3;
            }
            if (typeface != nullptr)
            {
              *typeface = d4;
            }
          }
        }
      }
    }
  }

  if (fontName != nullptr || pointSize != nullptr)
  {
    PathName fileName = fontPath.GetFileNameWithoutExtension();
    char * lpsz = fileName.GetData() + fileName.GetLength() - 1;
    while (IsDigit(*lpsz))
    {
      --lpsz;
    }
    ++lpsz;
    if (lpsz != fileName.GetData())
    {
      if (pointSize != nullptr)
      {
        *pointSize = lpsz;
      }
      *lpsz = 0;
    }
    else if (pointSize != nullptr)
    {
      *pointSize = "";
    }
    if (fontName != nullptr)
    {
      *fontName = fileName.ToString();
    }
  }
}

int Comp2(const string & s1, const string & s2)
{
  MIKTEX_ASSERT(s1.length() >= 2 && s2.length() == 2);
  return ToLower(s1[0]) == ToLower(s2[0]) && ToLower(s1[1]) == ToLower(s2[1]);
}

bool SessionImpl::GetFontInfo(const string & fontName, string & supplier, string & typeface, double * genSize)
{
  PathName pathFileName;

  // test TFM first
  bool found = FindFile(fontName.c_str(), FileType::OFM, pathFileName);

  // then MF
  if (!found)
  {
    found = FindFile(fontName.c_str(), FileType::MF, pathFileName);
  }

  // then possible sauterized MF
  if (!found)
  {
    string fontNameSauter("b-");
    fontNameSauter += fontName;
    found = FindFile(fontNameSauter.c_str(), FileType::MF, pathFileName);
  }

  // LH fonts get special treatment
  if (!found
    && ((Comp2(fontName, "wn") == 0)
      || (Comp2(fontName, "lh") == 0)
      || (Comp2(fontName, "ll") == 0)
      || (Comp2(fontName, "rx") == 0)
      || (Comp2(fontName, "la") == 0)
      || (Comp2(fontName, "lb") == 0)
      || (Comp2(fontName, "lc") == 0)))
  {
    string fontNameLH;
    fontNameLH = fontName[0];
    fontNameLH += fontName[1];
    fontNameLH += "codes";
    found = FindFile(fontNameLH.c_str(), FileType::MF, pathFileName);
  }

  // parse the path, if the font was found
  if (found && IsTEXMFFile(pathFileName))
  {
    SplitFontPath(pathFileName, nullptr, &supplier, &typeface, nullptr, nullptr);
  }
  else
  {
    // consult the font maps
    if (!InternalGetFontInfo(fontName, supplier, typeface))
    {
      return false;
    }
  }

  // determine the point size
  if (genSize != nullptr)
  {
    string pointSize;
    SplitFontPath(fontName, nullptr, nullptr, nullptr, nullptr, &pointSize);
    size_t l = pointSize.length();
    if (l == 0)
    {
      return false;
    }
    else if (pointSize == "11")
    {
      *genSize = 10.95;   // \magstephalf
    }
    else if (pointSize == "14")
    {
      *genSize = 14.4;    // \magstep2
    }
    else if (pointSize == "17")
    {
      *genSize = 17.28;   // \magstep3
    }
    else if (pointSize == "20")
    {
      *genSize = 20.74;   // \magstep4
    }
    else if (pointSize == "25")
    {
      *genSize = 24.88;   // \magstep5;
    }
    else if (pointSize == "30")
    {
      *genSize = 29.86;   // \magstep6
    }
    else if (pointSize == "36")
    {
      *genSize = 35.83;   // \magstep7
    }
    else if (l == 4 || l == 5)
    {
      /* The new convention is to have three or four letters for
	 the font name and four digits for the pointsize. The
	 number is pointsize * 100. We effectively divide by 100
	 by ignoring the last two digits. */
      *genSize = std::stoi(pointSize) / 100.0;
    }
    else
    {
      *genSize = std::stoi(pointSize);
    }
  }

  return true;
}

string SessionImpl::GetLocalFontDirectories()
{
  if (!flags.test((size_t)InternalFlag::CachedLocalFontDirs))
  {
    flags.set((size_t)InternalFlag::CachedLocalFontDirs);
#if defined(MIKTEX_WINDOWS)
    PathName winFontDir;
    if (GetWindowsFontsDirectory(winFontDir))
    {
      if (!localFontDirs.empty())
      {
	localFontDirs += PathName::PathNameDelimiter;
      }
      localFontDirs += winFontDir.GetData();
    }
    PathName atmFontDir;
    if (GetATMFontDir(atmFontDir))
    {
      if (!localFontDirs.empty())
      {
	localFontDirs += PathName::PathNameDelimiter;
      }
      localFontDirs += atmFontDir.GetData();
    }
    PathName acrobatFontDir;
    if (GetAcrobatFontDir(acrobatFontDir))
    {
      if (!localFontDirs.empty())
      {
	localFontDirs += PathName::PathNameDelimiter;
      }
      localFontDirs += acrobatFontDir.GetData();
    }
#endif
  }

  return localFontDirs;
}
