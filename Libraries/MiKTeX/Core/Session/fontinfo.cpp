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

MIKTEXSTATICFUNC(int) IsPrefixOf(const char * lpsz1, const char * lpsz2)
{
  size_t l1 = strlen(lpsz1);
  return l1 <= strlen(lpsz2) && strncmp(lpsz1, lpsz2, l1) == 0;
}

MIKTEXSTATICFUNC(bool) SessionImpl::FindInTypefaceMap(const char * lpszFontName, char * lpszTypeface)
{
  const size_t FONT_ABBREV_LENGTH = 2;

  if (strlen(lpszFontName) <= FONT_ABBREV_LENGTH)
  {
    return false;
  }

  // "ptmr8r" => "tm"
  char szFontAbbrev[FONT_ABBREV_LENGTH + 1];
  CopyString2(szFontAbbrev, FONT_ABBREV_LENGTH + 1, lpszFontName + 1, FONT_ABBREV_LENGTH);

  PathName pathFileName;
  if (!FindFile("typeface.map", MAP_SEARCH_PATH, pathFileName))
  {
    MIKTEX_UNEXPECTED();
  }

  StreamReader reader(pathFileName);

  string line;
  while (reader.ReadLine(line))
  {
    Tokenizer tok(line.c_str(), WHITESPACE);
    if (tok.GetCurrent() == nullptr || StringCompare(tok.GetCurrent(), szFontAbbrev) != 0)
    {
      continue;
    }
    ++tok;
    if (tok.GetCurrent() == nullptr || strlen(tok.GetCurrent()) >= BufferSizes::MaxPath)
    {
      continue;
    }
    StringUtil::CopyString(lpszTypeface, BufferSizes::MaxPath, tok.GetCurrent());
    trace_fonts->WriteFormattedLine("core", T_("found %s in typeface.map"), Q_(lpszTypeface));
    return true;
  }

  return false;
}

bool SessionImpl::FindInSupplierMap(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface)
{
  const size_t SUPPLIER_ABBREV_LENGTH = 1;

  if (strlen(lpszFontName) < SUPPLIER_ABBREV_LENGTH)
  {
    return false;
  }

  // "ptmr8r" => "p"
  char szSupplierAbbrev[SUPPLIER_ABBREV_LENGTH + 1];
  CopyString2(szSupplierAbbrev, SUPPLIER_ABBREV_LENGTH + 1, lpszFontName, SUPPLIER_ABBREV_LENGTH);

  PathName pathFileName;
  if (!FindFile("supplier.map", MAP_SEARCH_PATH, pathFileName))
  {
    MIKTEX_UNEXPECTED();
  }

  StreamReader reader(pathFileName);

  string line;
  bool bFound = false;
  while (!bFound && reader.ReadLine(line))
  {
    Tokenizer tok(line.c_str(), WHITESPACE);
    if (tok.GetCurrent() == nullptr || StringCompare(tok.GetCurrent(), szSupplierAbbrev) != 0)
    {
      continue;
    }
    ++tok;
    if (tok.GetCurrent() == nullptr || strlen(tok.GetCurrent()) >= BufferSizes::MaxPath)
    {
      continue;
    }
    StringUtil::CopyString(lpszSupplier, BufferSizes::MaxPath, tok.GetCurrent());
    trace_fonts->WriteFormattedLine("core", T_("found %s in supplier.map"), Q_(lpszSupplier));
    bFound = true;
  }

  return bFound && FindInTypefaceMap(lpszFontName, lpszTypeface);
}

inline char GetLastChar(const char * lpsz)
{
  MIKTEX_ASSERT(lpsz != 0);
  size_t len = StrLen(lpsz);
  return len < 2 ? 0 : lpsz[len - 1];
}

