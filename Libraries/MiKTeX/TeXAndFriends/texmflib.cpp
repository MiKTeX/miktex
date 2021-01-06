/* texmflib.cpp: TeX'n'Friends helpers

   Copyright (C) 1996-2021 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include <miktex/Core/Paths>
#include <miktex/Core/StreamReader>

#if defined(MIKTEX_TEXMF_SHARED)
#  define C4PEXPORT MIKTEXDLLEXPORT
#else
#  define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#if defined(MIKTEX_TEXMF_SHARED)
#  define MIKTEXMFEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXMFEXPORT
#endif
#define B8C7815676699B4EA2DE96F0BD727276
#include "miktex/TeXAndFriends/TeXMFApp.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

typedef C4P_FILE_STRUCT(unsigned char) bytefile;
typedef C4P::C4P_text alphafile;

bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenTFMFile(void* p, const PathName& fileName)
{
  MIKTEX_API_BEGIN("OpenTFMFile");
  MIKTEX_ASSERT_BUFFER(p, sizeof(bytefile));
  return TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(p), fileName.GetData(), FileType::TFM, MIKTEX_MAKETFM_EXE);
  MIKTEX_API_END("OpenTFMFile");
}

int MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenXFMFile(void* ptr, const PathName& fileName)
{
  MIKTEX_API_BEGIN("OpenXFMFile");
  MIKTEX_ASSERT_BUFFER(ptr, sizeof(bytefile));
  if (TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::TFM, MIKTEX_MAKETFM_EXE))
  {
    return 1;
  }
  if (TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::OFM, MIKTEX_MAKETFM_EXE))
  {
    return 2;
  }
  return 0;
  MIKTEX_API_END("OpenXFMFile");
}

bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenVFFile(void* ptr, const PathName& fileName)
{
  MIKTEX_API_BEGIN("OpenVFFile");
  MIKTEX_ASSERT_BUFFER(ptr, sizeof(bytefile));
  return TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::VF, nullptr);
  MIKTEX_API_END("OpenVFFile");
}

int MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenXVFFile(void* ptr, const PathName& fileName)
{
  MIKTEX_API_BEGIN("OpenXVFFile");
  MIKTEX_ASSERT_BUFFER(ptr, sizeof(bytefile));
  return TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::VF, nullptr)
    || TeXMFApp::GetTeXMFApp()->OpenFontFile(reinterpret_cast<bytefile*>(ptr), fileName.GetData(), FileType::OVF, nullptr);
  MIKTEX_API_END("OpenXVFFile");
}

STATICFUNC(bool) ProcessTCXFile(const char* lpszFileName, unsigned char* pChr, unsigned char* pOrd, unsigned char* pPrn)
{
  PathName tcxPath;

  shared_ptr<Session> session = Session::Get();

  if (!session->FindFile(lpszFileName, FileType::TCX, tcxPath))
  {
    return false;
  }

  StreamReader reader(tcxPath);

  string line;
  int lineNumber = 0;

  while (reader.ReadLine(line))
  {
    ++lineNumber;

    const char* start;
    char* end;

    if (line.empty() || line[0] == '%')
    {
      continue;
    }

    start = line.c_str();

    if (start == nullptr)
    {
      MIKTEX_FATAL_ERROR_2("Invalid tcx file.", "tcxPath", tcxPath.ToString());
    }

    // get xord index (src)
    long xordidx = strtol(start, &end, 0);
    if (start == end)
    {
      MIKTEX_FATAL_ERROR_2("Invalid tcx file.", "tcxPath", tcxPath.ToString());
    }
    else if (xordidx < 0 || xordidx > 255)
    {
      MIKTEX_FATAL_ERROR_2("Invalid tcx file.", "tcxPath", tcxPath.ToString());
    }

    // make the char printable by default
    long printable = 1;

    // get optional xchr index (dest)
    start = end;
    long xchridx = strtol(start, &end, 0);
    if (start == end)
    {
      // xchridx not specified; make it the same as xordidx
      xchridx = xordidx;
    }
    else if (xchridx < 0 || xchridx > 255)
    {
      MIKTEX_FATAL_ERROR_2("Invalid tcx file.", "tcxPath", tcxPath.ToString());
    }
    else
    {
      // get optional printable flag
      start = end;
      printable = strtol(start, &end, 0);
      if (start == end)
      {
        // not specified; by default printable
        printable = 1;
      }
      else if (printable < 0 || printable > 1)
      {
        MIKTEX_FATAL_ERROR_2("Invalid tcx file.", "tcxPath", tcxPath.ToString());
      }
    }

    // don't allow the 7bit ASCII set to become unprintable.
    if (printable == 0 && xordidx >= ' ' && xordidx <= '~')
    {
      printable = 1;
    }

    // update the char tables
    pOrd[xordidx] = static_cast<unsigned char>(xchridx);
    pChr[xchridx] = static_cast<unsigned char>(xordidx);
    if (pPrn != 0)
    {
      pPrn[xchridx] = static_cast<unsigned char>(printable);
    }
  }

  reader.Close();

  return true;
}

bool MIKTEXCEECALL MiKTeX::TeXAndFriends::InitializeCharTables(unsigned long flags, const PathName& fileName, char* pChr, char* pOrd, char* pPrn)
{
  MIKTEX_API_BEGIN("InitializeCharTables");
  MIKTEX_ASSERT_BUFFER(pChr, 256);
  MIKTEX_ASSERT_BUFFER(pOrd, 256);
  MIKTEX_ASSERT_BUFFER_OR_NIL(pPrn, 256);
  unsigned char* pxchr = reinterpret_cast<unsigned char*>(pChr);
  unsigned char* pxord = reinterpret_cast<unsigned char*>(pOrd);
  unsigned char* pxprn = reinterpret_cast<unsigned char*>(pPrn);
  for (unsigned idx = 0; idx < 256; ++idx)
  {
    pxchr[idx] = static_cast<unsigned char>(idx);
    pxord[idx] = static_cast<unsigned char>(idx);
    if (pxprn != nullptr)
    {
      if ((flags & ICT_TCX) != 0)
      {
        pxprn[idx] = 1;
      }
      else
      {
        if (idx >= ' ' && idx <= '~')
        {
          pxprn[idx] = 1;
        }
        else
        {
          pxprn[idx] = 0;
        }
      }
    }
  }
  if ((flags & ICT_TCX) != 0)
  {
    ProcessTCXFile(fileName.GetData(), pxchr, pxord, pxprn);
  }
  return true;
  MIKTEX_API_END("InitializeCharTables");
}

STATICFUNC(bool) OpenAlphaFile(void* p, const char* lpszFileName, FileType fileType, const char* lpszExtension)
{
  MIKTEX_ASSERT(p != nullptr);
  MIKTEX_ASSERT_BUFFER(p, sizeof(alphafile));
  MIKTEX_ASSERT_STRING(lpszFileName);
  shared_ptr<Session> session = Session::Get();
  PathName fileName(lpszFileName);
  if (!fileName.HasExtension() && lpszExtension != nullptr)
  {
    fileName.SetExtension(lpszExtension);
  }
  PathName path;
  if (!session->FindFile(fileName.ToString(), fileType, path))
  {
    return false;
  }
  FILE* pfile = session->TryOpenFile(path, FileMode::Open, FileAccess::Read, false);
  if (pfile == nullptr)
  {
    return false;
  }
  reinterpret_cast<alphafile*>(p)->Attach(pfile, true);
  reinterpret_cast<alphafile*>(p)->Read();
  return true;
}

#if 0
bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenMAPFile(void* p, const char* lpszFileName)
{
  MIKTEX_API_BEGIN("OpenMAPFile");
  return OpenAlphaFile(p, lpszFileName, FileType::MAP, ".map");
  MIKTEX_API_END("OpenMAPFile");
}

bool MIKTEXCEECALL MiKTeX::TeXAndFriends::OpenMETAFONTFile(void* p, const char* lpszFileName)
{
  MIKTEX_API_BEGIN("OpenMETAFONTFile");
  return OpenAlphaFile(p, lpszFileName, FileType::MF, ".mf");
  MIKTEX_API_END("OpenMETAFONTFile");
}
#endif