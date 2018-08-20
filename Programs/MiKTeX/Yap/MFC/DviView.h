/* DviView.h:                                           -*- C++ -*-

   Copyright (C) 1996-2018 Christian Schenk

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

#pragma once

#include "DviDoc.h"
#include "DviDraw.h"
#include "DviMagnifyingGlass.h"

class DrawingCancelledException :
  public MiKTeXException
{
public:
  MIKTEXTHISCALL DrawingCancelledException() :
    MiKTeXException(T_("Drawing cancelled."))
  {
  }
};

class SourceSpecialsDialog;
class DviMagnifyingGlass;

class DviView :
  public CScrollView,
  public DviDraw
{
protected:
  DECLARE_MESSAGE_MAP();

protected:
  DECLARE_DYNCREATE(DviView);

protected:
  DviView();

public:
  virtual ~DviView() override;

protected:
  void OnActivateView(BOOL activate, CView* pActivateView, CView* pDeactivateView) override;

protected:
  void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) override;

protected:
  void OnDraw(CDC* pDC) override;

protected:
  BOOL OnScrollBy(CSize sizeScroll, BOOL doScroll) override;

protected:
  void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) override;

protected:
  BOOL OnPreparePrinting(CPrintInfo* pInfo) override;

protected:
  void OnInitialUpdate() override;

protected:
  void OnPrint(CDC* pDC, CPrintInfo* pInfo) override;

protected:
  BOOL OnScroll(UINT scrollCode, UINT pos, BOOL doScroll) override;

protected:
  void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

protected:
  afx_msg void OnLighterText();

protected:
  afx_msg void OnUpdateLighterText(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateDarkerText(CCmdUI* pCmdUI);

protected:
  afx_msg void OnDarkerText();

protected:
  afx_msg void OnDviPageModePk();

protected:
  afx_msg void OnUpdateDviPageModePk(CCmdUI* pCmdUI);

protected:
  afx_msg void OnDviPageModeDvips();

protected:
  afx_msg void OnUpdateDviPageModeDvips(CCmdUI* pCmdUI);

protected:
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);

protected:
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT hitTest, UINT message);

protected:
  afx_msg LRESULT OnMakeFonts(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg void OnBack();

protected:
  afx_msg void OnChar(UINT chhar, UINT repeatCount, UINT flags);

protected:
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

protected:
  afx_msg void OnDocumentDown();

protected:
  afx_msg void OnDocumentUp();

protected:
  afx_msg void OnEnd();

protected:
  afx_msg void OnFileDocumentProperties();

protected:
  afx_msg void OnUpdateFileDocumentProperties(CCmdUI* pCmdUI);

protected:
  afx_msg void OnFileDvips();

protected:
  afx_msg void OnFirstPage();

protected:
  afx_msg void OnForward();

protected:
  afx_msg void OnGotoPage();

protected:
  afx_msg void OnHome();

protected:
  afx_msg void OnLButtonDblClk(UINT flags, CPoint point);

protected:
  afx_msg void OnLButtonDown(UINT flags, CPoint point);

protected:
  afx_msg void OnLButtonUp(UINT flags, CPoint point);

protected:
  afx_msg void OnLastPage();

protected:
  afx_msg void OnLineDown();

protected:
  afx_msg void OnLineLeft();

protected:
  afx_msg void OnLineRight();

protected:
  afx_msg void OnLineUp();

protected:
  afx_msg void OnMouseMove(UINT flags, CPoint point);

protected:
  afx_msg void OnNextPage();

protected:
  afx_msg void OnPageDown();

protected:
  afx_msg void OnPageEditor();

protected:
  afx_msg void OnPageLeft();

protected:
  afx_msg void OnPageRight();

protected:
  afx_msg void OnPageUp();

protected:
  afx_msg void OnPrevPage();

protected:
  afx_msg void OnToolsArrow();

protected:
  afx_msg void OnToolsHand();

protected:
  afx_msg void OnToolsMagnifyingGlass();

protected:
  afx_msg void OnToolsSourcespecials();

protected:
  afx_msg void OnUpdateBack(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateFirstPage(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateForward(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateLastPage(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateNextPage(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdatePrevPage(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateToolsArrow(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateToolsHand(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateToolsMagnifyingGlass(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateZoomIn(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateFileDvips(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdatePageEditor(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateZoomOut(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateToolsSourcespecials(CCmdUI* pCmdUI);

protected:
  afx_msg void OnUpdateGotoPage(CCmdUI* pCmdUI);

protected:
  afx_msg void OnViewRefresh();

protected:
  afx_msg void OnZoomIn();

protected:
  afx_msg void OnZoomOut();

public:
  bool IsZooming() const
  {
    return dynamic_cast<DviMagnifyingGlass*>(pToolWindow) != nullptr;
  }

public:
  int PixelShrink(int pxl) const
  {
    ASSERT_VALID(m_pDocument);
    return pxl / (reinterpret_cast<DviDoc*>(m_pDocument)->GetShrinkFactor());
  }

public:
  void CloseSourceSpecialsDialog();

public:
  DviDoc* GetDocument();

public:
  const DviDoc* GetDocument() const;

public:
  bool GetPageUnderCursor(int& pageIdx, int& x, int& y);

public:
  bool GetPoint(int& x, int& y);

public:
  bool GotoSrcSpecial(int line, const char* lpszFileName);

public:
  bool GetSource(PathName& fileName, int& line);

public:
  bool Navigate(const char* lpszUrl, bool remember = true);

private:
  bool warnPostScript = true;

private:
  struct location
  {
    int pageidx;
    CPoint pt;
  };

private:
  struct PrintRange
  {
    enum { All, Range } nRange;
    enum { EvenOdd, EvenOnly, OddOnly } nEvenOdd;
    int nFirst;
    int nLast;
  };

private:
  enum MouseTool { RulerTool = 0, Hand = 1, MagnifyingGlass = 2 };

private:
  int GetHorizontalInterPageOffset() const
  {
    return 6;
  }

private:
  int GetVerticalInterPageOffset() const
  {
    return 6;
  }

private:
  int GetPagePositionX(int pageIdx);

private:
  bool IsPointInView(CPoint pt);

private:
  int GetPagePositionY(int pageIdx);

private:
  int GetPageBorderWidth() const
  {
    return 1;
  }

private:
  int GetPageBorderShadow() const
  {
    return 2;
  }

private:
  int GetLeftMargin() const;

private:
  unsigned GetMode()
  {
    DviDoc* pDoc = GetDocument();
    ASSERT(pDoc != nullptr);
    return pDoc->IsPrintContext() ? DVIVIEW_PRINTER : DVIVIEW_DISPLAY;
  }

private:
  int GetRulerHeight() const
  {
    return 0;
  }

private:
  int GetRulerWidth() const
  {
    return 0;
  }

private:
  int GetTopMargin() const;

private:
  void ChangePage(int pageIdx);

private:
  void ClearSearchPosition();

private:
  void DrawPage(CDC* pDC, int pageIdx);

private:
  void DrawPaper(CDC* pDC);

private:
  void DrawRulers(CDC* pDC);

private:
  void DrawSourcePosition(CDC* pDC);

private:
  void DrawSpecials(CDC* pDC, int iteration, DviPage* pPage, int pageIdx);

private:
  void DrawTpicArc(CDC* pDC, TpicArcSpecial* pArc);

private:
  void DrawTpicEllipse(CDC* pDC, int x, int y, int xc, int yc, int xr, int yr);

private:
  void DrawTpicLine(CDC* pDC, int x, int y, const TpicSpecial::point& ptFrom, const TpicSpecial::point& ptTo);

private:
  void DrawTpicPoly(CDC* pDC, TpicPolySpecial* pPoly);

private:
  void DrawTpicSpecial(CDC* pDC, TpicSpecial* pTpicSpecial);

private:
  void DrawTpicSpline(CDC* pDC, TpicPolySpecial* pPoly);

private:
  bool FindDviFile(const char* lpszFileName, PathName& result);

private:
  void FlushTpicPath(CDC* pDC, int x, int y, const TpicSpecial::path& path);

private:
  void FlushTpicPathD(CDC* pDC, int x, int y, const TpicSpecial::path& path, double inchesPerDash, bool dotted);

private:
  void FlushTpicSpline(CDC* pDC, int x, int y, const TpicSpecial::path& path);

private:
  bool GetHyperTeXSpecialAtCursor(string& hyperTarget);

private:
  void GotoLocation(const location& loc);

private:
  void RenderGraphicsInclusions(CDC* pDC, DviPage* pPage);

private:
  bool IsOtherDviFileLabel(const char* lpszLabel, string& dviFileName, string& hashLabel);

private:
  void PrintPostScript(const char* lpszDviFileName, const char* lpszPrinterName, const DviView::PrintRange& printRange);

private:
  void RememberCurrentLocation(bool goingBack = false);

private:
  void Zoom(bool zoomIn);

private:
  CSize DeterminePageSize();

private:
  void GetVisiblePages(int& pageIdx1, int& pageIdx2) const;

public:
  int GetCurrentPageIdx() const;

private:
  int DetermineRowIdx(int y) const;

private:
  static void InitializeGammaTable();

private:
  static size_t GetGammaTableIndex();

private:
  stack<location> backLocations;

private:
  stack<location> forwardLocations;

private:
  int curPageIdx = -1;

private:
  CPoint cursorPosition;

private:
  CPoint searchPosition;

private:
  int pageSearchPosition;

private:
  SourceSpecialsDialog* pSourceSpecialDialog = nullptr;

private:
  MouseTool currentMouseTool;

private:
  CWnd* pToolWindow = nullptr;

private:
  bool mouseCaptured = false;

private:
  bool toolActive = false;

private:
  bool leftButtonDown = false;

private:
  CPrintInfo* pPrintInfo = nullptr;

private:
  double tpicConv;

private:
  static vector<double> gammaTable;

private:
  shared_ptr<Session> session = Session::Get();
};

inline const DviDoc* DviView::GetDocument() const
{
  return reinterpret_cast<const DviDoc*>(m_pDocument);
}

inline DviDoc* DviView::GetDocument()
{
  return reinterpret_cast<DviDoc*>(m_pDocument);
}

inline int UnPixelShrink(DviDoc* pDoc, int pxl)
{
  return pxl* pDoc->GetShrinkFactor();
}