bool SessionImpl::FindInSpecialMap(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface)
{
  PathName pathFileName;
  if (!FindFile("special.map", MAP_SEARCH_PATH, pathFileName))
  {
    MIKTEX_UNEXPECTED();
  }

  StreamReader reader(pathFileName);

  string line;
  bool bFound = false;
  while (!bFound && reader.ReadLine(line))
  {
    Tokenizer tok(line.c_str(), WHITESPACE);
    if (tok.GetCurrent() == nullptr
      || !(Utils::Equals(tok.GetCurrent(), lpszFontName)
	|| (IsPrefixOf(tok.GetCurrent(), lpszFontName)
	  && (IsDigit(GetLastChar(lpszFontName)))
	  && (!IsDigit(GetLastChar(tok.GetCurrent()))))))
    {
      continue;
    }
    ++tok;
    if (tok.GetCurrent() == nullptr || strlen(tok.GetCurrent()) >= BufferSizes::MaxPath)
    {
      continue;
    }
    StringUtil::CopyString(lpszSupplier, BufferSizes::MaxPath, tok.GetCurrent());
    ++tok;
    if (tok.GetCurrent() == nullptr || strlen(tok.GetCurrent()) >= BufferSizes::MaxPath)
    {
      continue;
    }
    StringUtil::CopyString(lpszTypeface, BufferSizes::MaxPath, tok.GetCurrent());
    trace_fonts->WriteFormattedLine("core", T_("found %s/%s in special.map"), Q_(lpszSupplier), Q_(lpszTypeface));
    return true;
  }

  return false;
}

bool SessionImpl::InternalGetFontInfo(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface)
{
  return (FindInSpecialMap(lpszFontName, lpszSupplier, lpszTypeface)
    || FindInSupplierMap(lpszFontName, lpszSupplier, lpszTypeface));
}

void SessionImpl::SplitFontPath(const char * lpszFontPath, char * lpszFontType, char * lpszSupplier, char * lpszTypeface, char * lpszFontName, char * lpszPointSize)
{
  MIKTEX_ASSERT_STRING(lpszFontPath);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpszFontType);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpszSupplier);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpszTypeface);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpszFontName);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpszPointSize);

  if (lpszSupplier != nullptr || lpszTypeface != nullptr)
  {
    if (lpszFontType != nullptr)
    {
      *lpszFontType = 0;
    }
    if (lpszSupplier != nullptr)
    {
      *lpszSupplier = 0;
    }
    if (lpszTypeface != nullptr)
    {
      *lpszTypeface = 0;
    }
    if (Utils::IsAbsolutePath(lpszFontPath))
    {
      PathName root;
      PathName pathRel;
      if (SplitTEXMFPath(lpszFontPath, root, pathRel) != INVALID_ROOT_INDEX)
      {
	const char *d1, *d2;
	PathNameParser tok(pathRel.Get());
	if (((d1 = tok.GetCurrent()) != nullptr) && PathName::Compare(d1, "fonts") == 0 && ((d2 = ++tok) != nullptr))
	{
	  if (lpszFontType != nullptr)
	  {
	    StringUtil::CopyString(lpszFontType, BufferSizes::MaxPath, d2);
	  }
	  const char *d3, *d4;
	  if (((d3 = ++tok) != nullptr) && ((d4 = ++tok) != nullptr))
	  {
	    if (lpszSupplier != nullptr)
	    {
	      StringUtil::CopyString(lpszSupplier, BufferSizes::MaxPath, d3);
	    }
	    if (lpszTypeface != nullptr)
	    {
	      StringUtil::CopyString(lpszTypeface, BufferSizes::MaxPath, d4);
	    }
	  }
	}
      }
    }
  }

  if (lpszFontName != nullptr || lpszPointSize != nullptr)
  {
    char szFileName[BufferSizes::MaxPath];
    PathName::Split(lpszFontPath, 0, 0, szFileName, BufferSizes::MaxPath, 0, 0);
    char * lpsz = szFileName + strlen(szFileName) - 1;
    while (IsDigit(*lpsz))
    {
      --lpsz;
    }
    ++lpsz;
    if (lpsz != szFileName)
    {
      if (lpszPointSize != nullptr)
      {
	StringUtil::CopyString(lpszPointSize, BufferSizes::MaxPath, lpsz);
      }
      *lpsz = 0;
    }
    else if (lpszPointSize != nullptr)
    {
      *lpszPointSize = 0;
    }
    if (lpszFontName != nullptr)
    {
      StringUtil::CopyString(lpszFontName, BufferSizes::MaxPath, szFileName);
    }
  }
}

inline int Comp2(const char * lpsz1, const char * lpsz2)
{
  MIKTEX_ASSERT(strlen(lpsz2) == 2);
  return ToLower(lpsz1[0]) == ToLower(lpsz2[0]) && ToLower(lpsz1[1]) == ToLower(lpsz2[1]);
}

