/* hypertex.cpp: HyperTeX specials

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

#include "Dvidoc.h"
#include "DviView.h"

bool DviView::GetHyperTeXSpecialAtCursor(string & name)
{
  DviDoc * pDoc = GetDocument();

  ASSERT_VALID(pDoc);

  int x, y;
  int pageIdx;

  if (!GetPageUnderCursor(pageIdx, x, y))
  {
    return false;
  }

  MIKTEX_ASSERT(pageIdx >= 0);

  if (pDoc->GetPageStatus(pageIdx) != PageStatus::Loaded)
  {
    return false;
  }

  CPoint ptDvi(PixelShrink(x), PixelShrink(y));

  DviPage * pDviPage = pDoc->GetLoadedPage(pageIdx);

  if (pDviPage == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }

  AutoUnlockPage autoUnlockPage(pDviPage);

  HypertexSpecial * pHyperSpecial;

  for (int idx = -1; (pHyperSpecial = pDviPage->GetNextHyperref(idx)) != nullptr; )
  {
    int llx, lly, urx, ury;
    pHyperSpecial->GetBbox(llx, lly, urx, ury);
    CRect hrefRect(PixelShrink(llx), PixelShrink(ury), PixelShrink(urx) + 1, PixelShrink(lly) + 1);
    if (hrefRect.PtInRect(ptDvi))
    {
      const char * lpsz = pHyperSpecial->GetName();
      name = lpsz != nullptr ? lpsz : "";
      return true;
    }
  }

  return false;
}

bool DviView::Navigate(const char * lpszUrl, bool remember)
{
  CWaitCursor wait;
  if (*lpszUrl == '#')
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    DviPosition position;
    if (pDoc->FindHyperLabel(lpszUrl + 1, position))
    {
      ClearSearchPosition();
      if (remember)
      {
        while (!forwardLocations.empty())
        {
          forwardLocations.pop();
        }
        RememberCurrentLocation();
      }
      if (curPageIdx != position.pageIdx)
      {
        ChangePage(position.pageIdx);
      }
      position.x = PixelShrink(position.x);
      position.x += GetPagePositionX(position.pageIdx);
      position.y = PixelShrink(position.y);
      position.y += GetPagePositionY(position.pageIdx);
      if (!IsPointInView(CPoint(position.x, position.y)))
      {
        CPoint ptScrollOld = GetScrollPosition();
        CenterOnPoint(CPoint(position.x, position.y));
        CPoint ptScroll = GetScrollPosition();
        if (g_pYapConfig->maintainHorizontalPosition && IsPointInView(CPoint(ptScrollOld.x, ptScroll.y)))
        {
          ScrollToPosition(CPoint(ptScrollOld.x, ptScroll.y));
        }
      }
      Invalidate();
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    // check to see whether it is DVI file reference
    if (_strnicmp("file:", lpszUrl, 5) == 0)
    {
      const char * lpszFileLabel = lpszUrl + 5;
      string dviFileName;
      string hashLabel;
      if (IsOtherDviFileLabel(lpszFileLabel, dviFileName, hashLabel))
      {
        PathName path;
        if (!FindDviFile(dviFileName.c_str(), path))
        {
          return false;
        }
        MIKTEX_ASSERT(AfxGetApp() != 0);
        CDocument * pDoc = AfxGetApp()->OpenDocumentFile(UT_(path.Get()));
        if (pDoc == nullptr)
        {
          return false;
        }
        if (hashLabel.empty())
        {
          return true;
        }
        if (!pDoc->IsKindOf(RUNTIME_CLASS(DviDoc)))
        {
          return false;
        }
        DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
        POSITION posView = pDviDoc->GetFirstViewPosition();
        while (posView != nullptr)
        {
          CView * pView = pDviDoc->GetNextView(posView);
          if (pView->IsKindOf(RUNTIME_CLASS(DviView)))
          {
            DviView * pDviView = reinterpret_cast<DviView*>(pView);
            if (pDviView->Navigate(hashLabel.c_str(), false))
            {
              return true;
            }
          }
        }
        return false;
      }
    }
    if (AllowShellCommand(lpszUrl))
    {
      ShellExecute(m_hWnd, _T("open"), UT_(lpszUrl), nullptr, nullptr, SW_SHOWNORMAL);
    }
    return true;
  }
}

void DviView::OnBack()
{
  try
  {
    MIKTEX_ASSERT(!backLocations.empty());
    location loc = backLocations.top();
    backLocations.pop();
    RememberCurrentLocation(true);
    GotoLocation(loc);
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void DviView::OnUpdateBack(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(!backLocations.empty());
}

void DviView::OnForward()
{
  try
  {
    MIKTEX_ASSERT(!forwardLocations.empty());
    location loc = forwardLocations.top();
    forwardLocations.pop();
    RememberCurrentLocation();
    GotoLocation(loc);
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void DviView::OnUpdateForward(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(!forwardLocations.empty());
}

bool DviView::IsOtherDviFileLabel(const char * lpszLabel, string & dviFileName, string & hashLabel)
{
  dviFileName = "";
  hashLabel = "";
  while (*lpszLabel != 0 && *lpszLabel != '#')
  {
    dviFileName += *lpszLabel++;
  }
  if (!PathName(dviFileName).HasExtension(".dvi"))
  {
    return false;
  }
  while (*lpszLabel != 0)
  {
    hashLabel += *lpszLabel++;
  }
  return true;
}

void DviView::RememberCurrentLocation(bool goingBack)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  location loc;
  loc.pageidx = GetCurrentPageIdx();
  MIKTEX_ASSERT(loc.pageidx >= 0 && loc.pageidx < pDoc->GetPageCount());
  loc.pt = GetScrollPosition();
  loc.pt.x *= pDoc->GetDisplayShrinkFactor();
  loc.pt.y *= pDoc->GetDisplayShrinkFactor();
  if (goingBack)
  {
    forwardLocations.push(loc);
  }
  else
  {
    backLocations.push(loc);
  }
}

void DviView::GotoLocation(const DviView::location & loc)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  if (loc.pageidx < 0 || loc.pageidx >= pDoc->GetPageCount())
  {
    MIKTEX_UNEXPECTED();
  }
  ChangePage(loc.pageidx);
  CPoint pt(loc.pt);
  pt.x /= pDoc->GetDisplayShrinkFactor();
  pt.y /= pDoc->GetDisplayShrinkFactor();
  ScrollToPosition(pt);
}

bool DviView::FindDviFile(const char * lpszFileName, PathName & result)
{
  if (Utils::IsAbsolutePath(lpszFileName))
  {
    result = lpszFileName;
  }
  else
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    MIKTEX_ASSERT(pDoc->GetPathName().GetLength() != 0);
    result = pDoc->GetDocDir();
    result /= lpszFileName;
  }
  if (File::Exists(result))
  {
    return true;
  }
  return session->FindFile(lpszFileName, FileType::DVI, result);
}

bool DviDoc::FindHyperLabel(const char * lpszFileName, DviPosition & position)
{
  return pDvi->FindHyperLabel(lpszFileName, position);
}
