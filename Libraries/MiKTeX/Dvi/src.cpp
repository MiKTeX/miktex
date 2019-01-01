/* src.cpp: src specials

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of the MiKTeX DVI Library.

   The MiKTeX DVI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX DVI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the MiKTeX DVI Library; if not, write to the
   Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
   USA.  */

#include "config.h"

#include "internal.h"

STATICFUNC(int) MyPathNameCompare(const PathName& path1, const PathName& path2)
{
  int ret = PathName::Compare(path1, path2);

  if (ret == 0)
  {
    return 0;
  }

  string extension1 = path1.GetExtension();
  string extension2 = path2.GetExtension();

  if (extension1.empty() == extension2.empty())
  {
    // both have no extenion or both have an extension
    return ret;
  }

  PathName path11(path1);
  PathName path22(path2);

  if (extension1.empty())
  {
    path11.AppendExtension(extension2);
  }
  else
  {
    MIKTEX_ASSERT(!path22.HasExtension());
    path22.AppendExtension(extension1);
  }

  return PathName::Compare(path11, path22);
}

bool DviImpl::FindSource(const char* fileName, int line, DviPosition& position)
{
  CheckCondition();

  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    trace_search->WriteFormattedLine("libdvi", T_("searching src special %d %s"), line, fileName);

    SourceSpecial* pSourceSpecial1Best = nullptr;
    SourceSpecial* pSourceSpecial2Best = nullptr;

    int pageIdx1 = -1;
    int pageIdx2 = -1;

    // get fully qualified path to the directory location of the
    // document
    PathName documentLocation(fqDviFileName);
    documentLocation.RemoveFileSpec();

    // file name relative to the location of the DVI document
    const char* lpszRelFileName;

    // absolute file name
    PathName fqFileName;

    if (Utils::IsAbsolutePath(fileName))
    {
      lpszRelFileName = Utils::GetRelativizedPath(fileName, documentLocation.GetData());
      fqFileName = fileName;
      fqFileName.MakeAbsolute();
    }
    else
    {
      lpszRelFileName = fileName;
      fqFileName = documentLocation;
      fqFileName /= fileName;
      fqFileName.MakeAbsolute();
    }

    //
    // scan the document
    //

    for (int pageIdx = 0; pageIdx < GetNumberOfPages(); ++pageIdx)
    {
      DviPageImpl* dviPage;

      try
      {
        dviPage = reinterpret_cast<DviPageImpl*>(GetLoadedPage(pageIdx));
      }
      catch (const OperationCancelledException&)
      {
        return false;
      }

      if (dviPage == nullptr)
      {
        throw DviPageNotFoundException("", T_("The DVI page could not be found."), MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION());
      }

      AutoUnlockPage autoUnlockPage(dviPage);

      SourceSpecial* pSourceSpecial1 = nullptr;
      SourceSpecial* pSourceSpecial2 = nullptr;

      SourceSpecial* pSourceSpecial;

      for (int j = -1; (pSourceSpecial = dviPage->GetNextSpecial<SourceSpecial>(j)) != nullptr; )
      {
        const char* name = pSourceSpecial->GetFileName();

        // try exact match
        bool nameMatch = (MyPathNameCompare(name, fileName) == 0);

        // try fully qualified file names
        if (!nameMatch)
        {
          PathName fqName;
          if (Utils::IsAbsolutePath(name))
          {
            fqName = name;
            fqName.MakeAbsolute();
          }
          else
          {
            fqName = documentLocation;
            fqName /= name;
            fqName.MakeAbsolute();
          }
          nameMatch = (MyPathNameCompare(fqName, fqFileName) == 0);
        }

        // try relative file names
        if (!nameMatch && lpszRelFileName != nullptr)
        {
          const char* lpszRelName;
          if (!Utils::IsAbsolutePath(name))
          {
            lpszRelName = name;
          }
          else
          {
            lpszRelName = Utils::GetRelativizedPath(name, documentLocation.GetData());
          }
          nameMatch = lpszRelName != nullptr && MyPathNameCompare(lpszRelName, lpszRelFileName) == 0;
        }

        if (!nameMatch)
        {
          continue;
        }

        if (line >= pSourceSpecial->GetLineNum())
        {
          if (pSourceSpecial1 == nullptr || (pSourceSpecial->GetLineNum() > pSourceSpecial1->GetLineNum()))
          {
            pSourceSpecial1 = pSourceSpecial;
          }
        }

        if (line <= pSourceSpecial->GetLineNum())
        {
          if (pSourceSpecial2 == nullptr || (pSourceSpecial->GetLineNum() < pSourceSpecial2->GetLineNum()))
          {
            pSourceSpecial2 = pSourceSpecial;
          }
        }
      }

      if (pSourceSpecial1 != nullptr && pSourceSpecial2 != nullptr)
      {
        pSourceSpecial1Best = pSourceSpecial1;
        pageIdx1 = pageIdx;
        pSourceSpecial2Best = pSourceSpecial2;
        pageIdx2 = pageIdx;
        break;
      }
      else if (pSourceSpecial1 != nullptr
        && (pSourceSpecial1Best == nullptr || (abs(pSourceSpecial1Best->GetLineNum() - line) > abs(pSourceSpecial1->GetLineNum() - line))))
      {
        pSourceSpecial1Best = pSourceSpecial1;
        pageIdx1 = pageIdx;
        pSourceSpecial2Best = nullptr;
        pageIdx2 = -1;
      }
      else if (pSourceSpecial2 != nullptr && (pSourceSpecial2Best == nullptr || (abs(pSourceSpecial2Best->GetLineNum() - line) > abs(pSourceSpecial2->GetLineNum() - line))))
      {
        pSourceSpecial2Best = pSourceSpecial2;
        pageIdx2 = pageIdx;
        pSourceSpecial1Best = nullptr;
        pageIdx1 = -1;
      }
    }

    //
    // evaluate the results
    //

    if (pSourceSpecial1Best == nullptr && pSourceSpecial2Best == nullptr)
    {
      trace_search->WriteLine("libdvi", T_("search failed"));
      return false;
    }

    if (pSourceSpecial1Best == nullptr)
    {
      trace_search->WriteFormattedLine("libdvi", T_("found src2 on page #%d"), pageIdx2);
      trace_search->WriteFormattedLine("libdvi", "   src2 = [%d (%d,%d)]", pSourceSpecial2Best->GetLineNum(), pSourceSpecial2Best->GetX(), pSourceSpecial2Best->GetY());
      position.pageIdx = pageIdx2;
      position.x = pSourceSpecial2Best->GetX();
      position.y = pSourceSpecial2Best->GetY();
    }
    else if (pSourceSpecial2Best == nullptr)
    {
      trace_search->WriteFormattedLine("libdvi", T_("found src1 on page #%d"), pageIdx1);
      trace_search->WriteFormattedLine("libdvi", "   src1 = [%d (%d,%d)]", pSourceSpecial1Best->GetLineNum(), pSourceSpecial1Best->GetX(), pSourceSpecial1Best->GetY());
      position.pageIdx = pageIdx1;
      position.x = pSourceSpecial1Best->GetX();
      position.y = pSourceSpecial1Best->GetY();
    }
    else
    {
      position.pageIdx = pageIdx1;
      trace_search->WriteFormattedLine("libdvi", T_("found src region on page #%d"), position.pageIdx);
      trace_search->WriteFormattedLine("libdvi", "   src1 = [%d (%d,%d)]", pSourceSpecial1Best->GetLineNum(), pSourceSpecial1Best->GetX(), pSourceSpecial1Best->GetY());
      trace_search->WriteFormattedLine("libdvi", "   src2 = [%d (%d,%d)]", pSourceSpecial2Best->GetLineNum(), pSourceSpecial2Best->GetX(), pSourceSpecial2Best->GetY());
      position.x = pSourceSpecial1Best->GetX();
      if (pSourceSpecial2Best->GetLineNum() == pSourceSpecial1Best->GetLineNum())
      {
        position.y = pSourceSpecial1Best->GetY();
      }
      else
      {
        position.y = (pSourceSpecial1Best->GetY()
          + ((line - pSourceSpecial1Best->GetLineNum())
            * (pSourceSpecial2Best->GetY()
              - pSourceSpecial1Best->GetY())
            / (pSourceSpecial2Best->GetLineNum()
              - pSourceSpecial1Best->GetLineNum())));
      }
      trace_search->WriteFormattedLine("libdvi", "   interpolated (x,y) = (%d,%d)", position.x, position.y);
    }

    return true;
  }
  END_CRITICAL_SECTION();
}