bool SessionImpl::GetFontInfo(const char * lpszFontName, char * lpszSupplier, char * lpszTypeface, double * lpGenSize)
{
  MIKTEX_ASSERT_STRING(lpszFontName);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpszSupplier);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpszTypeface);
  MIKTEX_ASSERT_PATH_BUFFER_OR_NIL(lpGenSize);

  PathName pathFileName;

  // test TFM first
  bool bFound = FindFile(lpszFontName, FileType::OFM, pathFileName);

  // then MF
  if (!bFound)
  {
    bFound = FindFile(lpszFontName, FileType::MF, pathFileName);
  }

  // then possible sauterized MF
  if (!bFound)
  {
    string strFontNameSauter("b-");
    strFontNameSauter += lpszFontName;
    bFound = FindFile(strFontNameSauter.c_str(), FileType::MF, pathFileName);
  }

  // LH fonts get special treatment
  if (!bFound
    && ((Comp2(lpszFontName, "wn") == 0)
      || (Comp2(lpszFontName, "lh") == 0)
      || (Comp2(lpszFontName, "ll") == 0)
      || (Comp2(lpszFontName, "rx") == 0)
      || (Comp2(lpszFontName, "la") == 0)
      || (Comp2(lpszFontName, "lb") == 0)
      || (Comp2(lpszFontName, "lc") == 0)))
  {
    string strFontNameLH;
    strFontNameLH = lpszFontName[0];
    strFontNameLH += lpszFontName[1];
    strFontNameLH += "codes";
    bFound = FindFile(strFontNameLH.c_str(), FileType::MF, pathFileName);
  }

  // parse the path, if the font was found
  if (bFound && IsTEXMFFile(pathFileName))
  {
    SplitFontPath(pathFileName.Get(), nullptr, lpszSupplier, lpszTypeface, nullptr, nullptr);
  }
  else
  {
    // consult the font maps
    if (!InternalGetFontInfo(lpszFontName, lpszSupplier, lpszTypeface))
    {
      return false;
    }
  }

  // determine the point size
  if (lpGenSize != 0)
  {
    char ptsize[BufferSizes::MaxPath];
    SplitFontPath(lpszFontName, nullptr, nullptr, nullptr, nullptr, ptsize);
    size_t l = strlen(ptsize);
    if (l == 0)
    {
      return false;
    }
    else if (Utils::Equals(ptsize, "11"))
    {
      *lpGenSize = 10.95;   // \magstephalf
    }
    else if (Utils::Equals(ptsize, "14"))
    {
      *lpGenSize = 14.4;    // \magstep2
    }
    else if (Utils::Equals(ptsize, "17"))
    {
      *lpGenSize = 17.28;   // \magstep3
    }
    else if (Utils::Equals(ptsize, "20"))
    {
      *lpGenSize = 20.74;   // \magstep4
    }
    else if (Utils::Equals(ptsize, "25"))
    {
      *lpGenSize = 24.88;   // \magstep5;
    }
    else if (Utils::Equals(ptsize, "30"))
    {
      *lpGenSize = 29.86;   // \magstep6
    }
    else if (Utils::Equals(ptsize, "36"))
    {
      *lpGenSize = 35.83;   // \magstep7
    }
    else if (l == 4 || l == 5)
    {
      /* The new convention is to have three or four letters for
	 the font name and four digits for the pointsize. The
	 number is pointsize * 100. We effectively divide by 100
	 by ignoring the last two digits. */
      *lpGenSize = atoi(ptsize) / 100.0;
    }
    else
    {
      *lpGenSize = atoi(ptsize);
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
      localFontDirs += winFontDir.Get();
    }
    PathName atmFontDir;
    if (GetATMFontDir(atmFontDir))
    {
      if (!localFontDirs.empty())
      {
	localFontDirs += PathName::PathNameDelimiter;
      }
      localFontDirs += atmFontDir.Get();
    }
    PathName acrobatFontDir;
    if (GetAcrobatFontDir(acrobatFontDir))
    {
      if (!localFontDirs.empty())
      {
	localFontDirs += PathName::PathNameDelimiter;
      }
      localFontDirs += acrobatFontDir.Get();
    }
#endif
  }

  return localFontDirs;
}
