/* DvipsDialog.cpp:

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

#include "DvipsDialog.h"

BEGIN_MESSAGE_MAP(DvipsDialog, CDialog)
  ON_BN_CLICKED(IDC_PAGE_RANGE, OnPageRange)
  ON_EN_CHANGE(IDC_FIRST_PAGE, OnChangeFirstPage)
  ON_EN_CHANGE(IDC_LAST_PAGE, OnChangeLastPage)
END_MESSAGE_MAP();

DvipsDialog::DvipsDialog(CWnd * pParent) :
  CDialog(IDD, pParent)
{
}

BOOL DvipsDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();

  try
  {
    // get printer names
    DWORD needed = 0, nPrinters = 0;
    if (!(EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 1, nullptr, 0, &needed, &nPrinters) || GetLastError() == ERROR_INSUFFICIENT_BUFFER))
    {
      MIKTEX_FATAL_WINDOWS_ERROR(T_("EnumPrinters"));
    }

    unsigned char * lpBuf = new unsigned char[needed];

    try
    {
      if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 1, lpBuf, needed, &needed, &nPrinters))
      {
        MIKTEX_FATAL_WINDOWS_ERROR(T_("EnumPrinters"));
      }

      for (DWORD index = 0; index < nPrinters; ++index)
      {
        LPPRINTER_INFO_1 lppi1 = reinterpret_cast<LPPRINTER_INFO_1>(lpBuf + index * sizeof(PRINTER_INFO_1));
        HANDLE hPrinter;
        if (!OpenPrinter(lppi1->pName, &hPrinter, nullptr))
        {
#if 1
          // TODO: TraceStream::TraceLastWin32Error("OpenPrinter", TU_(lppi1->pName), __FILE__, __LINE__);
          continue;
#else
          FATAL_WINDOWS_ERROR(T_("OpenPrinter"), 0);
#endif
        }
        AutoClosePrinter  autoClosePrinter(hPrinter);
        DWORD needed2 = 0;
        if (!(GetPrinter(hPrinter, 2, nullptr, 0, &needed2) || GetLastError() == ERROR_INSUFFICIENT_BUFFER))
        {
          MIKTEX_FATAL_WINDOWS_ERROR(T_("GetPrinter"));
        }
        PRINTER_INFO_2 * lppi2 = reinterpret_cast<LPPRINTER_INFO_2>(GlobalAlloc(GPTR, needed2));
        AutoGlobalMemory autoGlobalFree(lppi2);
        DWORD returned;
        if (!GetPrinter(hPrinter, 2, reinterpret_cast<LPBYTE>(lppi2), needed2, &returned))
        {
          MIKTEX_FATAL_WINDOWS_ERROR(T_("GetPrinter"));
        }
        if (m_comboPrinterName.AddString(lppi2->pPrinterName) < 0)
        {
          MIKTEX_UNEXPECTED();
        }
      }
    }

    catch (const exception &)
    {
      delete[] lpBuf;
      throw;
    }

    delete[] lpBuf;

    char szDefaultPrinter[300];
    size_t size = 300;
    if (Utils::GetDefPrinter(szDefaultPrinter, &size))
    {
      if (m_comboPrinterName.SelectString(-1, UT_(szDefaultPrinter)) < 0)
      {
        MIKTEX_UNEXPECTED();
      }
    }

    // trigger detection of Ghostscript
    PathName path;
    session->GetGhostscript(path.GetData(), 0);
  }

  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }

  return ret;
}

void DvipsDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_CBIndex(pDX, IDC_COMBO_PRINTER_NAME, m_nPrinterName);
  DDX_Control(pDX, IDC_COMBO_PRINTER_NAME, m_comboPrinterName);
  DDX_Control(pDX, IDC_FIRST_PAGE, m_editFirst);
  DDX_Radio(pDX, IDC_ALL_PAGES, m_nPageRange);
  DDX_Radio(pDX, IDC_EVEN_ODD_PAGES, m_nEvenOdd);
  DDX_Text(pDX, IDC_FIRST_PAGE, m_iFirstPage);
  DDX_Text(pDX, IDC_LAST_PAGE, m_iLastPage);

  if (pDX->m_bSaveAndValidate)
  {
    if (m_nPrinterName >= 0)
    {
      m_comboPrinterName.GetWindowText(m_strPrinterName);
    }
    else
    {
      m_strPrinterName = _T("");
    }
  }
}

void DvipsDialog::OnPageRange()
{
  m_editFirst.SetFocus();
}

void DvipsDialog::OnChangeFirstPage()
{
  CheckRadioButton(IDC_ALL_PAGES, IDC_PAGE_RANGE, IDC_PAGE_RANGE);
}

void DvipsDialog::OnChangeLastPage()
{
  CheckRadioButton(IDC_ALL_PAGES, IDC_PAGE_RANGE, IDC_PAGE_RANGE);
}