#define OFFSET(x, y) ((y) * PixelRound(maxH) + (x))

bool DviImpl::GetSource(const DviPosition& pos, PathName& fileName, int* pLineNum)
{
  CheckCondition();
  BEGIN_CRITICAL_SECTION(dviMutex)
  {
    if (pos.pageIdx >= GetNumberOfPages())
    {
      return false;
    }

    DviPageImpl* dviPage = reinterpret_cast<DviPageImpl*>(GetLoadedPage(pos.pageIdx));

    if (dviPage == nullptr)
    {
      throw DviPageNotFoundException("", T_("The DVI page could not be found."), MiKTeXException::KVMAP(), MIKTEX_SOURCE_LOCATION());
    }

    AutoUnlockPage autoUnlockPage(dviPage);

    SourceSpecial* pSourceSpecial1 = nullptr;
    SourceSpecial* pSourceSpecial2 = nullptr;

    int cursorOffset = OFFSET(pos.x, pos.y);

    int src1Offset = 0;
    int src2Offset = 0;

    SourceSpecial* pSourceSpecial;

    // find two adjacent src specials
    for (int j = -1; (pSourceSpecial = dviPage->GetNextSpecial<SourceSpecial>(j)) != nullptr; )
    {
      int srcOffset = OFFSET(pSourceSpecial->GetX(), pSourceSpecial->GetY());
      if (cursorOffset > srcOffset)
      {
        if (pSourceSpecial1 == nullptr || srcOffset > src1Offset)
        {
          pSourceSpecial1 = pSourceSpecial;
          src1Offset = srcOffset;
        }
      }
      else if (cursorOffset < srcOffset)
      {
        if (pSourceSpecial2 == nullptr || srcOffset < src2Offset)
        {
          pSourceSpecial2 = pSourceSpecial;
          src2Offset = srcOffset;
        }
      }
    }

    if (pSourceSpecial1 == nullptr && pSourceSpecial2 == nullptr)
    {
      return false;
    }

    int lineDelta = 0;

    if (pSourceSpecial1 == nullptr)
    {
      pSourceSpecial = pSourceSpecial2;
    }
    else if (pSourceSpecial2 == nullptr)
    {
      pSourceSpecial = pSourceSpecial1;
    }
    else if (MyPathNameCompare(pSourceSpecial1->GetFileName(), pSourceSpecial2->GetFileName()) != 0)
    {
      if (cursorOffset - src1Offset < src2Offset - cursorOffset)
      {
        pSourceSpecial = pSourceSpecial1;
      }
      else
      {
        pSourceSpecial = pSourceSpecial2;
      }
    }
    else
    {
      int d = src2Offset - src1Offset;
      if (d != 0)
      {
        int d1 = cursorOffset - src1Offset;
        int linesbetween
          = (pSourceSpecial2->GetLineNum()
            - pSourceSpecial1->GetLineNum()
            + 1);
        lineDelta = (d1 * linesbetween) / d;
      }
      pSourceSpecial = pSourceSpecial1;
    }
    fileName = pSourceSpecial->GetFileName();
    if (pLineNum != nullptr)
    {
      *pLineNum = pSourceSpecial->GetLineNum() + lineDelta;
    }

    return true;
  }
  END_CRITICAL_SECTION();
}

DviSpecialType SourceSpecialImpl::Parse()

{
  static atomic_long nextid = 1;
  id = nextid++;
  const char* lpsz = GetXXX() + 4; // src:
  while (*lpsz == ' ')
  {
    ++lpsz;
  }
  if (*lpsz != 0 && isdigit(*lpsz))
  {
    char* fileName;
    lineNum = strtol(lpsz, &fileName, 10);
    if (fileName != nullptr)
    {
      while (*fileName == ' ')
      {
        ++fileName;
      }
      if (*fileName != 0)
      {
        this->fileName = fileName;
      }
    }
  }
  return DviSpecialType::Source;
}
