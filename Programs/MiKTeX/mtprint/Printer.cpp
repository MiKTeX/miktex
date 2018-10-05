/* BitmapPrinter.cpp:

   Copyright (C) 2003-2016 Christian Schenk

   This file is part of MTPrint.

   MTPrint is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   MTPrint is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MTPrint; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "Printer.h"

Printer::Printer(const PRINTINFO & printInfo, bool printNothing) :
  printInfo(printInfo),
  dryRun(printNothing),
  trace_mtprint(TraceStream::Open(MIKTEX_TRACE_MTPRINT))
{
}

Printer::~Printer()
{
  try
  {
    Finalize();
  }
  catch (const exception &)
  {
  }
}

DEVMODEW * Printer::GetDevMode(const char * lpszPrinterName)
{
  HANDLE hPrinter;
  wchar_t szPrinterName[_MAX_PATH];
  StringUtil::CopyString(szPrinterName, _MAX_PATH, lpszPrinterName);
  if (!OpenPrinterW(szPrinterName, &hPrinter, nullptr))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("OpenPrinterW", "printerName", lpszPrinterName);
  }
  AutoClosePrinter autoClosePrinter(hPrinter);
  long bytesNeeded = DocumentPropertiesW(nullptr, hPrinter, szPrinterName, nullptr, nullptr, 0);
  if (bytesNeeded < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("DocumentPropertiesW", "printerName", lpszPrinterName);
  }
  DEVMODEW * pDevMode = reinterpret_cast<DEVMODEW*>(malloc(bytesNeeded));
  if (pDevMode == nullptr)
  {
    OUT_OF_MEMORY("malloc");
  }
  if (DocumentPropertiesW(nullptr, hPrinter, szPrinterName, pDevMode, nullptr, DM_OUT_BUFFER) < 0)
  {
    free(pDevMode);
    MIKTEX_FATAL_WINDOWS_ERROR_2("DocumentPropertiesW", "printerName", lpszPrinterName);
  }
  return pDevMode;
}

PRINTER_INFO_2 * Printer::GetPrinterInfo(const char * lpszPrinterName, DEVMODE ** ppDevMode)
{
  HANDLE hPrinter;
  wchar_t szPrinterName[_MAX_PATH];
  StringUtil::CopyString(szPrinterName, _MAX_PATH, lpszPrinterName);
  if (!OpenPrinterW(szPrinterName, &hPrinter, nullptr))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("OpenPrinterW", "printerName", lpszPrinterName);
  }
  AutoClosePrinter autoClosePrinter(hPrinter);
  unsigned long bytesNeeded;
  if (!(GetPrinterW(hPrinter, 2, nullptr, 0, &bytesNeeded) || GetLastError() == ERROR_INSUFFICIENT_BUFFER))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetPrinterW", "printerName", lpszPrinterName);
  }
  PRINTER_INFO_2W * p2 = reinterpret_cast<PRINTER_INFO_2*>(malloc(bytesNeeded));
  if (p2 == nullptr)
  {
    OUT_OF_MEMORY("malloc");
  }
  AutoMemoryPointer autoFreeResult(p2);
  unsigned long bytesReturned;
  if (!GetPrinterW(hPrinter, 2, reinterpret_cast<BYTE*>(p2), bytesNeeded, &bytesReturned))
  {
    MIKTEX_FATAL_WINDOWS_ERROR_2("GetPrinterW", "printerName", lpszPrinterName);
  }
  if (ppDevMode != nullptr)
  {
    long bytesNeeded = DocumentPropertiesW(nullptr, hPrinter, szPrinterName, nullptr, nullptr, 0);
    if (bytesNeeded < 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("DocumentPropertiesW", "printerName", lpszPrinterName);
    }
    *ppDevMode = reinterpret_cast<DEVMODE*>(malloc(bytesNeeded));
    if (ppDevMode == nullptr)
    {
      OUT_OF_MEMORY("malloc");
    }
    if (DocumentPropertiesW(nullptr, hPrinter, szPrinterName, *ppDevMode, nullptr, DM_OUT_BUFFER) < 0)
    {
      free(*ppDevMode);
      MIKTEX_FATAL_WINDOWS_ERROR_2("DocumentProperties", "printerName", lpszPrinterName);
    }
  }
  autoFreeResult.Detach();
  return p2;
}

HDC Printer::CreateDC(const char * lpszPrinterName)
{
  DEVMODEW * pDevMode = 0;
  PRINTER_INFO_2W * p2 = GetPrinterInfo(lpszPrinterName, &pDevMode);
  AutoMemoryPointer autoFree1(p2);
  AutoMemoryPointer autoFree2(pDevMode);
  size_t size = pDevMode->dmSize + pDevMode->dmDriverExtra;
  DEVMODE * pDevMode2 = reinterpret_cast<DEVMODE*>(_alloca(size));
  memcpy(pDevMode2, p2->pDevMode, size);
#if 0
  trace_mtprint->WriteFormattedLine("mtprint", "CreateDC(\"%s\", \"%s\")", p2->pDriverName, p2->pPrinterName);
#endif
  HDC hdc = ::CreateDCW(p2->pDriverName, p2->pPrinterName, nullptr, pDevMode2);
  if (hdc == 0)
  {
    MIKTEX_FATAL_ERROR_2(T_("The device context could not be created."), "printerName", lpszPrinterName);
  }
  return hdc;
}

void Printer::CreateDC()
{
  hdcPrinter = CreateDC(printInfo.printerName.c_str());
}

void Printer::GetPrinterCaps(const char * lpszPrinterName, unsigned & resolution)
{
  HDC hdc = CreateDC(lpszPrinterName);
  resolution = static_cast<unsigned>(GetDeviceCaps(hdc, LOGPIXELSX));
  ::DeleteDC(hdc);
}

void Printer::StartJob()
{
  trace_mtprint->WriteLine("mtprint", "starting print job");
  CreateDC();
  DOCINFOW di;
  di.cbSize = sizeof(di);
  di.lpszDocName = L"My Document";
  di.lpszOutput = nullptr;
  di.lpszDatatype = nullptr;
  di.fwType = 0;
  if (!dryRun)
  {
    jobId = StartDocW(hdcPrinter, &di);
    if (jobId <= 0)
    {
      DeleteDC(hdcPrinter);
      hdcPrinter = nullptr;
      MIKTEX_FATAL_ERROR(T_("The print job could not be started."));
    }
  }
  jobStarted = true;
  pageStarted = false;
  currentPageNum = 0;
}

void Printer::EndJob()
{
  trace_mtprint->WriteLine(T_("mtprint"), "Ending print job");
  if (!dryRun)
  {
    ::EndDoc(hdcPrinter);
  }
  jobStarted = false;
}

void Printer::StartPage()
{
  currentPageNum += 1;
  trace_mtprint->WriteFormattedLine("mtprint", "starting page %u", currentPageNum);
  if (!dryRun)
  {
    if (::StartPage(hdcPrinter) <= 0)
    {
      MIKTEX_FATAL_ERROR_2(T_("The page could not be started."), "currentPage", std::to_string(static_cast<unsigned>(currentPageNum)));
    }
  }
  pageStarted = true;
  if (printInfo.pCallback != nullptr)
  {
    printInfo.pCallback->Report("[%u", currentPageNum);
  }
}

void Printer::EndPage()
{
  trace_mtprint->WriteLine("mtprint", "ending page");
  if (!dryRun)
  {
    ::EndPage(hdcPrinter);
  }
  if (printInfo.pCallback != nullptr)
  {
    printInfo.pCallback->Report("]");
  }
  pageStarted = false;
}

void Printer::Finalize()
{
  if (PageStarted())
  {
    EndPage();
  }
  if (JobStarted())
  {
    EndJob();
  }
  if (hdcPrinter != nullptr)
  {
    DeleteDC(hdcPrinter);
    hdcPrinter = nullptr;
  }
  if (trace_mtprint != nullptr)
  {
    trace_mtprint->Close();
    trace_mtprint = nullptr;
  }
}
