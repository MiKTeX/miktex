/* draw.cpp: DVI drawing routines

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

#include "DviView.h"
#include "MainFrame.h"

void DviView::OnDraw(CDC * pDC)
{
  try
  {
    ASSERT_VALID(pDC);

    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    // do nothing if the DVI file could not be loaded
    DviDoc::DviFileStatus fileStatus = pDoc->GetDviFileStatus();
    if (fileStatus != DviDoc::DVIFILE_LOADED)
    {
      return;
    }

    // disable display updates while printing the document
    if (pDoc->IsPrintContext() && !pDC->IsPrinting())
    {
      return;
    }

    // prevent recursive invocations
    static bool insideOnDraw = false;
    if (insideOnDraw)
    {
      return;
    }
    AutoRestore<bool> restore_insideOnDraw(insideOnDraw);
    insideOnDraw = true;

    // find visible pages
    int pageIdx1;
    int pageIdx2;
    GetVisiblePages(pageIdx1, pageIdx2);
    if (pageIdx1 < 0)
    {
      return;
    }
    MIKTEX_ASSERT(pageIdx1 <= pageIdx2);

    // initialize glyph palettes
    if (gamma != g_pYapConfig->gamma)
    {
      gamma = g_pYapConfig->gamma;
      InitializeDviBitmapPalettes();
    }

    // select default glyph palette
    foreback fb;
    fb.fore = RGB(0, 0, 0);
    fb.back = RGB(255, 255, 255);
    fb.numcolors = (pDoc->GetShrinkFactor() == 1 ? 2 : 16);
    if (!SelectPalette(pDC->GetSafeHdc(), foregroundPalettes[fb], FALSE))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SelectPalette");
    }
    pDC->RealizePalette();

    // get the viewport origin
    CPoint ptViewportOrg(0, 0);
    if (!pDoc->IsPrintContext())
    {
      ptViewportOrg = -GetScrollPosition();
    }

    int savedDC = pDC->SaveDC();

    if (savedDC == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("SaveDC");
    }

    AutoRestoreDC autoRestoreDC(pDC, savedDC);

    CSize sizePage = pDoc->GetPaperSize();

    // loop visible pages
    for (int pageIdx = pageIdx1; pageIdx <= pageIdx2; ++pageIdx)
    {
      // calculate horizontal/vertical offsets
      CSize shift(0, 0);
      if (pDoc->IsPrintContext())
      {
        shift.cx -= pDC->GetDeviceCaps(PHYSICALOFFSETX);
        shift.cx += g_pYapConfig->pageXShift;
        shift.cy -= pDC->GetDeviceCaps(PHYSICALOFFSETY);
        shift.cy += g_pYapConfig->pageYShift;
      }
      else
      {
        shift.cx = GetLeftMargin();
        shift.cy = GetTopMargin();
        int n = (g_pYapConfig->continuousView
          ? (g_pYapConfig->doublePage
            ? pDoc->GetRowIdx(pageIdx)
            : pageIdx)
          : 0);
        if (n > 0)
        {
          shift.cy += n * sizePage.cy;
          shift.cy += n * GetVerticalInterPageOffset();
        }
      }

      // change the viewport origin
      CPoint ptOrigin = ptViewportOrg;
      ptOrigin += shift;
      if (!pDoc->IsPrintContext()
        && g_pYapConfig->doublePage
        && pDoc->IsOnRight(pageIdx))
      {
        ptOrigin.x += sizePage.cx;
        ptOrigin.x += GetHorizontalInterPageOffset();
      }
      pDC->SetViewportOrg(ptOrigin);

      // draw the page
      DrawPage(pDC, pageIdx);
    }
  }

  catch (const DrawingCancelledException &)
  {
    ;
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

void DviView::DrawPage(CDC * pDC, int pageIdx)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  // get the status of the DVI page
  PageStatus pageStatus = pDoc->GetPageStatus(pageIdx);

  if (pageStatus == PageStatus::Changed)
  {
    YapLog(T_("DVI document has been changed"));
  }

  bool pageLoaded = (pageStatus == PageStatus::Loaded);

  CWaitCursor * pWaitCursor = 0;

  // display a wait cursor if the page is not loaded already
  if (!pageLoaded && !pDoc->IsPrintContext())
  {
    pWaitCursor = new CWaitCursor;
  }

  auto_ptr<CWaitCursor> autoWaitCursor(pWaitCursor);

  // draw page edges
  if (!pDoc->IsPrintContext())
  {
    DrawRulers(pDC);
    DrawPaper(pDC);
  }

  // lock the DVI page
  DviPage * pPage = pDoc->GetLoadedPage(pageIdx);
  if (pPage == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  AutoUnlockPage autoUnlockPage(pPage);

  // render graphics
  if (g_pYapConfig->renderGraphicsInBackground)
  {
    RenderGraphicsInclusions(pDC, pPage);
  }

  // draw background rules
  DrawRules(pDC, true, pDoc, pPage);

  // draw DVI bitmaps
  DrawDviBitmaps(pDC, pDoc, pPage);

  // draw DIB chunks
  DrawDibChunks(pDC, pDoc, pPage);

  // draw foreground rules
  DrawRules(pDC, false, pDoc, pPage);

  // interpret non-graphics specials
  DrawSpecials(pDC, 3, pPage, pageIdx);

  if (!g_pYapConfig->renderGraphicsInBackground)
  {
    RenderGraphicsInclusions(pDC, pPage);
  }

  // draw search marker
  if (!pDoc->IsPrintContext() && pageSearchPosition == pageIdx)
  {
    DrawSourcePosition(pDC);
  }
}

void
DviView::DrawSpecials(CDC * pDC, int iteration, DviPage * pPage, int pageIdx)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  MIKTEX_ASSERT(pPage != nullptr);

  DviSpecial * pSpecial;

  for (int idx = 0; (pSpecial = pPage->GetSpecial(idx)) != nullptr; ++idx)
  {
    switch (iteration)
    {
    case 3:
      switch (pSpecial->GetType())
      {
      case DviSpecialType::Hypertex:
        if (g_pYapConfig->dviPageMode != DviPageMode::Dvips)
        {
          HypertexSpecial * pHypertexSpecial = reinterpret_cast<HypertexSpecial*>(pSpecial);
          if (!pDoc->IsPrintContext() && !pHypertexSpecial->IsName())
          {
            CPen pen;
            if (!pen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME)))
            {
              MIKTEX_FATAL_WINDOWS_ERROR("CreatePen");
            }
            AutoDeleteObject autoDeletePen(&pen);
            CPen * pOldPen = pDC->SelectObject(&pen);
            if (pOldPen == nullptr)
            {
              MIKTEX_UNEXPECTED();
            }
            AutoSelectObject autoSelectOldPen(pDC, pOldPen);
            int llx, lly, urx, ury;
            pHypertexSpecial->GetBbox(llx, lly, urx, ury);
            pDC->MoveTo(PixelShrink(llx), PixelShrink(lly) + 2);
            if (!pDC->LineTo(PixelShrink(urx) + 1, PixelShrink(lly) + 2))
            {
              MIKTEX_FATAL_WINDOWS_ERROR("LineTo");
            }
          }
        }
        break;
      case DviSpecialType::Source:
        if (!pDoc->IsPrintContext() && g_pYapConfig->showSourceSpecials)
        {
          CPen pen;
          if (!pen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW)))
          {
            MIKTEX_FATAL_WINDOWS_ERROR("CreatePen");
          }
          AutoDeleteObject autoDeletePen(&pen);
          CPen * pOldPen = pDC->SelectObject(&pen);
          if (pOldPen == nullptr)
          {
            MIKTEX_UNEXPECTED();
          }
          AutoSelectObject autoSelectOldPen(pDC, pOldPen);
          if (!pDC->Ellipse(PixelShrink(pSpecial->GetX()) - 4, PixelShrink(pSpecial->GetY()) - 4, PixelShrink(pSpecial->GetX()) + 5, PixelShrink(pSpecial->GetY()) + 5))
          {
            MIKTEX_FATAL_WINDOWS_ERROR("Ellipse");
          }
        }
        break;
      case DviSpecialType::Tpic:
        DrawTpicSpecial(pDC, reinterpret_cast<TpicSpecial*>(pSpecial));
        break;
      case DviSpecialType::SolidLine:
      {
        SolidLineSpecial * pSolidLineSpecial = reinterpret_cast<SolidLineSpecial*>(pSpecial);
        unsigned w = PixelShrink(pSolidLineSpecial->GetWidth());
        if (w == 0)
        {
          w = 1;
        }
        CPen pen;
        if (!pen.CreatePen(PS_SOLID, w, pSolidLineSpecial->GetColor()))
        {
          MIKTEX_FATAL_WINDOWS_ERROR("CreatePen");
        }
        AutoDeleteObject autoDeletePen(&pen);
        CPen * pOldPen = pDC->SelectObject(&pen);
        if (pOldPen == 0)
        {
          MIKTEX_UNEXPECTED();
        }
        AutoSelectObject autoSelectOldPen(pDC, pOldPen);
        pDC->MoveTo(PixelShrink(pSolidLineSpecial->GetStartX()), PixelShrink(pSolidLineSpecial->GetStartY()));
        if (!pDC->LineTo((PixelShrink(pSolidLineSpecial->GetEndX()) + 1), (PixelShrink(pSolidLineSpecial->GetEndY()) + 1)))
        {
          MIKTEX_FATAL_WINDOWS_ERROR("LineTo");
        }
      }
      break;
      }
    }
  }
}

void DviView::DrawPaper(CDC * pDC)
{
  ASSERT_VALID(pDC);

  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  int savedDC = pDC->SaveDC();

  if (savedDC == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SaveDC");
  }

  AutoRestoreDC autoRestoreDC(pDC, savedDC);

  CSize sizePage = pDoc->GetPaperSize();
  CRect rectPage(0, 0, sizePage.cx, sizePage.cy);

  // fill paper
  {
    CBrush brushWhite(0x00ffffff); // <fixme>use page background color<fixme/>
    AutoDeleteObject autoDeleteBrush(&brushWhite);
    CBrush * pOldBrush = pDC->SelectObject(&brushWhite);
    if (pOldBrush == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    AutoSelectObject autoSelectOldBrush(pDC, pOldBrush);
    if (!pDC->PatBlt(rectPage.left, rectPage.top, rectPage.Width(), rectPage.Height(), PATCOPY))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("PatBlt");
    }
  }

  MainFrame * pMain = reinterpret_cast<MainFrame*>((AfxGetApp())->m_pMainWnd);
  ASSERT_VALID(pMain);
  MIKTEX_ASSERT(pMain->IsKindOf(RUNTIME_CLASS(MainFrame)));

  // draw edges
  if (!pMain->IsFullScreen())
  {
    int lm = 0;
    int tm = 0;

    CBrush * pOldBrush = pDC->SelectObject(CBrush::FromHandle(reinterpret_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH))));

    if (pOldBrush == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }

    AutoSelectObject autoSelectOldBrush(pDC, pOldBrush);

    // top border
    if (!pDC->Rectangle(lm, tm - GetPageBorderWidth(), lm + sizePage.cx + GetPageBorderWidth(), tm))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("Rectangle");
    }

    // right border
    if (!pDC->Rectangle(lm + sizePage.cx, tm, lm + sizePage.cx + GetPageBorderWidth(), tm + sizePage.cy + GetPageBorderWidth()))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("Rectangle");
    }

    // bottom border
    if (!pDC->Rectangle(lm - GetPageBorderWidth(), tm + sizePage.cy, lm + sizePage.cx, tm + sizePage.cy + GetPageBorderWidth()))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("Rectangle");
    }

    // left border
    if (!pDC->Rectangle(lm - GetPageBorderWidth(), tm - GetPageBorderWidth(), lm, tm + sizePage.cy + GetPageBorderWidth()))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("Rectangle");
    }

    // right shadow
    if (!pDC->Rectangle(lm + sizePage.cx + GetPageBorderWidth(), tm + GetPageBorderShadow(), lm + sizePage.cx + GetPageBorderWidth() + GetPageBorderShadow(), tm + sizePage.cy + GetPageBorderWidth() + GetPageBorderShadow()))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("Rectangle");
    }

    // bottom shadow
    if (!pDC->Rectangle(lm + GetPageBorderShadow(), tm + sizePage.cy + GetPageBorderWidth(), lm + sizePage.cx + GetPageBorderWidth(), tm + sizePage.cy + GetPageBorderWidth() + GetPageBorderShadow()))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("Rectangle");
    }
  }
}

void DviView::DrawRulers(CDC * pDC)
{
  // TODO
  UNUSED_ALWAYS(pDC);
}

void DviView::DrawSourcePosition(CDC * pDC)
{
  MIKTEX_ASSERT(searchPosition.x >= 0 && searchPosition.y >= 0);
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  int savedDC = pDC->SaveDC();
  if (savedDC == 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("SaveDC");
  }
  AutoRestoreDC autoRestoreDC(pDC, savedDC);
  CPen pen;
  if (!pen.CreatePen(PS_SOLID, 3, GetSysColor(COLOR_BTNSHADOW)))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CreatePen");
  }
  AutoDeleteObject autoDeletePen(&pen);
  if (pDC->SelectObject(&pen) == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  if (!pDC->Ellipse(PixelShrink(searchPosition.x) - 4, PixelShrink(searchPosition.y) - 4, PixelShrink(searchPosition.x) + 5, PixelShrink(searchPosition.y) + 5))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("Ellipse");
  }
}

BOOL DviView::OnEraseBkgnd(CDC * pDC)
{
  try
  {
    ASSERT_VALID(pDC);
    MainFrame * pMain = reinterpret_cast<MainFrame*>((AfxGetApp())->m_pMainWnd);
    ASSERT_VALID(pMain);
    MIKTEX_ASSERT(pMain->IsKindOf(RUNTIME_CLASS(MainFrame)));
    CBrush brushBack(pMain->IsFullScreen() ? RGB(0, 0, 0) : GetSysColor(COLOR_APPWORKSPACE));
    CBrush * pOldBrush = pDC->SelectObject(&brushBack);
    if (pOldBrush == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    AutoSelectObject autoSelectOldBrush(pDC, pOldBrush);
    CRect rect;
    if (pDC->GetClipBox(&rect) == ERROR)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GetClipBox");
    }
    if (!pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("PatBlt");
    }
    return TRUE;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
}

void DviView::RenderGraphicsInclusions(CDC * pDC, DviPage * pPage)
{
  DviDoc * pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  int shrinkFactor = pDoc->GetShrinkFactor();
  int nGraphicsInclusions = pPage->GetNumberOfGraphicsInclusions(shrinkFactor);
  for (int idx = 0; idx < nGraphicsInclusions; ++idx)
  {
    pPage->GetGraphicsInclusion(shrinkFactor, idx)->Render(pDC->GetSafeHdc());
  }
}
