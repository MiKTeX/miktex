/* Printer.h:                                           -*- C++ -*-

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

#pragma once

class IPrinterCallback
{
public:
  virtual void Report(const char * lpszFormat, ...) = 0;
};

class Printer
{
public:
  virtual ~Printer();

public:
  struct PRINTINFO
  {
    string printerName;
    IPrinterCallback * pCallback;
  };

public:
  Printer()
  {
  }

public:
  Printer(const PRINTINFO & printInfo, bool printNothing = false);

public:
  static PRINTER_INFO_2W * GetPrinterInfo(const char * lpszPrinterName, DEVMODEW ** ppDevMode);

public:
  static DEVMODEW * GetDevMode(const char * lpszPrinterName);

public:
  static void GetPrinterCaps(const char * lpszPrinterName, unsigned & resolution);

protected:
  void StartJob();

protected:
  void EndJob();

protected:
  void StartPage();

protected:
  void EndPage();

protected:
  void Finalize();

protected:
  bool JobStarted() const
  {
    return jobStarted;
  }

protected:
  bool PageStarted() const
  {
    return pageStarted;
  }

protected:
  HDC GetDC() const
  {
    return hdcPrinter;
  }

private:
  static HDC CreateDC(const char * lpszPrinterName);

private:
  void CreateDC();

private:
  HDC hdcPrinter = nullptr;

private:
  bool jobStarted = false;

private:
  bool pageStarted = false;

private:
  int jobId;

private:
  PRINTINFO printInfo;

private:
  size_t currentPageNum;

protected:
  bool dryRun = false;

protected:
  unique_ptr<TraceStream> trace_mtprint;
};
