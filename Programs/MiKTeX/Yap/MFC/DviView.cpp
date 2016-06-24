/* DviView.cpp:

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

#include "DocumentFontsPage.h"
#include "DviMagnifyingGlass.h"
#include "DviView.h"
#include "Dvidoc.h"
#include "GotoPageDialog.h"
#include "MainFrame.h"
#include "Ruler.h"
#include "SourceSpecialsDialog.h"

IMPLEMENT_DYNCREATE(DviView, CScrollView);

vector<double> DviView::gammaTable;

BEGIN_MESSAGE_MAP(DviView, CScrollView)
  ON_COMMAND(ID_BACK, &DviView::OnBack)
  ON_COMMAND(ID_DARKER_TEXT, &DviView::OnDarkerText)
  ON_COMMAND(ID_DOCUMENT_DOWN, &DviView::OnDocumentDown)
  ON_COMMAND(ID_DOCUMENT_UP, &DviView::OnDocumentUp)
  ON_COMMAND(ID_END, &DviView::OnEnd)
  ON_COMMAND(ID_FILE_DOCUMENT_PROPERTIES, &DviView::OnFileDocumentProperties)
  ON_COMMAND(ID_FILE_DVIPS, &DviView::OnFileDvips)
  ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
  ON_COMMAND(ID_FIRST_PAGE, &DviView::OnFirstPage)
  ON_COMMAND(ID_FORWARD, &DviView::OnForward)
  ON_COMMAND(ID_GOTO_PAGE, &DviView::OnGotoPage)
  ON_COMMAND(ID_HOME, &DviView::OnHome)
  ON_COMMAND(ID_LAST_PAGE, &DviView::OnLastPage)
  ON_COMMAND(ID_LIGHTER_TEXT, &DviView::OnLighterText)
  ON_COMMAND(ID_NEXT_PAGE, &DviView::OnNextPage)
  ON_COMMAND(ID_PAGEMODE_DVIPS, &DviView::OnDviPageModeDvips)
  ON_COMMAND(ID_PAGEMODE_PK, &DviView::OnDviPageModePk)
  ON_COMMAND(ID_PAGE_EDITOR, &DviView::OnPageEditor)
  ON_COMMAND(ID_PREV_PAGE, &DviView::OnPrevPage)
  ON_COMMAND(ID_SB_LINEDOWN, &DviView::OnLineDown)
  ON_COMMAND(ID_SB_LINELEFT, &DviView::OnLineLeft)
  ON_COMMAND(ID_SB_LINERIGHT, &DviView::OnLineRight)
  ON_COMMAND(ID_SB_LINEUP, &DviView::OnLineUp)
  ON_COMMAND(ID_SB_PAGEDOWN, &DviView::OnPageDown)
  ON_COMMAND(ID_SB_PAGELEFT, &DviView::OnPageLeft)
  ON_COMMAND(ID_SB_PAGERIGHT, &DviView::OnPageRight)
  ON_COMMAND(ID_SB_PAGEUP, &DviView::OnPageUp)
  ON_COMMAND(ID_TOOLS_ARROW, &DviView::OnToolsArrow)
  ON_COMMAND(ID_TOOLS_HAND, &DviView::OnToolsHand)
  ON_COMMAND(ID_TOOLS_MAGNIFYING_GLASS, &DviView::OnToolsMagnifyingGlass)
  ON_COMMAND(ID_TOOLS_SOURCESPECIALS, &DviView::OnToolsSourcespecials)
  ON_COMMAND(ID_VIEW_REFRESH, &DviView::OnViewRefresh)
  ON_COMMAND(ID_ZOOM_IN, &DviView::OnZoomIn)
  ON_COMMAND(ID_ZOOM_OUT, &DviView::OnZoomOut)
  ON_MESSAGE(WM_MAKEFONTS, &DviView::OnMakeFonts)
  ON_UPDATE_COMMAND_UI(ID_BACK, &DviView::OnUpdateBack)
  ON_UPDATE_COMMAND_UI(ID_DARKER_TEXT, &DviView::OnUpdateDarkerText)
  ON_UPDATE_COMMAND_UI(ID_FILE_DOCUMENT_PROPERTIES, &DviView::OnUpdateFileDocumentProperties)
  ON_UPDATE_COMMAND_UI(ID_FIRST_PAGE, &DviView::OnUpdateFirstPage)
  ON_UPDATE_COMMAND_UI(ID_FORWARD, &DviView::OnUpdateForward)
  ON_UPDATE_COMMAND_UI(ID_LAST_PAGE, &DviView::OnUpdateLastPage)
  ON_UPDATE_COMMAND_UI(ID_LIGHTER_TEXT, &DviView::OnUpdateLighterText)
  ON_UPDATE_COMMAND_UI(ID_NEXT_PAGE, &DviView::OnUpdateNextPage)
  ON_UPDATE_COMMAND_UI(ID_PAGEMODE_DVIPS, &DviView::OnUpdateDviPageModeDvips)
  ON_UPDATE_COMMAND_UI(ID_PAGEMODE_PK, &DviView::OnUpdateDviPageModePk)
  ON_UPDATE_COMMAND_UI(ID_PREV_PAGE, &DviView::OnUpdatePrevPage)
  ON_UPDATE_COMMAND_UI(ID_TOOLS_ARROW, &DviView::OnUpdateToolsArrow)
  ON_UPDATE_COMMAND_UI(ID_TOOLS_HAND, &DviView::OnUpdateToolsHand)
  ON_UPDATE_COMMAND_UI(ID_TOOLS_MAGNIFYING_GLASS, &DviView::OnUpdateToolsMagnifyingGlass)
  ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, &DviView::OnUpdateZoomIn)
  ON_WM_CHAR()
  ON_WM_CONTEXTMENU()
  ON_WM_ERASEBKGND()
  ON_WM_GETMINMAXINFO()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_RBUTTONDOWN()
  ON_WM_SETCURSOR()
  ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, &DviView::OnUpdateFilePrint)
  ON_UPDATE_COMMAND_UI(ID_FILE_DVIPS, &DviView::OnUpdateFileDvips)
  ON_UPDATE_COMMAND_UI(ID_PAGE_EDITOR, &DviView::OnUpdatePageEditor)
  ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, &DviView::OnUpdateZoomOut)
  ON_UPDATE_COMMAND_UI(ID_TOOLS_SOURCESPECIALS, &DviView::OnUpdateToolsSourcespecials)
  ON_UPDATE_COMMAND_UI(ID_GOTO_PAGE, &DviView::OnUpdateGotoPage)
END_MESSAGE_MAP();

DviView::DviView() :
  currentMouseTool(static_cast<MouseTool>(g_pYapConfig->lastTool)),
  cursorPosition(0, 0)
{
  ClearSearchPosition();
}

DviView::~DviView()
{
  try
  {
    if (pSourceSpecialDialog != nullptr)
    {
      if (!pSourceSpecialDialog->DestroyWindow())
      {
        // TODO: TraceStream::TraceLastWin32Error("DestroyWindow", nullptr, __FILE__, __LINE__);
      }
      pSourceSpecialDialog = nullptr;
    }
    if (pToolWindow != nullptr)
    {
      if (!pToolWindow->DestroyWindow())
      {
        // TODO: TraceStream::TraceLastWin32Error("DestroyWindow", nullptr, __FILE__, __LINE__);
      }
      pToolWindow = nullptr;
    }
    if (mouseCaptured)
    {
      if (g_pYapConfig->magGlassHidesCursor)
      {
        ShowCursor(TRUE);
      }
      if (!ReleaseCapture())
      {
        // TODO: TraceStream::TraceLastWin32Error("ReleaseCapture", nullptr, __FILE__, __LINE__);
      }
      mouseCaptured = false;
    }
  }
  catch (const exception &)
  {
  }
}

void DviView::OnActivateView(BOOL activate, CView * pActivateView, CView * pDeactiveView)
{
  try
  {
    if (activate)
    {
      DviDoc * pDoc = GetDocument();
      ASSERT_VALID(pDoc);
      DviDoc::DviFileStatus fileStatus = pDoc->GetDviFileStatus();
      switch (fileStatus)
      {
      case DviDoc::DVIFILE_LOST:
      case DviDoc::DVIFILE_MODIFIED:
        OnViewRefresh();
        break;
      }
    }
    else
    {
      if (mouseCaptured)
      {
        if (g_pYapConfig->magGlassHidesCursor)
        {
          ShowCursor(TRUE);
        }
        if (!ReleaseCapture())
        {
          MIKTEX_FATAL_WINDOWS_ERROR("ReleaseCapture");
        }
        mouseCaptured = false;
      }
      if (pToolWindow != nullptr)
      {
        if (!pToolWindow->DestroyWindow())
        {
          MIKTEX_FATAL_WINDOWS_ERROR("DestroyWindow");
        }
        pToolWindow = nullptr;
      }
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  CScrollView::OnActivateView(activate, pActivateView, pDeactiveView);
}

void DviView::OnInitialUpdate()
{
  CScrollView::OnInitialUpdate();
  curPageIdx = 0;
  try
  {
    if (g_pYapConfig->dviPageMode != DviPageMode::Dvips)
    {
      PostMessage(WM_MAKEFONTS);
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }
}

CSize DviView::DeterminePageSize()
{
  CRect rect;
  GetClientRect(&rect);
  CSize sizePage(rect.right / 3, rect.bottom / 3);
  return sizePage;
}

void DviView::OnUpdate(CView * pSender, LPARAM lHint, CObject * pHint)
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      CSize sizePage = pDoc->GetPaperSize();

      int n = (g_pYapConfig->continuousView
        ? (g_pYapConfig->doublePage
          ? pDoc->GetRowCount()
          : pDoc->GetPageCount())
        : 1);

      if (n <= 0)
      {
        n = 1;
      }

      CSize sizeTotal;

      sizeTotal.cy = GetTopMargin();
      sizeTotal.cy += n * sizePage.cy;
      sizeTotal.cy += (n - 1) * GetVerticalInterPageOffset();

      sizeTotal.cx = GetLeftMargin();
      sizeTotal.cx += sizePage.cx;
      if (g_pYapConfig->doublePage)
      {
        sizeTotal.cx += sizePage.cx;
        sizeTotal.cx += GetHorizontalInterPageOffset();
      }
      sizeTotal.cx += 10;

      SetScrollSizes(MM_TEXT, sizeTotal, DeterminePageSize());
    }

    CScrollView::OnUpdate(pSender, lHint, pHint);
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

BOOL DviView::OnScroll(UINT scrollCode, UINT pos, BOOL doScroll)
{
  try
  {
    // See Q166473.
    SCROLLINFO info;
    info.cbSize = sizeof(SCROLLINFO);
    info.fMask = SIF_TRACKPOS;

    if (LOBYTE(scrollCode) == SB_THUMBTRACK)
    {
      if (!GetScrollInfo(SB_HORZ, &info, SIF_TRACKPOS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("OnScroll");
      }
      pos = info.nTrackPos;
    }

    if (HIBYTE(scrollCode) == SB_THUMBTRACK)
    {
      if (!GetScrollInfo(SB_VERT, &info, SIF_TRACKPOS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("OnScroll");
      }
      pos = info.nTrackPos;
    }

  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  return CScrollView::OnScroll(scrollCode, pos, doScroll);
}

BOOL DviView::OnScrollBy(CSize sizeScroll, BOOL doScroll)
{
  return CScrollView::OnScrollBy(sizeScroll, doScroll);
}

bool DviView::IsPointInView(CPoint pt)
{
  CRect rect;
  GetClientRect(&rect);
  CPoint ptScroll = GetScrollPosition();
  if (pt.x < ptScroll.x)
  {
    return false;
  }
  if (pt.x >= ptScroll.x + rect.right)
  {
    return false;
  }
  if (pt.y < ptScroll.y)
  {
    return false;
  }
  if (pt.y >= ptScroll.y + rect.bottom)
  {
    return false;
  }
  return true;
}

int DviView::GetPagePositionX(int pageIdx)
{
  int x = GetLeftMargin();
  if (!g_pYapConfig->doublePage)
  {
    return x;
  }
  MIKTEX_ASSERT(pageIdx >= 0);
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  if (pDoc->IsOnRight(pageIdx))
  {
    x += pDoc->GetPaperSize().cx;
    x += GetHorizontalInterPageOffset();
  }
  return x;
}

int DviView::GetPagePositionY(int pageIdx)
{
  int y = GetTopMargin();
  if (!g_pYapConfig->continuousView)
  {
    return y;
  }
  MIKTEX_ASSERT(pageIdx >= 0);
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  int n = (g_pYapConfig->doublePage
    ? pDoc->GetRowIdx(pageIdx)
    : pageIdx);
  MIKTEX_ASSERT(n >= 0);
  if (n > 0)
  {
    y += n * (pDoc->GetPaperSize().cy);
    y += n * GetVerticalInterPageOffset();
  }
  return y;
}

void DviView::ChangePage(int pageIdx)
{
  MIKTEX_ASSERT(pageIdx >= 0);
  curPageIdx = pageIdx;

  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CPoint pt = GetScrollPosition();
  if (!g_pYapConfig->maintainHorizontalPosition)
  {
    pt.x = 0;
  }

  if (g_pYapConfig->continuousView)
  {
    ScrollToPosition(CPoint(pt.x, GetPagePositionY(pageIdx)));
    return;
  }

  if (!g_pYapConfig->maintainVerticalPosition)
  {
    pt.y = 0;
  }

  ScrollToPosition(pt);

  int lm = GetLeftMargin();
  int tm = GetTopMargin();

  CRect rectClient;
  GetClientRect(&rectClient);

  // clear the search cursor
  if (pageSearchPosition != pageIdx)
  {
    ClearSearchPosition();
  }

  // check to see whether we're on the right-hand side of the row
  bool onRight = pDoc->IsOnRight(pageIdx);

  // invalidate left margin
  CRect rectLM(0, 0, lm, rectClient.bottom);
  InvalidateRect(&rectLM, TRUE);

  // invalidate top margin
  CRect rectTM(0, 0, rectClient.right, tm);
  InvalidateRect(&rectTM, TRUE);

  // check to see whether the page exists
  BOOL pageExists = !(g_pYapConfig->doublePage && onRight);

  // invalidate paper
  CSize sizePage = pDoc->GetPaperSize();
  CRect rectPaper(lm, tm, lm + sizePage.cx, tm + sizePage.cy);
  InvalidateRect(&rectPaper, !pageExists);

  // invalidate top border
  CRect rectTB(lm, tm - GetPageBorderWidth(), lm + sizePage.cx + GetPageBorderWidth(), tm);
  InvalidateRect(&rectTB, !pageExists);

  // invalidate right border
  CRect rectRB(lm + sizePage.cx, tm, lm + sizePage.cx + GetPageBorderWidth(), tm + sizePage.cy + GetPageBorderWidth());
  InvalidateRect(&rectRB, !pageExists);

  // invalidate bottom border
  CRect rectBB(lm - GetPageBorderWidth(), tm + sizePage.cy, lm + sizePage.cx, tm + sizePage.cy + GetPageBorderWidth());
  InvalidateRect(&rectBB, !pageExists);

  // invalidate left border
  CRect rectLB(lm - GetPageBorderWidth(), tm - GetPageBorderWidth(), lm, tm + sizePage.cy + GetPageBorderWidth());
  InvalidateRect(&rectLB, !pageExists);

  // invalidate right shadow
  CRect rectRS(lm + sizePage.cx + GetPageBorderWidth(), tm + GetPageBorderShadow(), (lm + sizePage.cx + GetPageBorderWidth()
    + GetPageBorderShadow()), (tm + sizePage.cy + GetPageBorderWidth()
      + GetPageBorderShadow()));
  InvalidateRect(&rectRS, !pageExists);

  // invalidate bottom shadow
  CRect rectBS(lm + GetPageBorderShadow(), tm + sizePage.cy + GetPageBorderWidth(), lm + sizePage.cx + GetPageBorderWidth(), (tm + sizePage.cy + GetPageBorderWidth()
    + GetPageBorderShadow()));
  InvalidateRect(&rectBS, !pageExists);

  // same procedure for page on the right
  if (g_pYapConfig->doublePage)
  {
#if 0
    CRect rectInter(lm + sizePage.cx
      + GetPageBorderWidth() + GetPageBorderShadow(), 0, GetHorizontalInterPageOffset(), rectClient.bottom);
    InvalidateRect(rectInter, TRUE);
#else
    rectLM.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    InvalidateRect(rectLM, TRUE);
#endif
    rectPaper.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    rectTB.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    rectRB.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    rectBB.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    rectLB.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    rectRS.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    rectBS.OffsetRect(sizePage.cx + GetHorizontalInterPageOffset(), 0);
    pageExists = pageIdx + 1 < pDoc->GetPageCount() && pDoc->IsOnRight(pageIdx + 1);
    InvalidateRect(&rectPaper, !pageExists);
    InvalidateRect(&rectTB, !pageExists);
    InvalidateRect(&rectRB, !pageExists);
    InvalidateRect(&rectBB, !pageExists);
    InvalidateRect(&rectLB, !pageExists);
    InvalidateRect(&rectRS, !pageExists);
    InvalidateRect(&rectBS, !pageExists);
  }
}

void DviView::OnNextPage()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    int pageIdx = GetCurrentPageIdx();
    if (pageIdx < 0)
    {
      MIKTEX_UNEXPECTED();
    }
    int d = 1;
    string commandPrefix = GetCommandPrefix(true);
    if (!commandPrefix.empty())
    {
      d = std::stoi(commandPrefix);
    }
    if (g_pYapConfig->doublePage
      && pDoc->IsOnLeft(pageIdx)
      && pageIdx + 2 * d >= 0
      && pageIdx + 2 * d <= pDoc->GetPageCount())
    {
      d *= 2;
    }
    if (pageIdx + d < 0 || pageIdx + d >= pDoc->GetPageCount())
    {
      return;
    }
    ChangePage(pageIdx + d);
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

void DviView::OnUpdateNextPage(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      int pageIdx = GetCurrentPageIdx();
      if (pageIdx < 0)
      {
        MIKTEX_UNEXPECTED();
      }
      if (g_pYapConfig->doublePage)
      {
        enable =
          (pDoc->IsOnRight(pageIdx)
            ? (pDoc->GetPageCount() - 1 > pageIdx)
            : (pDoc->GetPageCount() - 2 > pageIdx));
      }
      else
      {
        enable = (pDoc->GetPageCount() - 1 > pageIdx);
      }
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnPrevPage()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    int pageIdx = GetCurrentPageIdx();
    if (pageIdx < 0)
    {
      MIKTEX_UNEXPECTED();
    }
    int pageIdxOld = pageIdx;
    int d = -1;
    string commandPrefix = GetCommandPrefix(true);
    if (!commandPrefix.empty())
    {
      d = std::stoi(commandPrefix) * -1;
    }
    if (g_pYapConfig->doublePage
      && pDoc->IsOnLeft(pageIdx)
      && pageIdx + 2 * d >= 0
      && pageIdx + 2 * d <= pDoc->GetPageCount())
    {
      d *= 2;
    }
    if (pageIdx + d < 0 || pageIdx + d >= pDoc->GetPageCount())
    {
      return;
    }
    pageIdx += d;
    if (g_pYapConfig->doublePage && pDoc->IsOnRight(pageIdx))
    {
      if (pageIdx + 1 != pageIdxOld)
      {
        pageIdx += 1;
      }
      else if (pageIdx > 0 && pDoc->IsOnLeft(pageIdx - 1))
      {
        pageIdx -= 1;
      }
    }
    ChangePage(pageIdx);
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

void DviView::OnUpdatePrevPage(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      int pageIdx = GetCurrentPageIdx();
      if (pageIdx < 0)
      {
        MIKTEX_UNEXPECTED();
      }
      enable = pageIdx > 0;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnPageDown()
{
  try
  {
    OnScrollBy(CSize(0, DeterminePageSize().cy), TRUE);
    UpdateWindow();
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

void DviView::OnDocumentDown()
{
  try
  {
    if (OnScrollBy(CSize(0, DeterminePageSize().cy), TRUE))
    {
      UpdateWindow();
    }
    else if (!g_pYapConfig->continuousView)
    {
      AutoRestore<bool> autoRestore(g_pYapConfig->maintainVerticalPosition);
      g_pYapConfig->maintainVerticalPosition = false;
      OnNextPage();
    }
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

void DviView::OnPageUp()
{
  try
  {
    if (OnScrollBy(CSize(0, -DeterminePageSize().cy), TRUE))
    {
      UpdateWindow();
    }
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

void DviView::OnDocumentUp()
{
  try
  {
    if (OnScrollBy(CSize(0, -DeterminePageSize().cy), TRUE))
    {
      UpdateWindow();
    }
    else if (!g_pYapConfig->continuousView)
    {
      AutoRestore<bool> autoRestore(g_pYapConfig->maintainVerticalPosition);
      g_pYapConfig->maintainVerticalPosition = false;
      OnPrevPage();
    }
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

void DviView::OnLineDown()
{
  int mapMode;
  CSize sizeTotal, sizePage, sizeLine;
  GetDeviceScrollSizes(mapMode, sizeTotal, sizePage, sizeLine);
  if (OnScrollBy(CSize(0, sizeLine.cy), TRUE))
  {
    UpdateWindow();
  }
}

void DviView::OnLineUp()
{
  int mapMode;
  CSize sizeTotal, sizePage, sizeLine;
  GetDeviceScrollSizes(mapMode, sizeTotal, sizePage, sizeLine);
  if (OnScrollBy(CSize(0, -sizeLine.cy), TRUE))
  {
    UpdateWindow();
  }
}

void DviView::OnLineLeft()
{
  int mapMode;
  CSize sizeTotal, sizePage, sizeLine;
  GetDeviceScrollSizes(mapMode, sizeTotal, sizePage, sizeLine);
  if (OnScrollBy(CSize(-sizeLine.cx, 0), TRUE))
  {
    UpdateWindow();
  }
}

void DviView::OnLineRight()
{
  int mapMode;
  CSize sizeTotal, sizePage, sizeLine;
  GetDeviceScrollSizes(mapMode, sizeTotal, sizePage, sizeLine);
  if (OnScrollBy(CSize(sizeLine.cx, 0), TRUE))
  {
    UpdateWindow();
  }
}

void DviView::OnPageLeft()
{
  try
  {
    if (OnScrollBy(CSize(-DeterminePageSize().cx, 0), TRUE))
    {
      UpdateWindow();
    }
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

void DviView::OnPageRight()
{
  try
  {
    if (OnScrollBy(CSize(DeterminePageSize().cx, 0), TRUE))
    {
      UpdateWindow();
    }
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

void DviView::OnLastPage()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    int p = pDoc->GetPageCount();
    if (p <= 0)
    {
      MIKTEX_UNEXPECTED();
    }
    ChangePage(p - 1);
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

void DviView::OnUpdateLastPage(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = (pDoc->GetPageCount() - 1 > GetCurrentPageIdx());
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnFirstPage()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetPageCount() <= 0)
    {
      MIKTEX_UNEXPECTED();
    }
    ChangePage(0);
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

void DviView::OnUpdateFirstPage(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = (GetCurrentPageIdx() > 0);
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnGotoPage()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    string commandPrefix = GetCommandPrefix(true);
    int pageIdx;
    if (!commandPrefix.empty())
    {
      int pageNum = std::stoi(commandPrefix);
      pageIdx = pDoc->FindPage(pageNum);
    }
    else
    {
      GotoPageDialog dlg(this, pDoc, curPageIdx);
      if (dlg.DoModal() != IDOK)
      {
        return;
      }
      pageIdx = dlg.GetPageIdx();
    }
    if (pageIdx < 0)
    {
      MIKTEX_UNEXPECTED();
    }
    RememberCurrentLocation();
    while (!forwardLocations.empty())
    {
      forwardLocations.pop();
    }
    ChangePage(pageIdx);
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

void DviView::OnUpdateGotoPage(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = TRUE;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnHome()
{
  CPoint pt = GetScrollPosition();
  if (!g_pYapConfig->maintainHorizontalPosition)
  {
    pt.x = 0;
  }
  pt.y = 0;
  ScrollToPosition(pt);
}

void DviView::OnEnd()
{
  try
  {
    CPoint pt = GetScrollPosition();
    if (!g_pYapConfig->maintainHorizontalPosition)
    {
      pt.x = 0;
    }
    pt.y = GetTotalSize().cy;
    ScrollToPosition(pt);
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

void DviView::Zoom(bool zoomIn)
{
  int sign = (zoomIn ? -1 : 1);
  int d = 1 * sign;
  string commandPrefix = GetCommandPrefix(true);
  if (!commandPrefix.empty())
  {
    d = std::stoi(commandPrefix) * sign;
  }
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  int oldShrink = pDoc->GetDisplayShrinkFactor();
  int newShrink = pDoc->GetDisplayShrinkFactor() + d;
  if (newShrink < 1)
  {
    return;
  }
  double f = static_cast<double>(oldShrink) / newShrink;
  YapLog(T_("Yap zoom: %d/%d=%f"), oldShrink, newShrink, f);
  CPoint oldScrollPosition = GetScrollPosition();
  pDoc->Shrink(d);
  g_pYapConfig->displayShrinkFactor = newShrink;
  pDoc->UpdateAllViews(0);
  CPoint pt;
  CSize size = GetTotalSize();
  pt.x = static_cast<int>(oldScrollPosition.x * f + 0.5);
  if (pt.x > size.cx)
  {
    pt.x = size.cx;
  }
  pt.y = static_cast<int>(oldScrollPosition.y * f + 0.5);
  if (pt.y > size.cy)
  {
    pt.y = size.cy;
  }
  ScrollToPosition(pt);
}

void DviView::OnZoomIn()
{
  try
  {
    Zoom(true);
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

void DviView::OnUpdateZoomIn(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = (pDoc->GetDisplayShrinkFactor() > 1);
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnZoomOut()
{
  try
  {
    Zoom(false);
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

void DviView::OnUpdateZoomOut(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = TRUE;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

void DviView::OnLButtonDown(UINT flags, CPoint point)
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    cursorPosition = point;

    leftButtonDown = true;

    string hyperTarget;

    int pageIdx;
    int x, y;

    bool havePage = GetPageUnderCursor(pageIdx, x, y);

    if (havePage && pageIdx < 0)
    {
      MIKTEX_UNEXPECTED();
    }

    if (!havePage || GetHyperTeXSpecialAtCursor(hyperTarget))
    {
    }
    else if (currentMouseTool == RulerTool)
    {
      MIKTEX_ASSERT(!mouseCaptured);
      MIKTEX_ASSERT(pToolWindow == 0);
      SetCapture();
      if (g_pYapConfig->magGlassHidesCursor)
      {
        ShowCursor(FALSE);
      }
      mouseCaptured = true;
      CPoint screenPoint = point;
      ClientToScreen(&screenPoint);
      Ruler * pRuler =
        new Ruler(this, pDoc->GetPaperSize(),
          g_pYapConfig->unit,
          static_cast<double>(pDoc->GetDisplayResolution()) / pDoc->GetDisplayShrinkFactor(),
          (((flags & MK_SHIFT) != 0) ? Ruler::Vertical : Ruler::Horizontal));
      pRuler->Create(screenPoint);
      pToolWindow = pRuler;
    }
    else if (currentMouseTool == Hand)
    {
      ::SetCursor(AfxGetApp()->LoadCursor(IDC_PRESSED_HAND));
      toolActive = true;
    }
    else if (currentMouseTool == MagnifyingGlass)
    {
      MIKTEX_ASSERT(!mouseCaptured);
      MIKTEX_ASSERT(pToolWindow == nullptr);
      DviMagnifyingGlass::Size size = DviMagnifyingGlass::Small;
      int magGlassShrinkFactor = 0;
      if ((flags & MK_SHIFT) != 0)
      {
        size = DviMagnifyingGlass::Medium;
        magGlassShrinkFactor = g_pYapConfig->magGlassMediumShrinkFactor;
      }
      if ((flags & MK_CONTROL) != 0)
      {
        size = DviMagnifyingGlass::Large;
        magGlassShrinkFactor = g_pYapConfig->magGlassLargeShrinkFactor;
      }
      string commandPrefix = GetCommandPrefix(true);
      if (!commandPrefix.empty())
      {
        magGlassShrinkFactor = atoi(commandPrefix.c_str());
      }
      if (magGlassShrinkFactor <= 0)
      {
        magGlassShrinkFactor = g_pYapConfig->magGlassSmallShrinkFactor;
      }
      SetCapture();
      if (g_pYapConfig->magGlassHidesCursor)
      {
        ShowCursor(FALSE);
      }
      mouseCaptured = true;
      CPoint screenPoint = point;
      ClientToScreen(&screenPoint);
      DviMagnifyingGlass * pGlass = new DviMagnifyingGlass(this, GetDocument(), size, magGlassShrinkFactor);
      pGlass->Create(screenPoint);
      pToolWindow = pGlass;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  CScrollView::OnLButtonDown(flags, point);
}

void DviView::OnLButtonUp(UINT flags, CPoint point)
{
  try
  {
    cursorPosition = point;

    leftButtonDown = false;

    toolActive = false;

    if (pToolWindow != nullptr)
    {
      MIKTEX_ASSERT(mouseCaptured);
      if (g_pYapConfig->magGlassHidesCursor)
      {
        ShowCursor(TRUE);
      }
      if (!ReleaseCapture())
      {
        MIKTEX_UNEXPECTED();
      }
      mouseCaptured = false;
      pToolWindow->ShowWindow(SW_HIDE);
      pToolWindow->DestroyWindow();
      pToolWindow = nullptr;
    }

    string hyperTarget;

    if (GetHyperTeXSpecialAtCursor(hyperTarget))
    {
      Navigate(hyperTarget.c_str());
    }
    else if (currentMouseTool == Hand)
    {
      ::SetCursor(AfxGetApp()->LoadCursor(IDC_HAND_TOOL));
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  CScrollView::OnLButtonUp(flags, point);
}

void DviView::OnMouseMove(UINT flags, CPoint point)
{
  try
  {
    switch (currentMouseTool)
    {
    case RulerTool:
      if (pToolWindow != nullptr)
      {
        Ruler * pRuler = dynamic_cast<Ruler *>(pToolWindow);
        if (pRuler == nullptr)
        {
          MIKTEX_UNEXPECTED();
        }
        CPoint screenPoint = point;
        ClientToScreen(&screenPoint);
        pRuler->MoveRuler(screenPoint);
      }
      break;
    case Hand:
      if ((flags & MK_LBUTTON) != 0
        && toolActive
        && OnScrollBy(CSize(cursorPosition.x - point.x, cursorPosition.y - point.y), TRUE))
      {
        UpdateWindow();
      }
      break;
    case MagnifyingGlass:
      if (pToolWindow != nullptr)
      {
        DviMagnifyingGlass * pGlass = dynamic_cast<DviMagnifyingGlass *>(pToolWindow);
        if (pGlass == nullptr)
        {
          MIKTEX_UNEXPECTED();
        }
        CPoint screenPoint = point;
        ClientToScreen(&screenPoint);
        pGlass->MoveGlass(screenPoint);
      }
      break;
    }

    cursorPosition = point;

    ASSERT_VALID(AfxGetApp());
    MainFrame * pMain = reinterpret_cast<MainFrame*>((AfxGetApp())->m_pMainWnd);
    ASSERT_VALID(pMain);
    MIKTEX_ASSERT(pMain->IsKindOf(RUNTIME_CLASS(MainFrame)));

    string hyperTarget;

    if (!leftButtonDown && (GetHyperTeXSpecialAtCursor(hyperTarget)))
    {
      pMain->SetMessageText(UT_(hyperTarget.c_str()));
    }
    else
    {
      pMain->SetMessageText(_T(""));
    }

#if 0
    {
      int pageIdx, x, y;
      bool bPage = GetPageUnderCursor(pageIdx, x, y);
      int sheetIdx
        = (g_pYapConfig->continuousView
          ? DetermineRowIdx(GetScrollPosition().y + point.y)
          : 0);
      CString info;
      info.Format("r:%d p:%d(%d) x:%d y:%d", sheetIdx, pageIdx, static_cast<int>(bPage), x, y);
      pMain->SetWindowText(strInfo);
    }
#endif
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  CScrollView::OnMouseMove(flags, point);
}

BOOL DviView::OnSetCursor(CWnd * pWnd, UINT hitTest, UINT message)
{
  if (hitTest != HTCLIENT)
  {
    return CScrollView::OnSetCursor(pWnd, hitTest, message);
  }

  try
  {
    string hyperTarget;

    int pageIdx;
    int x, y;
    if (!GetPageUnderCursor(pageIdx, x, y))
    {
      return CScrollView::OnSetCursor(pWnd, hitTest, message);
    }
    if (pageIdx < 0)
    {
      MIKTEX_UNEXPECTED();
    }
    if (!leftButtonDown && GetHyperTeXSpecialAtCursor(hyperTarget))
    {
      ::SetCursor(AfxGetApp()->LoadCursor(IDC_HAND_POINT));
    }
    else
    {
      switch (currentMouseTool)
      {
      case Hand:
        if (leftButtonDown)
        {
          ::SetCursor(AfxGetApp()->LoadCursor(IDC_PRESSED_HAND));
        }
        else
        {
          ::SetCursor(AfxGetApp()->LoadCursor(IDC_HAND_TOOL));
        }
        break;
      case MagnifyingGlass:
        ::SetCursor(AfxGetApp()->LoadCursor(IDC_MAG));
        break;
      default:
        ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
        break;
      }
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  return FALSE;
}

void DviView::OnContextMenu(CWnd * pWnd, CPoint point)
{
  UNUSED_ALWAYS(pWnd);
  try
  {
    if (mouseCaptured)
    {
      if (g_pYapConfig->magGlassHidesCursor)
      {
        ShowCursor(TRUE);
      }
      if (!ReleaseCapture())
      {
        MIKTEX_UNEXPECTED();
      }
      mouseCaptured = false;
    }
    if (pToolWindow != nullptr)
    {
      if (!pToolWindow->DestroyWindow())
      {
        MIKTEX_UNEXPECTED();
      }
      pToolWindow = nullptr;
    }
    ASSERT_VALID(AfxGetApp());
    ASSERT_VALID(AfxGetApp()->m_pMainWnd);
    CMenu menu;
    if (!menu.LoadMenu(IDR_CONTEXT_MENU))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CMenu::LoadMenu");
    }
    AutoDetachMenu autoDetachMenu(&menu);
    CMenu * pPopup = menu.GetSubMenu(0);
    MIKTEX_ASSERT(pPopup != nullptr);
    if (!pPopup->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, AfxGetApp()->m_pMainWnd, nullptr))
    {
      MIKTEX_UNEXPECTED();
    }
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

void DviView::OnToolsHand()
{
  currentMouseTool = Hand;
  g_pYapConfig->lastTool = Hand;
}

void DviView::OnUpdateToolsHand(CCmdUI * pCmdUI)
{
  pCmdUI->SetCheck(currentMouseTool == Hand ? 1 : 0);
}

void DviView::OnToolsMagnifyingGlass()
{
  currentMouseTool = MagnifyingGlass;
  g_pYapConfig->lastTool = MagnifyingGlass;
}

void DviView::OnUpdateToolsMagnifyingGlass(CCmdUI * pCmdUI)
{
  pCmdUI->SetCheck(currentMouseTool == MagnifyingGlass ? 1 : 0);
}

void DviView::OnToolsArrow()
{
  currentMouseTool = RulerTool;
  g_pYapConfig->lastTool = RulerTool;
}

void DviView::OnUpdateToolsArrow(CCmdUI * pCmdUI)
{
  pCmdUI->SetCheck(currentMouseTool == RulerTool ? 1 : 0);
}

void DviView::OnLButtonDblClk(UINT flags, CPoint point)
{
  UNUSED_ALWAYS(flags);
  cursorPosition = point;
  OnPageEditor();
}

bool DviView::GetPoint(int & x, int & y)
{
  int pageIdx;
  if (!GetPageUnderCursor(pageIdx, x, y))
  {
    return false;
  }
  if (pageIdx < 0)
  {
    MIKTEX_UNEXPECTED();
  }
  return true;
}

void DviView::ClearSearchPosition()
{
  searchPosition.x = searchPosition.y = -1;
  pageSearchPosition = -1;
}

void DviView::OnChar(UINT ch, UINT repeatCount, UINT flags)
{
  try
  {
    if (!AddCommandPrefixChar(static_cast<char>(ch)))
    {
      CScrollView::OnChar(ch, repeatCount, flags);
    }
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

bool DviView::GetPageUnderCursor(int & pageIdx, int & x, int & y)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  if (pDoc->GetDviFileStatus() != DviDoc::DVIFILE_LOADED)
  {
    return false;
  }

  CPoint ptScroll = GetScrollPosition();

  if (g_pYapConfig->continuousView)
  {
    int n = DetermineRowIdx(ptScroll.y + cursorPosition.y);
    if (g_pYapConfig->doublePage)
    {
      pageIdx = pDoc->DeterminePageIdx(n, false);
    }
    else
    {
      pageIdx = n;
    }
  }
  else
  {
    pageIdx = curPageIdx;
  }

  if (pageIdx < 0)
  {
    MIKTEX_UNEXPECTED();
  }

  CSize sizePage = pDoc->GetPaperSize();

  if (ptScroll.x + cursorPosition.x > GetLeftMargin() + sizePage.cx)
  {
    if (g_pYapConfig->doublePage)
    {
      if (ptScroll.x + cursorPosition.x >= (GetLeftMargin() + sizePage.cx + GetHorizontalInterPageOffset()))
      {
        if (pDoc->IsOnLeft(pageIdx) && pageIdx + 1 < pDoc->GetPageCount())
        {
          ++pageIdx;
        }
      }
    }
  }

  if (g_pYapConfig->doublePage)
  {
    if (pDoc->IsOnLeft(pageIdx))
    {
      x = ptScroll.x + cursorPosition.x - GetLeftMargin();
    }
    else
    {
      x = ptScroll.x + cursorPosition.x - GetLeftMargin() - sizePage.cx - GetHorizontalInterPageOffset();
    }
  }
  else
  {
    x = ptScroll.x + cursorPosition.x - GetLeftMargin();
  }

  int n = g_pYapConfig->continuousView ? (g_pYapConfig->doublePage ? pDoc->GetRowIdx(pageIdx) : pageIdx) : 0;

  y = ptScroll.y + cursorPosition.y;

  y -= GetTopMargin();
  y -= n * sizePage.cy;
  y -= n * GetVerticalInterPageOffset();

  bool havePage = x >= 0 && x < sizePage.cx && y >= 0 && y < sizePage.cy;

  x = UnPixelShrink(pDoc, x);
  y = UnPixelShrink(pDoc, y);

  return havePage;
}

int DviView::GetLeftMargin() const
{
  int x = 10;
  bool center = true;
  if (center)
  {
    RECT rect;
    GetClientRect(&rect);
    int widthWindow = rect.right - rect.left;
    const DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    CSize sizePage = pDoc->GetPaperSize();
    int widthDisplay;
    if (g_pYapConfig->doublePage)
    {
      widthDisplay = 2 * sizePage.cx + GetHorizontalInterPageOffset();
    }
    else
    {
      widthDisplay = sizePage.cx;
    }
    if (widthDisplay < widthWindow)
    {
      x = (widthWindow - widthDisplay) / 2;
    }
  }
  return x + GetRulerWidth();
}

int DviView::GetTopMargin() const
{
  int y = 10;
  bool center = true;
  if (center && !g_pYapConfig->continuousView)
  {
    RECT rect;
    GetClientRect(&rect);
    int heightWindow = rect.bottom - rect.top;
    const DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    CSize sizePage = pDoc->GetPaperSize();
    if (sizePage.cy < heightWindow)
    {
      y = (heightWindow - sizePage.cy) / 2;
    }
  }
  return y + GetRulerHeight();
}

void DviView::OnViewRefresh()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    int pageIdx = curPageIdx;
    pDoc->Reread();
    if (pageIdx < pDoc->GetPageCount())
    {
      curPageIdx = pageIdx;
    }
    else
    {
      curPageIdx = pDoc->GetPageCount() - 1;
    }
    pDoc->UpdateAllViews(0);
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

void DviView::GetVisiblePages(int & pageIdx1, int & pageIdx2) const
{
  const DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  if (pPrintInfo != nullptr)
  {
    pageIdx1 = pageIdx2 = pPrintInfo->m_nCurPage - 1;
    MIKTEX_ASSERT(pageIdx1 >= 0);
  }
  else if (g_pYapConfig->continuousView)
  {
    CRect rectClient;
    GetClientRect(&rectClient);
    int iRowIdx1 = DetermineRowIdx(GetScrollPosition().y);
    int iRowIdx2 = DetermineRowIdx(GetScrollPosition().y + rectClient.bottom - 1);
    if (!g_pYapConfig->doublePage)
    {
      pageIdx1 = iRowIdx1;
      pageIdx2 = iRowIdx2;
    }
    else
    {
      pageIdx1 = pDoc->DeterminePageIdx(iRowIdx1, false);
      pageIdx2 = pDoc->DeterminePageIdx(iRowIdx2, true);
    }
    return;
  }
  else if (!g_pYapConfig->doublePage)
  {
    pageIdx1 = pageIdx2 = curPageIdx;
  }
  else
  {
    pageIdx1 = pageIdx2 = curPageIdx;
    if (pDoc->IsOnLeft(pageIdx1)
      && pageIdx1 + 1 < pDoc->GetPageCount()
      && pDoc->IsOnRight(pageIdx1 + 1))
    {
      pageIdx2 = pageIdx1 + 1;
    }
  }
}

int DviView::GetCurrentPageIdx() const
{
  if (g_pYapConfig->continuousView)
  {
    int sheetIdx = DetermineRowIdx(GetScrollPosition().y);
    if (g_pYapConfig->doublePage)
    {
      const DviDoc * pDoc = GetDocument();
      ASSERT_VALID(pDoc);
      return pDoc->DeterminePageIdx(sheetIdx, false);
    }
    else
    {
      return sheetIdx;
    }
  }
  else
  {
    return curPageIdx;
  }
}

int DviView::DetermineRowIdx(int y) const
{
  MIKTEX_ASSERT(g_pYapConfig->continuousView);
#if 0
  if (y < 0)
  {
    MIKTEX_UNEXPECTED();
  }
#endif
  const DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  int n = (g_pYapConfig->doublePage
    ? pDoc->GetRowCount()
    : pDoc->GetPageCount());
  if (n <= 0)
  {
    MIKTEX_UNEXPECTED();
  }
  int cy = pDoc->GetPaperSize().cy * n;
  cy += (GetTopMargin() + (n - 1) * GetVerticalInterPageOffset());
  if (y >= cy)
  {
    return n - 1;
  }
  return static_cast<int>((static_cast<double>(y) * n) / cy);
}

void DviView::OnFileDocumentProperties()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    CPropertySheet dlg(T_(_T("Document Properties")), 0, 0);
    DocumentFontsPage pageFonts;
    pageFonts.fontTable = pDoc->GetFontTable();
    dlg.AddPage(&pageFonts);
    dlg.DoModal();
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

void DviView::OnUpdateFileDocumentProperties(CCmdUI * pCmdUI)
{
  BOOL enable = FALSE;
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetDviFileStatus() == DviDoc::DVIFILE_LOADED)
    {
      enable = TRUE;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  pCmdUI->Enable(enable);
}

LRESULT DviView::OnMakeFonts(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(lParam);
  UNUSED_ALWAYS(wParam);
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (pDoc->GetPageCount() > 0)
    {
      curPageIdx = 0;
      CWaitCursor wait;
      pDoc->MakeFonts();
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  return 0;
}

size_t DviView::GetGammaTableIndex()
{
  if (gammaTable.size() == 0)
  {
    InitializeGammaTable();
  }
  size_t bestIdx = UINT_MAX;
  double bestd = 100000.0;
  for (size_t idx = 0; idx < gammaTable.size(); ++idx)
  {
    double d = fabs(g_pYapConfig->gamma - gammaTable[idx]);
    if (d < bestd)
    {
      bestIdx = idx;
      bestd = d;
    }
  }
  return bestIdx;
}

void DviView::OnLighterText()
{
  try
  {
    size_t idx = GetGammaTableIndex();
    MIKTEX_ASSERT(idx > 0 && idx < gammaTable.size());
    g_pYapConfig->gamma = gammaTable[idx - 1];
    UpdateAllDviViews();
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

void DviView::OnUpdateLighterText(CCmdUI * pCmdUI)
{
  try
  {
    size_t idx = GetGammaTableIndex();
    MIKTEX_ASSERT(idx >= 0 && idx < gammaTable.size());
    pCmdUI->Enable(idx > 0 && g_pYapConfig->dviPageMode != DviPageMode::Dvips);
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

void DviView::OnDarkerText()
{
  try
  {
    size_t idx = GetGammaTableIndex();
    MIKTEX_ASSERT(idx + 1 < gammaTable.size());
    g_pYapConfig->gamma = gammaTable[idx + 1];
    UpdateAllDviViews();
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

void DviView::OnUpdateDarkerText(CCmdUI * pCmdUI)
{
  try
  {
    size_t idx = GetGammaTableIndex();
    MIKTEX_ASSERT(idx >= 0 && idx < gammaTable.size());
    pCmdUI->Enable(idx + 1 < gammaTable.size()
      && g_pYapConfig->dviPageMode != DviPageMode::Dvips);
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

void DviView::InitializeGammaTable()
{
  gammaTable.clear();
  CSVList gammas(g_pYapConfig->gammaValues, _T(' '));
  while (gammas.GetCurrent() != 0)
  {
    gammaTable.push_back(atof(gammas.GetCurrent()));
    ++gammas;
  }
  if (gammaTable.size() == 0)
  {
    gammaTable.push_back(1.0);
  }
  sort(gammaTable.begin(), gammaTable.end());
}

void DviView::OnDviPageModePk()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    pDoc->SetDviPageMode(DviPageMode::Pk);
    pDoc->Reread();
    pDoc->UpdateAllViews(nullptr);
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

void DviView::OnUpdateDviPageModePk(CCmdUI * pCmdUI)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  pCmdUI->SetCheck(pDoc->GetDviPageMode() == DviPageMode::Pk);
}

void DviView::OnDviPageModeDvips()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    pDoc->SetDviPageMode(DviPageMode::Dvips);
    pDoc->Reread();
    pDoc->UpdateAllViews(0);
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

void DviView::OnUpdateDviPageModeDvips(CCmdUI * pCmdUI)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  pCmdUI->SetCheck(pDoc->GetDviPageMode() == DviPageMode::Dvips);
}
