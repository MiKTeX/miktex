/* DviDoc.cpp:

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

#include "DviDoc.h"
#include "DviView.h"
#include "MainFrame.h"

IMPLEMENT_DYNCREATE(DviDoc, CDocument);

BEGIN_MESSAGE_MAP(DviDoc, CDocument)
END_MESSAGE_MAP();

namespace {
  DviDoc * pLastDoc = nullptr;
}

DviDoc::DviDoc() :
  displayShrinkFactor(g_pYapConfig->displayShrinkFactor),
  dviPageMode(g_pYapConfig->dviPageMode)
{
  pLastDoc = this;
  modificationTime = 0;
  PaperSizeInfo paperSizeInfo;
  if (!pSession->GetPaperSizeInfo(-1, paperSizeInfo))
  {
    MIKTEX_UNEXPECTED();
  }
  dvipsPaperName = paperSizeInfo.dvipsName;
}

DviDoc::~DviDoc()
{
  try
  {
    MIKTEX_ASSERT(pDviSave == nullptr);
    MIKTEX_ASSERT(!isPrintContext);
    if (pDvi != nullptr)
    {
      delete pDvi;
      pDvi = nullptr;
      fileStatus = DVIFILE_NOT_LOADED;
    }
    if (pLastDoc == this)
    {
      pLastDoc = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

BOOL DviDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
  try
  {
    MIKTEX_ASSERT(pDvi == nullptr);
    MIKTEX_ASSERT(pDviSave == nullptr);
    MIKTEX_ASSERT(!isPrintContext);
    CreateDocument(TU_(lpszPathName));
    return TRUE;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(nullptr, e);
    return FALSE;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(nullptr, e);
    return FALSE;
  }
}

DviPage * DviDoc::GetLoadedPage(int pageIdx)
{
  MIKTEX_ASSERT(pDvi != nullptr);
  return pDvi->GetLoadedPage(pageIdx);
}

void DviDoc::BeginDviPrinting(const CDC * pPrinterDC)
{
  UNUSED_ALWAYS(pPrinterDC);
  MIKTEX_ASSERT(!isPrintContext);
  MIKTEX_ASSERT(pDviSave == nullptr);
  MIKTEX_ASSERT(pDvi != nullptr);
  isPrintContext = true;
  pDviSave = pDvi;
  pDvi = nullptr;
  try
  {
    CreateDocument(TU_(GetPathName()));
  }
  catch (const exception &)
  {
    this->pDvi = pDviSave;
    pDviSave = nullptr;
    isPrintContext = false;
    throw;
  }
}

void DviDoc::EndDviPrinting()
{
  MIKTEX_ASSERT(isPrintContext);
  MIKTEX_ASSERT(this->pDvi != nullptr);
  isPrintContext = false;
  Dvi * pDvi = this->pDvi;
  this->pDvi = pDviSave;
  pDviSave = nullptr;
  if (pDvi != nullptr)
  {
    delete pDvi;
  }
}

void DviDoc::CreateDocument(const char * lpszPathName)
{
  fileStatus = DVIFILE_NOT_LOADED;
  modificationTime = File::GetLastWriteTime(lpszPathName);
  MIKTEXMFMODE mfmode;
  if (!pSession->GetMETAFONTMode(GetMetafontMode(), &mfmode))
  {
    MIKTEX_UNEXPECTED();
  }
  MIKTEX_ASSERT(pDvi == nullptr);
  pDvi = Dvi::Create(lpszPathName,
    mfmode.szMnemonic,
    GetResolution(),
    GetShrinkFactor(),
    IsPrintContext() ? DviAccess::Sequential : DviAccess::Random,
    IsPrintContext() ? DviPageMode::Dvips : dviPageMode,
    pSession->GetPaperSizeInfo(dvipsPaperName.c_str()),
    landscape,
    this);
  pDvi->Scan();
  fileStatus = DVIFILE_LOADED;
}

void DviDoc::Reread()
{
  MIKTEX_ASSERT(!isPrintContext);
  Dvi * pDvi = this->pDvi;
  this->pDvi = nullptr;
  fileStatus = DVIFILE_NOT_LOADED;
  if (pDvi != nullptr)
  {
    delete pDvi;
  }
  CreateDocument(TU_(GetPathName()));
}

CSize DviDoc::GetPaperSize()
const
{
  MIKTEX_ASSERT(pDvi != nullptr);
  PaperSizeInfo size = pDvi->GetPaperSizeInfo();
  return CSize(pt2pxl(size.width), pt2pxl(size.height));
}

int DviDoc::GetDisplayResolution()
const
{
  MIKTEXMFMODE mfmode;
  MIKTEX_ASSERT(g_pYapConfig != nullptr);
  if (!pSession->GetMETAFONTMode(g_pYapConfig->displayMetafontMode, &mfmode))
  {
    MIKTEX_UNEXPECTED();
  }
  return mfmode.iHorzRes;
}

int DviDoc::GetPrinterResolution() const
{
  MIKTEXMFMODE mfmode;
  MIKTEX_ASSERT(g_pYapConfig != nullptr);
  if (!pSession->GetMETAFONTMode(g_pYapConfig->printerMetafontMode, &mfmode))
  {
    MIKTEX_UNEXPECTED();
  }
  return mfmode.iHorzRes;
}

void DviDoc::Shrink(int d)
{
  displayShrinkFactor += d;
}

void DviDoc::Unshrink()
{
  if (displayShrinkFactor > 1)
  {
    Shrink(-1);
  }
}

const char * DviDoc::GetPageName(unsigned pageIdx)
{
  MIKTEX_ASSERT(pDvi != nullptr);
  DviPage * pDviPage = pDvi->GetPage(pageIdx);
  if (pDviPage == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  AutoUnlockPage autoUnlockPage(pDviPage);
  return pDviPage->GetName();
}

int DviDoc::GetPageNum(int pageIdx) const
{
  MIKTEX_ASSERT(pDvi != nullptr);
  DviPage * pDviPage = pDvi->GetPage(pageIdx);
  if (pDviPage == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  AutoUnlockPage autoUnlockPage(pDviPage);
  return pDviPage->GetReg(0);
}

int DviDoc::GetMinPageNum() const
{
  MIKTEX_ASSERT(pDvi != nullptr);
  return pDvi->GetMinPageNumber();
}

int DviDoc::GetMaxPageNum() const

{
  MIKTEX_ASSERT(pDvi != nullptr);
  return pDvi->GetMaxPageNumber();
}

DWORD DviDoc::GetMetafontMode() const
{
  MIKTEX_ASSERT(g_pYapConfig != nullptr);
  return isPrintContext ? g_pYapConfig->printerMetafontMode : g_pYapConfig->displayMetafontMode;
}

bool DviDoc::IsPortrait() const
{
  return !landscape;
}

int DviDoc::SetDisplayShrinkFactor(int shrinkFactor)
{
  int oldDisplayShrinkFactor = displayShrinkFactor;
  displayShrinkFactor = shrinkFactor;
  return oldDisplayShrinkFactor;
}

DviPageMode DviDoc::SetDviPageMode(const DviPageMode & dviPageMode)
{
  DviPageMode old(this->dviPageMode);
  this->dviPageMode = dviPageMode;
  return old;
}

DviDoc * DviDoc::GetActiveDocument()
{
  ASSERT_VALID(AfxGetApp());
  MainFrame * pMain = reinterpret_cast<MainFrame*>((AfxGetApp())->m_pMainWnd);
  ASSERT_VALID(pMain);
  MIKTEX_ASSERT(pMain->IsKindOf(RUNTIME_CLASS(MainFrame)));
  CMDIChildWnd * pChild = pMain->MDIGetActive();
  ASSERT_VALID(pChild);
  DviDoc * pDviDoc = reinterpret_cast<DviDoc*>(pChild->GetActiveDocument());
  if (pDviDoc == nullptr)
  {
    pDviDoc = pLastDoc;
  }
  MIKTEX_ASSERT(pDviDoc != nullptr);
  MIKTEX_ASSERT(pDviDoc->IsKindOf(RUNTIME_CLASS(DviDoc)));
  return pDviDoc;
}

PathName DviDoc::GetDocDir()
{
  PathName result = GetPathName();
  result.RemoveFileSpec();
  return result;
}

int DviDoc::FindPage(int pageNum) const
{
  for (int pageIdx = 0; pageIdx < GetPageCount(); ++pageIdx)
  {
    if (GetPageNum(pageIdx) == pageNum)
    {
      return pageIdx;
    }
  }
  return -1;
}

void DviDoc::OnIdle()
{
  try
  {
    CDocument::OnIdle();
    DviFileStatus oldStatus = fileStatus;
    DviFileStatus newStatus = GetDviFileStatus();
    if (newStatus == DVIFILE_MODIFIED)
    {
      YapLog(T_("document has been modified"));
      POSITION posView = GetFirstViewPosition();
      while (posView != nullptr)
      {
        CView * pView = GetNextView(posView);
        if (pView->IsKindOf(RUNTIME_CLASS(DviView)))
        {
          DviView * pDviView = reinterpret_cast<DviView*>(pView);
          if (!pDviView->PostMessage(WM_COMMAND, ID_VIEW_REFRESH))
          {
            // unexpected
          }
        }
      }
    }
    else if (pDvi != nullptr)
    {
      pSession->UnloadFilenameDatabase(); // FIXME
    }
  }
  catch (const exception &)
  {
  }
}

DviDoc::DviFileStatus DviDoc::GetDviFileStatus()
{
  if (fileStatus == DVIFILE_LOADED)
  {
    if (!File::Exists(PathName(GetPathName())))
    {
      fileStatus = DVIFILE_LOST;
    }
    else
    {
      time_t timeMod = modificationTime;
      modificationTime = File::GetLastWriteTime(PathName(GetPathName()));
      if (timeMod != modificationTime)
      {
        YapLog(T_("%s has been modified"), Q_(TU_(GetPathName())));
        modificationTime = timeMod;
        fileStatus = DVIFILE_MODIFIED;
      }
    }
  }
  return fileStatus;
}

void DviDoc::OnProgress(DviNotification nf)
{
  if (nf == DviNotification::BeginLoadFont && makingFonts)
  {
    MIKTEX_ASSERT(pDvi != nullptr);
    string statusText = pDvi->GetStatusText();
    if (statusText.empty())
    {
      return;
    }
    if (pProgressDialog == nullptr)
    {
      pProgressDialog = ProgressDialog::Create();
      pProgressDialog->StartProgressDialog(AfxGetMainWnd()->GetSafeHwnd());
      pProgressDialog->SetTitle("Yap");
      pProgressDialog->SetLine(1, T_("Yap is creating font bitmaps:"));
    }
    pProgressDialog->SetLine(2, statusText.c_str());
  }
}

void DviDoc::MakeFonts()
{
  MIKTEX_ASSERT(pDvi != nullptr);
  bool done = true;
  try
  {
    AutoRestore<bool> autoRestore(makingFonts);
    makingFonts = true;
    done = pDvi->MakeFonts();
  }
  catch (const exception &)
  {
    if (pProgressDialog != nullptr)
    {
      pProgressDialog->StopProgressDialog();
      delete pProgressDialog;
      pProgressDialog = nullptr;
    }
    throw;
  }
  if (pProgressDialog != nullptr)
  {
    pProgressDialog->StopProgressDialog();
    delete pProgressDialog;
    pProgressDialog = nullptr;
  }
  if (!done)
  {
    MIKTEX_FATAL_ERROR(T_("Not all font bitmaps could be created. See 'File->Document Properties', for details."));
  }
}
