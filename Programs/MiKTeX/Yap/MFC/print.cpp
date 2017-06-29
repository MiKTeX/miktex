/* print.cpp: DVI printing

   Copyright (C) 1996-2017 Christian Schenk

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
#include "Dvidoc.h"
#include "DvipsDialog.h"

BOOL DviView::OnPreparePrinting(CPrintInfo * pInfo)
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    pInfo->m_nCurPage = GetCurrentPageIdx() + 1;
    pInfo->SetMinPage(1);
    pInfo->SetMaxPage(pDoc->GetPageCount());

    if (!DoPreparePrinting(pInfo))
    {
      return FALSE;
    }

    if (pInfo->m_bPreview)
    {
      MIKTEX_UNEXPECTED();
    }

    int dpi = GetDeviceCaps(pInfo->m_pPD->m_pd.hDC, LOGPIXELSX);
    if (dpi != pDoc->GetPrinterResolution())
    {
      string str = std::to_string(dpi);
      str += " vs. ";
      str += std::to_string(pDoc->GetPrinterResolution());
      MIKTEX_FATAL_ERROR_2(T_("METAFONT mode mismatch."), "info", str);
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

void DviView::OnBeginPrinting(CDC * pDC, CPrintInfo * pInfo)
{
  try
  {
    CScrollView::OnBeginPrinting(pDC, pInfo);
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    pDoc->BeginDviPrinting(pDC);
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

void DviView::OnEndPrinting(CDC * pDC, CPrintInfo * pInfo)
{
  try
  {
    CScrollView::OnEndPrinting(pDC, pInfo);
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    pDoc->EndDviPrinting();
    pPrintInfo = nullptr;
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

void
DviView::OnFileDvips()
{
  try
  {
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    DvipsDialog dlg(this);
    dlg.m_iFirstPage = pDoc->GetPageNum(0);
    dlg.m_iLastPage = pDoc->GetPageNum(pDoc->GetPageCount() - 1);
    dlg.m_nPageRange = 0;
    if (dlg.DoModal() != IDOK)
    {
      return;
    }
    PrintRange pr;
    switch (dlg.m_nPageRange)
    {
    case 0:                 // all
      pr.nRange = PrintRange::All;
      break;
    case 1:                 // current
      pr.nRange = PrintRange::Range;
      pr.nFirst = pr.nLast = GetCurrentPageIdx() + 1;
      break;
    case 2:                 // range
      pr.nRange = PrintRange::Range;
      pr.nFirst = dlg.m_iFirstPage;
      pr.nLast = dlg.m_iLastPage;
      break;
    }
    switch (dlg.m_nEvenOdd)
    {
    case 0:
      pr.nEvenOdd = PrintRange::EvenOdd;
      break;
    case 1:
      pr.nEvenOdd = PrintRange::EvenOnly;
      break;
    case 2:
      pr.nEvenOdd = PrintRange::OddOnly;
      break;
    }
    PrintPostScript(TU_(pDoc->GetPathName()), dlg.GetPrinterName(), pr);
    g_pYapConfig->Save();
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

void DviView::OnUpdateFileDvips(CCmdUI * pCmdUI)
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

void DviView::PrintPostScript(const char * lpszDviFileName, const char * lpszPrinterName, const DviView::PrintRange & pr)
{
  // locate mtprint.exe
  PathName mtprint;
  if (!session->FindFile("mtprint.exe", FileType::EXE, mtprint))
  {
    MIKTEX_UNEXPECTED();
  }

  Utils::RemoveBlanksFromPathName(mtprint);

  // make command-line
  vector<string> args{ "mtprint" };
  args.push_back("--printer="s + lpszPrinterName);
  args.push_back("--print-method="s + "ps");
  switch (pr.nRange)
  {
  case PrintRange::All:
    break;
  case PrintRange::Range:
    string str(std::to_string(pr.nFirst));
    str += "-";
    str += std::to_string(pr.nLast);
    args.push_back("--page-range="s + str);
    break;
  }
  switch (pr.nEvenOdd)
  {
  case PrintRange::EvenOnly:
    args.push_back("--even-only");
    break;
  case PrintRange::OddOnly:
    args.push_back("--odd-only");
    break;
  }
  args.push_back(lpszDviFileName);

  unique_ptr<ProgressDialog> pProgDlg(ProgressDialog::Create());
  pProgDlg->StartProgressDialog(GetSafeHwnd());
  pProgDlg->SetTitle(T_("Print"));
  pProgDlg->SetLine(1, T_("Being printed:"));
  pProgDlg->SetLine(2, lpszDviFileName);
  ProcessOutput<4096> processOutput;
  int exitCode;
  bool done = Process::Run(mtprint, args, &processOutput, &exitCode, nullptr);
  pProgDlg->StopProgressDialog();
  pProgDlg.reset();

  if (!done)
  {
    MIKTEX_UNEXPECTED();
  }

  if (exitCode != 0)
  {
    MIKTEX_FATAL_ERROR(T_("The MiKTeX print utility failed for some reason."));
  }
}

void DviView::OnPrint(CDC * pDC, CPrintInfo * pInfo)
{
  try
  {
    pPrintInfo = pInfo;
    CScrollView::OnPrint(pDC, pInfo);
    DviDoc * pDoc = GetDocument();
    ASSERT_VALID(pDoc);
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

void DviView::OnUpdateFilePrint(CCmdUI * pCmdUI)
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
