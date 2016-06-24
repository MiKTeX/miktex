/* MainFrame.cpp:

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "stdafx.h"
#include "mpm.h"

#include "MainFrame.h"
#include "MpmDoc.h"
#include "MpmView.h"

IMPLEMENT_DYNCREATE(MainFrame, CFrameWnd);

BEGIN_MESSAGE_MAP(MainFrame, CFrameWnd)
  ON_WM_CREATE()
  ON_UPDATE_COMMAND_UI(IDOK, &MainFrame::OnUpdateFilterOut)
  ON_COMMAND(ID_EDIT_PASTE, &MainFrame::OnEditPaste)
  ON_COMMAND(ID_EDIT_CUT, &MainFrame::OnEditCut)
  ON_COMMAND(ID_EDIT_COPY, &MainFrame::OnEditCopy)
  ON_COMMAND(ID_EDIT_UNDO, &MainFrame::OnEditUndo)
  ON_UPDATE_COMMAND_UI(ID_INDICATOR_NUM_PACKAGES, &MainFrame::OnUpdateNumPackages)
END_MESSAGE_MAP();

namespace {

  UINT indicators[] =
  {
    ID_SEPARATOR, ID_INDICATOR_NUM_PACKAGES,
  };

}

int MainFrame::OnCreate(LPCREATESTRUCT pCreateStruct)
{
  try
  {
    if (CFrameWnd::OnCreate(pCreateStruct) < 0)
    {
      MIKTEX_UNEXPECTED();
    }

    if (!toolBar.CreateEx(this) || !toolBar.LoadToolBar(IDR_MAINFRAME))
    {
      MIKTEX_UNEXPECTED();
    }

    if (!dialogBar.Create(this, IDR_MAINFRAME, CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
    {
      MIKTEX_UNEXPECTED();
    }

    if (!reBar.Create(this)
      || !reBar.AddBar(&toolBar)
      || !reBar.AddBar(&dialogBar))
    {
      MIKTEX_UNEXPECTED();
    }

    if (!statusBar.Create(this)
      || !statusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
    {
      MIKTEX_UNEXPECTED();
    }

    toolBar.SetBarStyle(toolBar.GetBarStyle()
      | CBRS_TOOLTIPS
      | CBRS_FLYBY);

    return 0;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return -1;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return -1;
  }
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT & cs)
{
  try
  {
    if (!CFrameWnd::PreCreateWindow(cs))
    {
      MIKTEX_UNEXPECTED();
    }
    cs.style &= ~FWS_ADDTOTITLE;
    if (pSession->IsAdminMode())
    {
      SetTitle(T_(_T("MiKTeX Package Manager (Admin)")));
    }
    else
    {
      SetTitle(T_(_T("MiKTeX Package Manager")));
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

void MainFrame::OnUpdateFilterOut(CCmdUI * pCmdUI)
{
  pCmdUI->Enable(TRUE);
}

void MainFrame::OnUpdateNumPackages(CCmdUI * pCmdUI)
{
  try
  {
    CView * pView = GetActiveView();
    pCmdUI->Enable(pView != nullptr);
    if (pView == nullptr)
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    if (!pView->IsKindOf(RUNTIME_CLASS(MpmView)))
    {
      pCmdUI->SetText(_T(""));
      return;
    }
    MpmView * pMpmView = reinterpret_cast<MpmView*>(pView);
    CString str;
    str.Format(T_(_T("Total: %d")), pMpmView->GetNumberOfPackages());
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

void MainFrame::OnEditCut()
{
  CWnd * pWnd = GetFocus();
  if (pWnd != nullptr)
  {
    pWnd->SendMessage(WM_CUT, 0, 0);
  }
}

void MainFrame::OnEditPaste()
{
  CWnd * pWnd = GetFocus();
  if (pWnd != nullptr)
  {
    pWnd->SendMessage(WM_PASTE, 0, 0);
  }
}

void MainFrame::OnEditCopy()
{
  CWnd * pWnd = GetFocus();
  if (pWnd != nullptr)
  {
    pWnd->SendMessage(WM_COPY, 0, 0);
  }
}

void MainFrame::OnEditUndo()
{
  CWnd * pWnd = GetFocus();
  if (pWnd != nullptr)
  {
    pWnd->SendMessage(WM_UNDO, 0, 0);
  }
}
