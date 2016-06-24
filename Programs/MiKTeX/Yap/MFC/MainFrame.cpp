/* MainFrame.cpp: main frame window

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

#include "AdvancedOptionsPage.h"
#include "DisplayOptionsPage.h"
#include "DviDoc.h"
#include "DviView.h"
#include "InverseSearchOptionsPage.h"
#include "MagnifyingGlassOptionsPage.h"
#include "MainFrame.h"
#include "PrintOptionsPage.h"
#include "SecurityOptionsPage.h"
#include "UIOptionsPage.h"
#include "winplace.h"
#include "yaphelp.h"

IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWnd);

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWnd)
  ON_COMMAND(ID_CANCEL, &MainFrame::OnCancel)
  ON_COMMAND(ID_CONTEXT_HELP, &CMDIFrameWnd::OnContextHelp)
  ON_COMMAND(ID_CONTINUOUS_DOUBLE, &MainFrame::OnContinuousDouble)
  ON_COMMAND(ID_CONTINUOUS_SINGLE, &MainFrame::OnContinuousSingle)
  ON_COMMAND(ID_DEFAULT_HELP, &CMDIFrameWnd::OnHelpFinder)
  ON_COMMAND(ID_DOUBLE_PAGE, &MainFrame::OnDoublePage)
  ON_COMMAND(ID_HELP, &CMDIFrameWnd::OnHelp)
  ON_COMMAND(ID_HELP_FINDER, &CMDIFrameWnd::OnHelpFinder)
  ON_COMMAND(ID_HELP_KEYBOARD, &MainFrame::OnHelpKeyboard)
  ON_COMMAND(ID_SINGLE_PAGE, &MainFrame::OnSinglePage)
  ON_COMMAND(ID_VIEW_FULLSCREEN, &MainFrame::OnViewFullScreen)
  ON_COMMAND(ID_VIEW_OPTIONS, &MainFrame::OnViewOptions)
  ON_UPDATE_COMMAND_UI(ID_CONTINUOUS_DOUBLE, &MainFrame::OnUpdateContinuousDouble)
  ON_UPDATE_COMMAND_UI(ID_CONTINUOUS_SINGLE, &MainFrame::OnUpdateContinuousSingle)
  ON_UPDATE_COMMAND_UI(ID_DOUBLE_PAGE, &MainFrame::OnUpdateDoublePage)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_COMMAND_PREFIX, &MainFrame::OnUpdateCommandPrefix)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_PAGE_M_OF_N, &MainFrame::OnUpdatePageMofN)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_POINT, &MainFrame::OnUpdatePoint)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_EFFECTIVE_PAGE_MODE, &MainFrame::OnUpdateEffectivePageMode)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_SOURCE, &MainFrame::OnUpdateSource)
  ON_UPDATE_COMMAND_UI(ID_SINGLE_PAGE, &MainFrame::OnUpdateSinglePage)
  ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, &MainFrame::OnUpdateViewFullScreen)
  ON_WM_CHAR()
  ON_WM_CLOSE()
  ON_WM_CREATE()
  ON_WM_GETMINMAXINFO()
  ON_COMMAND(ID_VIEW_CENTIMETERS, &MainFrame::OnViewCentimeters)
  ON_UPDATE_COMMAND_UI(ID_VIEW_CENTIMETERS, &MainFrame::OnUpdateViewCentimeters)
  ON_COMMAND(ID_VIEW_INCHES, &MainFrame::OnViewInches)
  ON_UPDATE_COMMAND_UI(ID_VIEW_INCHES, &MainFrame::OnUpdateViewInches)
  ON_COMMAND(ID_VIEW_MILLIMETERS, &MainFrame::OnViewMillimeters)
  ON_UPDATE_COMMAND_UI(ID_VIEW_MILLIMETERS, &MainFrame::OnUpdateViewMillimeters)
  ON_COMMAND(ID_VIEW_PICAS, &MainFrame::OnViewPicas)
  ON_UPDATE_COMMAND_UI(ID_VIEW_PICAS, &MainFrame::OnUpdateViewPicas)
  ON_COMMAND(ID_VIEW_POINTS, &MainFrame::OnViewPoints)
  ON_UPDATE_COMMAND_UI(ID_VIEW_POINTS, &MainFrame::OnUpdateViewPoints)
END_MESSAGE_MAP();

namespace {

  UINT
    indicators[] =
  {
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_COMMAND_PREFIX,
    ID_INDICATOR_SOURCE,
    ID_INDICATOR_EFFECTIVE_PAGE_MODE,
    ID_INDICATOR_POINT,
    ID_INDICATOR_PAGE_M_OF_N,
  };

}

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  try
  {
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
      return -1;
    }

    if (!toolBar.CreateEx(this,
      TBSTYLE_FLAT,
      (0
        | CBRS_FLYBY
        | CBRS_GRIPPER
        | CBRS_SIZE_DYNAMIC
        | CBRS_TOOLTIPS
        | CBRS_TOP
        | WS_CHILD
        | WS_VISIBLE
        | 0))
      || !toolBar.LoadToolBar(IDR_MAINFRAME))
    {
      MIKTEX_UNEXPECTED();
    }

    if (!statusBar.Create(this) || !statusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
    {
      MIKTEX_UNEXPECTED();
    }

    toolBar.SetBarStyle(toolBar.GetBarStyle()
      | CBRS_FLYBY
      | CBRS_SIZE_DYNAMIC
      | CBRS_TOOLTIPS
      | 0);

    toolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&toolBar);

    LoadBarState(_T("Settings"));

    return 0;
  }

  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(nullptr, e);
    return -1;
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(nullptr, e);
    return -1;
  }
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT & cs)
{
  SetTitle(T_(_T("Yap")));
  cs.lpszClass = YAP_WND_CLASS;
  return CMDIFrameWnd::PreCreateWindow(cs);
}

void MainFrame::OnViewOptions()
{
  try
  {
    CPropertySheet dlg(T_(_T("Options")), 0, 0);

#if 0
    dlg.EnableStackedTabs(FALSE);
#endif

    UIOptionsPage pageUI;
    dlg.AddPage(&pageUI);

    DisplayOptionsPage pageDisplay;
    dlg.AddPage(&pageDisplay);

    PrintOptionsPage pagePrint;
    dlg.AddPage(&pagePrint);

    MagnifyingGlassOptionsPage pageMag;
    dlg.AddPage(&pageMag);

    InverseSearchOptionsPage pageIsearch;
    dlg.AddPage(&pageIsearch);

    SecurityOptionsPage pageSec;
    dlg.AddPage(&pageSec);

    AdvancedOptionsPage pageAdvanced;
    dlg.AddPage(&pageAdvanced);

    if (dlg.DoModal() == IDOK)
    {
      UpdateAllDviViews(true);
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

void MainFrame::OnUpdatePageMofN(CCmdUI * pCmdUI)
{
  try
  {
    CMDIChildWnd * pChild = MDIGetActive();
    pCmdUI->Enable(pChild != nullptr);
    if (pChild == nullptr)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    CView * pView = pChild->GetActiveView();
    if (pView == nullptr || !pView->IsKindOf(RUNTIME_CLASS(DviView)))
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    DviView * pDviView = reinterpret_cast<DviView*>(pView);
    CDocument * pDoc = pChild->GetActiveDocument();
    DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
    if (pDviDoc->GetDviFileStatus() != DviDoc::DVIFILE_LOADED || pDviView->GetCurrentPageIdx() < 0)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    int m = pDviView->GetCurrentPageIdx() + 1;
    int n = pDviDoc->GetPageCount();
    CString str;
    str.Format(_T("Page: %s (%d%s of %d)"),
      static_cast<LPTSTR>(UT_(pDviDoc->GetPageName(pDviView->GetCurrentPageIdx()))),
      m,
      (m % 10 == 1
        ? (m % 100 == 11
          ? _T("th")
          : _T("st"))
        : (m % 10 == 2
          ? (m % 100 == 12
            ? _T("th")
            : _T("nd"))
          : (m % 10 == 3
            ? (m % 100 == 13
              ? _T("th")
              : _T("rd"))
            : _T("th")))),
      n);
    pCmdUI->SetText(str);
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

void MainFrame::OnUpdateCommandPrefix(CCmdUI * pCmdUI)
{
  pCmdUI->SetText(UT_(commandPrefix.c_str()));
}

void MainFrame::OnClose()
{
  if (!IsFullScreen())
  {
    SaveBarState(_T("Settings"));
    CWindowPlacement wp;
    wp.Save(_T("Settings"), this);
  }
  CMDIFrameWnd::OnClose();
}

void MainFrame::OnHelpKeyboard()
{
  AfxGetApp()->HtmlHelp(KEYBOARD_SHORTCUTS);
}

void MainFrame::OnUpdateSource(CCmdUI * pCmdUI)
{
  try
  {
    CMDIChildWnd * pChild = MDIGetActive();
    pCmdUI->Enable(pChild != nullptr);
    if (pChild == nullptr)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    CView * pView = pChild->GetActiveView();
    if (pView == nullptr || !pView->IsKindOf(RUNTIME_CLASS(DviView)))
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    DviView * pDviView = reinterpret_cast<DviView*>(pView);
    CDocument * pDoc = pChild->GetActiveDocument();
    DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
    if (pDviDoc->GetDviFileStatus() != DviDoc::DVIFILE_LOADED || pDviView->IsZooming())
    {
      // <fixme/>
      return;
    }
    PathName sourceFileName;
    int line;
    if (pDviView->GetSource(sourceFileName, line))
    {
      CString str;
      str.Format(_T("%s L:%d"), static_cast<LPTSTR>(UT_(sourceFileName.Get())), line);
      pCmdUI->SetText(str);
    }
    else
    {
      CString s;
      if (!s.LoadString(ID_INDICATOR_SOURCE))
      {
        MIKTEX_UNEXPECTED();
      }
      pCmdUI->SetText(s);
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

void MainFrame::OnUpdatePoint(CCmdUI * pCmdUI)
{
  try
  {
    CMDIChildWnd * pChild = MDIGetActive();
    pCmdUI->Enable(pChild != nullptr);
    if (pChild == nullptr)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    CView * pView = pChild->GetActiveView();
    if (pView == nullptr || !pView->IsKindOf(RUNTIME_CLASS(DviView)))
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    CDocument * pDoc = pChild->GetActiveDocument();
    DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
    int x, y;
    if (pDviDoc->GetDviFileStatus() != DviDoc::DVIFILE_LOADED
      || !reinterpret_cast<DviView*>(pView)->GetPoint(x, y))
    {
      pCmdUI->SetText(_T(""));
      return;
    }
#define pxl2bp(pxl) (((pxl) * 72.0) / pDviDoc->GetDisplayResolution())
#define pxl2cm(pxl) (((pxl) * 2.54) / pDviDoc->GetDisplayResolution())
#define pxl2in(pxl) (((pxl) * 1.00) / pDviDoc->GetDisplayResolution())
#define pxl2mm(pxl) (((pxl) * 25.4) / pDviDoc->GetDisplayResolution())
#define pxl2pc(pxl) (((pxl) * 12.0) / pDviDoc->GetDisplayResolution())
    double x2, y2;
    const char * lpszUnit = nullptr;
    switch (g_pYapConfig->unit)
    {
    case Unit::Centimeters:
      x2 = pxl2cm(x);
      y2 = pxl2cm(y);
      lpszUnit = "cm";
      break;
    case Unit::Inches:
      x2 = pxl2in(x);
      y2 = pxl2in(y);
      lpszUnit = T_("in");
      break;
    case Unit::Millimeters:
      x2 = pxl2mm(x);
      y2 = pxl2mm(y);
      lpszUnit = "mm";
      break;
    case Unit::Picas:
      x2 = pxl2pc(x);
      y2 = pxl2pc(y);
      lpszUnit = "pc";
      break;
    case Unit::BigPoints:
      x2 = pxl2bp(x);
      y2 = pxl2bp(y);
      lpszUnit = "pt";
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
    CString str;
    int precision = 2;
    str.Format(_T("%.*f,%.*f%s"), precision, x2, precision, y2, static_cast<LPTSTR>(UT_(lpszUnit)));
    pCmdUI->SetText(str);
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

void MainFrame::OnUpdateEffectivePageMode(CCmdUI * pCmdUI)
{
  try
  {
    CMDIChildWnd * pChild = MDIGetActive();
    pCmdUI->Enable(pChild != nullptr);
    if (pChild == nullptr)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    CDocument * pDoc = pChild->GetActiveDocument();
    DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pDoc);
    if (pDviDoc->GetDviFileStatus() != DviDoc::DVIFILE_LOADED)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    CView * pView = pChild->GetActiveView();
    if (pView == nullptr || !pView->IsKindOf(RUNTIME_CLASS(DviView)))
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    DviView * pDviView = reinterpret_cast<DviView*>(pView);
    int pageIdx;
    int x, y;
    if (!pDviView->GetPageUnderCursor(pageIdx, x, y))
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    if (pDviDoc->GetPageStatus(pageIdx) != PageStatus::Loaded)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    DviPage * pDviPage = pDviDoc->GetLoadedPage(pageIdx);
    if (pDviPage == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    AutoUnlockPage autoUnlockPage(pDviPage);
    if (pDviPage->GetDviPageMode() == DviPageMode::Dvips)
    {
      pCmdUI->SetText(_T("Dvips"));
    }
    else
    {
      pCmdUI->SetText(_T("Pk"));
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

bool MainFrame::AddCommandPrefixChar(char ch)
{
  bool done = true;
  if (isdigit(ch))
  {
    commandPrefix += ch;
  }
  else if (ch == '-')
  {
    if (!commandPrefix.empty() && commandPrefix[0] == '-')
    {
      commandPrefix = commandPrefix.substr(1, commandPrefix.length() - 1);
    }
    else
    {
      string tmp = commandPrefix;
      commandPrefix = "-";
      commandPrefix += tmp;
    }
  }
  else if (ch == VK_BACK && !commandPrefix.empty())
  {
    commandPrefix = commandPrefix.substr(0, commandPrefix.length() - 1);
  }
  else
  {
    done = false;
  }
  return done;
}

string MainFrame::GetCommandPrefix(bool clear)
{
  string result = commandPrefix;
  if (clear)
  {
    commandPrefix = "";
  }
  return result;
}

void MainFrame::OnContinuousSingle()
{
  try
  {
    g_pYapConfig->continuousView = true;
    g_pYapConfig->doublePage = false;
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

void MainFrame::OnUpdateContinuousSingle(CCmdUI * pCmdUI)
{
#if WIN95NOCONTVIEW
  bool supported = !g_pYapConfig->win95;
#else
  bool supported = true;
#endif
  pCmdUI->Enable(supported);
  pCmdUI->SetCheck((supported && g_pYapConfig->continuousView && !g_pYapConfig->doublePage) ? 1 : 0);
}

void MainFrame::OnContinuousDouble()
{
  try
  {
    g_pYapConfig->continuousView = true;
    g_pYapConfig->doublePage = true;
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

void MainFrame::OnUpdateContinuousDouble(CCmdUI * pCmdUI)
{
#if WIN95NOCONTVIEW
  bool supported = !g_pYapConfig->win95;
#else
  bool supported = true;
#endif
  pCmdUI->Enable(supported);
  pCmdUI->SetCheck((supported && g_pYapConfig->continuousView && g_pYapConfig->doublePage) ? 1 : 0);
}

void MainFrame::OnSinglePage()
{
  try
  {
    g_pYapConfig->continuousView = false;
    g_pYapConfig->doublePage = false;
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

void MainFrame::OnUpdateSinglePage(CCmdUI * pCmdUI)
{
  pCmdUI->SetCheck((!g_pYapConfig->doublePage && !g_pYapConfig->continuousView) ? 1 : 0);
}

void MainFrame::OnDoublePage()
{
  try
  {
    g_pYapConfig->continuousView = false;
    g_pYapConfig->doublePage = true;
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

void MainFrame::OnUpdateDoublePage(CCmdUI * pCmdUI)
{
  pCmdUI->SetCheck((g_pYapConfig->doublePage && !g_pYapConfig->continuousView) ? 1 : 0);
}

void MainFrame::OnViewFullScreen()
{
  try
  {
    CMDIChildWnd * pChild = MDIGetActive();

    if (pChild == nullptr)
    {
      return;
    }

    WINDOWPLACEMENT wpNew;

    if (!IsFullScreen())
    {

      statusBar.ShowWindow(SW_HIDE);
      toolBar.ShowWindow(SW_HIDE);

      previousWindowPlacement.length = sizeof(previousWindowPlacement);

      GetWindowPlacement(&previousWindowPlacement);

      RECT rectDesktop;

      if (!::GetWindowRect(::GetDesktopWindow(), &rectDesktop))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("GetWindowRect");
      }

      rectDesktop.top -= 2;
      rectDesktop.left -= 2;
      rectDesktop.bottom += 2;
      rectDesktop.right += 2;

      if (!::AdjustWindowRectEx(&rectDesktop, GetStyle(), TRUE, GetExStyle()))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("AdjustWindowRectEx");
      }

      fullScreenWindowRect = rectDesktop;

      wpNew.length = sizeof(wpNew);
      wpNew.showCmd = SW_SHOWNORMAL;
      wpNew.rcNormalPosition = rectDesktop;

      pFullScreenBar = new CToolBar;

      if (!pFullScreenBar->Create(this, CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLOATING) || !pFullScreenBar->LoadToolBar(IDR_FULLSCREEN))
      {
        MIKTEX_UNEXPECTED();
      }

      pFullScreenBar->EnableDocking(0);

      if (!pFullScreenBar->SetWindowPos(nullptr, 100, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW))
      {
        MIKTEX_UNEXPECTED();
      }

      pFullScreenBar->SetWindowText(T_(_T("Full Screen")));

      FloatControlBar(pFullScreenBar, CPoint(100, 100));

      isFullScreen = true;

      pChild->ModifyStyle(WS_CAPTION, 0, 0);
    }
    else
    {
      pFullScreenBar->DestroyWindow();
      delete pFullScreenBar;
      isFullScreen = false;
      statusBar.ShowWindow(SW_SHOWNORMAL);
      toolBar.ShowWindow(SW_SHOWNORMAL);
      wpNew = previousWindowPlacement;
      pChild->ModifyStyle(0, WS_CAPTION, 0);
    }

    if (!SetWindowPlacement(&wpNew))
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

void MainFrame::OnUpdateViewFullScreen(CCmdUI * pCmdUI)
{
  CMDIChildWnd * pChild = MDIGetActive();
  pCmdUI->Enable(pChild != nullptr);
  if (IsFullScreen())
  {
    pCmdUI->SetCheck();
  }
  else
  {
    pCmdUI->SetCheck(0);
  }
}

void MainFrame::OnGetMinMaxInfo(MINMAXINFO * lpMMI)
{
  if (IsFullScreen())
  {
    lpMMI->ptMaxSize.y = fullScreenWindowRect.Height();
    lpMMI->ptMaxTrackSize.y = lpMMI->ptMaxSize.y;
    lpMMI->ptMaxSize.x = fullScreenWindowRect.Width();
    lpMMI->ptMaxTrackSize.x = lpMMI->ptMaxSize.x;
  }
  CFrameWnd::OnGetMinMaxInfo(lpMMI);
}

void MainFrame::OnCancel()
{
  if (IsFullScreen())
  {
    OnViewFullScreen();
  }
  commandPrefix = "";
}

void MainFrame::OnViewCentimeters()
{
  g_pYapConfig->unit = Unit::Centimeters;
}

void MainFrame::OnUpdateViewCentimeters(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(TRUE);
  pCmdUI->SetCheck(g_pYapConfig->unit == Unit::Centimeters);
}

void MainFrame::OnViewInches()
{
  g_pYapConfig->unit = Unit::Inches;
}

void MainFrame::OnUpdateViewInches(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(TRUE);
  pCmdUI->SetCheck(g_pYapConfig->unit == Unit::Inches);
}

void MainFrame::OnViewMillimeters()
{
  g_pYapConfig->unit = Unit::Millimeters;
}

void MainFrame::OnUpdateViewMillimeters(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(TRUE);
  pCmdUI->SetCheck(g_pYapConfig->unit == Unit::Millimeters);
}

void MainFrame::OnViewPicas()
{
  g_pYapConfig->unit = Unit::Picas;
}

void MainFrame::OnUpdateViewPicas(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(TRUE);
  pCmdUI->SetCheck(g_pYapConfig->unit == Unit::Picas);
}

void MainFrame::OnViewPoints()
{
  g_pYapConfig->unit = Unit::BigPoints;
}

void MainFrame::OnUpdateViewPoints(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(TRUE);
  pCmdUI->SetCheck(g_pYapConfig->unit == Unit::BigPoints);
}
