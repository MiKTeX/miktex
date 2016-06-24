/* mtprint.cpp: MiKTeX Print Utility

   Copyright (C) 2003-2016 Christian Schenk

   This file is part of MiKTeX Print Utility.

   MiKTeX Print Utility is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Print Utility is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Print Utility; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "BitmapPrinter.h"

#ifndef THE_NAME_OF_THE_GAME
#  define THE_NAME_OF_THE_GAME T_("MiKTeX Print Utility")
#endif

const char * DEFAULT_TRACE_STREAMS = MIKTEX_TRACE_ERROR "," MIKTEX_TRACE_MTPRINT;

struct PAPERSIZEINFO
{
  short paperSize;
  const char * lpszName;
  const char * lpszDvipsSize;
  unsigned width;
  unsigned height;
};

struct DVIPSOPTS
{
  struct PAGERANGE
  {
    int firstPage, lastPage;
  };
  bool evenPagesOnly = false;
  bool oddPagesOnly = false;
  bool runAsFilter = false ;
  bool runQuietly = false;
  bool sendCtrlDAtEnd = false;
  bool shiftLowCharsToHigherPos = false;
  string config;
  string offsetX;
  string offsetY;
  string paperFormat;
  vector<PAGERANGE> pageRanges;
};

struct GSOPTS
{
  GSOPTS()
  {
  }
};

class PrintUtility :
  public Application,
  public IPrinterCallback
{
private:
  enum PrintMethod {
    PrintMethodPostScript,
    PrintMethodPostScriptBMP
  };

public:
  PrintUtility() :
    trace_mtprint(TraceStream::Open(MIKTEX_TRACE_MTPRINT)),
    trace_error(TraceStream::Open(MIKTEX_TRACE_ERROR))
  {
  }

public:
  virtual ~PrintUtility()
  {
    try
    {
      trace_mtprint->Close();
      trace_error->Close();
    }
    catch (const exception &)
    {
    }
  }

public:
  virtual void Report(const char * lpszFormat, ...);

public:
  void Run(int argc, const char ** argv);

private:
  void ShowVersion();

private:
  bool GetPaperSizeInfo(short paperSize, PAPERSIZEINFO & psi);

private:
  void StartDvips(const char * lpszDviFileName, const DVIPSOPTS & dvipsOpts, unsigned resolution, const char * lpszPrinterName, short paperSize, FILE * * ppfileDvipsOutRd = 0, FILE * * ppfileDvipsErrRd = 0);

private:
  void StartGhostscript(const GSOPTS & gsOpts, unsigned resolution, short paperSize, FILE * pfileGsIn, FILE ** ppfileGsOut);

private:
  void Spool(const char * lpszFileName, PrintMethod printMethod, const DVIPSOPTS & dvipsOpts, const GSOPTS & gsOpts, const string & printerName);

private:
  string printerName;

private:
  PrintMethod printMethod = PrintMethodPostScriptBMP;

private:
  bool verbose = false;

private:
  bool dryRun = false;

private:
  unique_ptr<TraceStream> trace_mtprint;

private:
  unique_ptr<TraceStream> trace_error;

private:
  static const PAPERSIZEINFO paperSizes[];

private:
  static const struct poptOption aoption[];
};

#define mm2pt(mm) static_cast<int>((mm) * (1.0 / 25.4) * 72.0)
#define in2pt(inch) static_cast<int>((inch) * 72.0)

#define MM2PT(a, b) #a "mm," #b "mm", mm2pt(a), mm2pt(b)
#define IN2PT(a, b) #a "in," #b "in", in2pt(a), in2pt(b)

#define INCHPAPER(a, b, c, d) DMPAPER_##a, b, IN2PT(c, d)
#define MMPAPER(a, b, c, d) DMPAPER_##a, b, MM2PT(c, d)

const PAPERSIZEINFO PrintUtility::paperSizes[] = {
  INCHPAPER (LETTER, "Letter", 8.5, 11),
  INCHPAPER (LETTERSMALL, "Letter Small", 8.5, 11),
  INCHPAPER (TABLOID, "Tabloid", 11, 17),
  INCHPAPER (LEDGER, "Ledger", 17, 11),
  INCHPAPER (LEGAL, "Legal", 8.5, 14),
  INCHPAPER (STATEMENT, "Statement", 5.5, 8.5),
  INCHPAPER (EXECUTIVE, "Executive", 7.25, 10.5),
  MMPAPER (A3, "A3", 297, 420),
  MMPAPER (A4, "A4", 210, 297),
  MMPAPER (A4SMALL, "A4 Small", 210, 297),
  MMPAPER (A5, "A5", 148, 210),
  MMPAPER (B4, "B4", 250, 354), // <fixme/>
  MMPAPER (B5, "B5", 182, 257),
  INCHPAPER (FOLIO, "Folio", 8.5, 13),
  MMPAPER (QUARTO, "Quarto", 215, 275), // <fixme/>
  INCHPAPER (10X14, "10x14", 10, 14),
  INCHPAPER (11X17, "11x17", 11, 17),
  INCHPAPER (NOTE, "Note", 8.5, 11),
  INCHPAPER (ENV_9, "Envelope #9", 3.875, 8.875),
  INCHPAPER (ENV_10, "Envelope #10", 4.125, 9.5),
  INCHPAPER (ENV_11, "Envelope #11", 4.5, 10.375),
  INCHPAPER (ENV_12, "Envelope #12", 4.75, 11),
  INCHPAPER (ENV_14, "Envelope #14", 5, 11.5),
  INCHPAPER (CSHEET, "C", 17, 22),
  INCHPAPER (DSHEET, "D", 22, 34),
  INCHPAPER (ESHEET, "E", 34, 44),
  MMPAPER (ENV_DL, "Envelope DL", 110, 220),
  MMPAPER (ENV_C5, "Envelope C5", 162, 229),
  MMPAPER (ENV_C3, "Envelope C3", 324, 458),
  MMPAPER (ENV_C4, "Envelope C4", 229, 324),
  MMPAPER (ENV_C6, "Envelope C6", 114, 162),
  MMPAPER (ENV_C65, "Envelope C65", 114, 229),
  MMPAPER (ENV_B4, "Envelope B4", 250, 353),
  MMPAPER (ENV_B5, "Envelope B5", 176, 250),
  MMPAPER (ENV_B6, "Envelope B6", 176, 125),
  MMPAPER (ENV_ITALY, "Italy Envelope", 110, 230),
  INCHPAPER (ENV_MONARCH, "Envelope Monarch", 3.875, 7.5),
  INCHPAPER (ENV_PERSONAL, "6 3/4 Envelope", 3.625, 6.5),
  INCHPAPER (FANFOLD_US, "US Std Fanfold", 14.875, 11),
  INCHPAPER (FANFOLD_STD_GERMAN, "German Std Fanfold", 8.5, 12),
  INCHPAPER (FANFOLD_LGL_GERMAN, "German Legal Fanfold", 8.5, 13),
#if(WINVER >= 0x0400)
  MMPAPER (ISO_B4, "B4 (ISO)", 250, 353),
  MMPAPER (JAPANESE_POSTCARD, "Japanese Postcard", 100, 148),
  INCHPAPER (9X11, "9x11", 9, 11),
  INCHPAPER (10X11, "10x11", 10, 11),
  INCHPAPER (15X11, "15x11", 15, 11),
  MMPAPER (ENV_INVITE, "Envelope Invite", 220, 220),
  INCHPAPER (LETTER_EXTRA, "Letter Extra", 9.5, 12),
  INCHPAPER (LEGAL_EXTRA, "Legal Extra", 9.5, 15),
  INCHPAPER (TABLOID_EXTRA, "Tabloid Extra", 11.69, 18),
  INCHPAPER (A4_EXTRA, "A4 Extra", 9.27, 12.69),
  INCHPAPER (LETTER_TRANSVERSE, "Letter Transverse", 8.5, 11),
  MMPAPER (A4_TRANSVERSE, "A4 Transverse", 210, 297),
  INCHPAPER (LETTER_EXTRA_TRANSVERSE, "Letter Extra Transverse", 9.5, 12),
  MMPAPER (A_PLUS, "SuperA/SuperA/A4", 227, 356),
  MMPAPER (B_PLUS, "SuperB/SuperB/A3", 305, 487),
  INCHPAPER (LETTER_PLUS, "Letter Plus", 8.5, 12.69),
  MMPAPER (A4_PLUS, "A4 Plus", 210, 330),
  MMPAPER (A5_TRANSVERSE, "A5 Transverse", 148, 210),
  MMPAPER (B5_TRANSVERSE, "B5 (JIS) Transverse", 182, 257),
  MMPAPER (A3_EXTRA, "A3 Extra", 322, 445),
  MMPAPER (A5_EXTRA, "A5 Extra", 174, 235),
  MMPAPER (B5_EXTRA, "B5 (ISO) Extra", 201, 276),
  MMPAPER (A2, "A2", 420, 594),
  MMPAPER (A3_TRANSVERSE, "A3 Transverse", 297, 420),
  MMPAPER (A3_EXTRA_TRANSVERSE, "A3 Extra Transverse", 322, 445),
#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0500)
  MMPAPER (DBL_JAPANESE_POSTCARD, "Japanese Double Postcard", 200, 148),
  MMPAPER (A6, "A6", 105, 148),
  // DMPAPER_JENV_KAKU2          71  /* Japanese Envelope Kaku #2 */
  // DMPAPER_JENV_KAKU3          72  /* Japanese Envelope Kaku #3 */
  // DMPAPER_JENV_CHOU3          73  /* Japanese Envelope Chou #3 */
  // DMPAPER_JENV_CHOU4          74  /* Japanese Envelope Chou #4 */
  INCHPAPER (LETTER_ROTATED, "Letter Rotated", 11, 8.5),
  MMPAPER (A3_ROTATED, "A3 Rotated", 420, 297),
  MMPAPER (A4_ROTATED, "A4 Rotated", 297, 210),
  MMPAPER (A5_ROTATED, "A5 Rotated", 210, 148),
  MMPAPER (B4_JIS_ROTATED, "B4 (JIS) Rotated", 364, 257),
  MMPAPER (B5_JIS_ROTATED, "B5 (JIS) Rotated", 257, 182),
  MMPAPER (JAPANESE_POSTCARD_ROTATED, "Japanese Postcard Rotated", 148, 100),
  MMPAPER (DBL_JAPANESE_POSTCARD_ROTATED, "Double Japanese Postcard Rotated", 148, 200),
  MMPAPER (A6_ROTATED, "A6 Rotated", 148, 105),
  // DMPAPER_JENV_KAKU2_ROTATED  84  /* Japanese Envelope Kaku #2 Rotated */
  // DMPAPER_JENV_KAKU3_ROTATED  85  /* Japanese Envelope Kaku #3 Rotated */
  // DMPAPER_JENV_CHOU3_ROTATED  86  /* Japanese Envelope Chou #3 Rotated */
  // DMPAPER_JENV_CHOU4_ROTATED  87  /* Japanese Envelope Chou #4 Rotated */
  MMPAPER (B6_JIS, "B6 (JIS)", 128, 182),
  MMPAPER (B6_JIS_ROTATED, "B6 (JIS) Rotated", 182, 128),
  DMPAPER_12X11, "12x11", IN2PT(12, 11),
  // DMPAPER_JENV_YOU4           91  /* Japanese Envelope You #4        */
  // DMPAPER_JENV_YOU4_ROTATED   92  /* Japanese Envelope You #4 Rotated*/
  MMPAPER (P16K, "PRC 16K", 146, 215),
  MMPAPER (P32K, "PRC 32K", 97, 151),
  MMPAPER (P32KBIG, "PRC 32K(Big)", 97, 151),
  MMPAPER (PENV_1, "PRC Envelope #1", 102, 165),
  MMPAPER (PENV_2, "PRC Envelope #2", 102, 176),
  MMPAPER (PENV_3, "PRC Envelope #3", 125, 176),
  MMPAPER (PENV_4, "PRC Envelope #4", 110, 208),
  MMPAPER (PENV_5, "PRC Envelope #5", 110, 220),
  MMPAPER (PENV_6, "PRC Envelope #6", 120, 230),
  MMPAPER (PENV_7, "PRC Envelope #7", 160, 230),
  MMPAPER (PENV_8, "PRC Envelope #8", 120, 309),
  MMPAPER (PENV_9, "PRC Envelope #9", 229, 324),
  MMPAPER (PENV_10, "PRC Envelope #10", 324, 458),
  MMPAPER (P16K_ROTATED, "PRC 16K Rotated", 215, 146),
  MMPAPER (P32K_ROTATED, "PRC 32K Rotated", 151, 97),
  MMPAPER (P32KBIG_ROTATED, "PRC 32K(Big) Rotated", 151, 97),
  MMPAPER (PENV_1_ROTATED, "PRC Envelope #1 Rotated", 165, 102),
  MMPAPER (PENV_2_ROTATED, "PRC Envelope #2 Rotated", 176, 102),
  MMPAPER (PENV_3_ROTATED, "PRC Envelope #3 Rotated", 176, 125),
  MMPAPER (PENV_4_ROTATED, "PRC Envelope #4 Rotated", 208, 110),
  MMPAPER (PENV_5_ROTATED, "PRC Envelope #5 Rotated", 220, 110),
  MMPAPER (PENV_6_ROTATED, "PRC Envelope #6 Rotated", 230, 120),
  MMPAPER (PENV_7_ROTATED, "PRC Envelope #7 Rotated", 230, 160),
  MMPAPER (PENV_8_ROTATED, "PRC Envelope #8 Rotated", 309, 120),
  MMPAPER (PENV_9_ROTATED, "PRC Envelope #9 Rotated", 324, 229),
  MMPAPER (PENV_10_ROTATED, "PRC Envelope #10 Rotated", 458, 324),
#endif /* WINVER >= 0x0500 */
};

enum Option
{
  OPT_PRINTER = 1000,
  OPT_PRINT_METHOD,
  OPT_PAGE_RANGE,
  OPT_EVEN_ONLY,
  OPT_ODD_ONLY,
  OPT_LANDSCAPE,
  OPT_PRINT_NOTHING,
  OPT_HHELP,
  OPT_VERSION,
};

const struct poptOption PrintUtility::aoption[] = {
  {
    "even-only", 0, POPT_ARG_NONE, nullptr, OPT_EVEN_ONLY,
    T_("Prints only even TeX pages."),
    nullptr
  },
  {
    "hhelp", 0, POPT_ARG_NONE, nullptr, OPT_HHELP,
    T_("Show the manual page."),
    nullptr
  },
  {
    "landscape", 0, POPT_ARG_NONE, nullptr, OPT_LANDSCAPE,
    T_("Selects landscape output format."),
    nullptr
  },
  {
    "odd-only", 0, POPT_ARG_NONE, nullptr, OPT_ODD_ONLY,
    T_("Prints only odd TeX pages."),
    nullptr
  },
  {
    "page-range", 0, POPT_ARG_STRING, nullptr, OPT_PAGE_RANGE,
    T_("Selects a TeX page range (e.g. 20-21).  Multiple --page-range options accumulate."),
    T_("RANGE"),
  },
  {
    "print-method", 0, POPT_ARG_STRING, nullptr, OPT_PRINT_METHOD,
    T_("Selects a print method. One of: psbmp, ps."),
    T_("METHOD"),
  },
  {
    "print-nothing", 0, POPT_ARG_NONE, nullptr, OPT_PRINT_NOTHING,
    T_("Simulates printing."),
    nullptr
  },
  { "printer", 0, POPT_ARG_STRING, nullptr, OPT_PRINTER,
    T_("Selects a printing device."),
    T_("PRINTER"),
  },
  {
    "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },
  POPT_AUTOHELP
  POPT_TABLEEND
};

void PrintUtility::Report(const char * lpszFormat, ...)
{
  if (!verbose)
  {
    return;
  }
  va_list argList;
  VA_START(argList, lpszFormat);
  cout << StringUtil::FormatString(lpszFormat, argList);
  VA_END(argList);
}

bool PrintUtility::GetPaperSizeInfo(short paperSize, PAPERSIZEINFO & paperSizeInfo)
{
  for (size_t idx = 0; idx < sizeof(paperSizes) / sizeof(paperSizes[0]); ++idx)
  {
    if (paperSizes[idx].paperSize == paperSize)
    {
      paperSizeInfo = paperSizes[idx];
      return true;
    }
  }
  return false;
}

void PrintUtility::StartDvips(const char * lpszDviFileName, const DVIPSOPTS & dvipsOpts, unsigned resolution, const char * lpszPrinterName, short paperSize, FILE * * ppfileDvipsOutRd, FILE * * ppfileDvipsErrRd)
{
  // locate dvips.exe
  PathName dvipsPath;
  if (!session->FindFile(MIKTEX_DVIPS_EXE, FileType::EXE, dvipsPath))
  {
    MIKTEX_UNEXPECTED();
  }

  // make dvips command line
  CommandLineBuilder commandLine;
  if (dvipsOpts.oddPagesOnly)
  {
    commandLine.AppendOption("-A");
  }
  if (dvipsOpts.evenPagesOnly)
  {
    commandLine.AppendOption("-B");
  }
  if (resolution > 0)
  {
    commandLine.AppendOption("-D", std::to_string(resolution));
  }
  PAPERSIZEINFO paperSizeInfo;
  if (GetPaperSizeInfo(paperSize, paperSizeInfo))
  {
    commandLine.AppendOption("-T", paperSizeInfo.lpszDvipsSize);
  }
  if (dvipsOpts.runAsFilter)
  {
    commandLine.AppendOption("-f", "1");
  }
  if (dvipsOpts.sendCtrlDAtEnd)
  {
    commandLine.AppendOption("-F"), "1";
  }
  if (dvipsOpts.shiftLowCharsToHigherPos)
  {
    commandLine.AppendOption("-G"), "1";
  }
  if (dvipsOpts.offsetX.length() > 0 && dvipsOpts.offsetY.length() > 0)
  {
    string str(dvipsOpts.offsetX);
    str += ",";
    str += dvipsOpts.offsetY;
    commandLine.AppendOption("-O", str.c_str());
  }
  if (!dvipsOpts.runAsFilter)
  {
    PRINTER_INFO_2W * pi2 = Printer::GetPrinterInfo(lpszPrinterName, nullptr);
    AutoMemoryPointer autoFree(pi2);
    commandLine.AppendOption("-o ", dryRun ? "nul" : WU_(pi2->pPortName));
  }
  for (vector<DVIPSOPTS::PAGERANGE>::const_iterator it = dvipsOpts.pageRanges.begin(); it != dvipsOpts.pageRanges.end(); ++it)
  {
    string str(std::to_string(it->firstPage));
    str += ':';
    str += std::to_string(it->lastPage);
    commandLine.AppendOption("-pp ", str.c_str());
  }
  if (dvipsOpts.config.length() > 0)
  {
    commandLine.AppendOption("-P ", dvipsOpts.config.c_str());
  }
  if (dvipsOpts.runQuietly)
  {
    commandLine.AppendOption("-q", "1");
  }
  if (dvipsOpts.paperFormat.length() > 0)
  {
    commandLine.AppendOption("-t ", dvipsOpts.paperFormat.c_str());
  }
  commandLine.AppendArgument(lpszDviFileName);

  trace_mtprint->WriteLine("mtprint", commandLine.ToString().c_str());

  // start Dvips
  if (dvipsOpts.runAsFilter)
  {
    Process::Start(dvipsPath.Get(), commandLine.ToString(), nullptr, nullptr, ppfileDvipsOutRd, ppfileDvipsErrRd, nullptr);
  }
  else
  {
    Process::Run(dvipsPath.Get(), commandLine.ToString());
  }
}

void PrintUtility::StartGhostscript(const GSOPTS & gsOpts, unsigned resolution, short paperSize, FILE * pfileGsIn, FILE ** ppfileGsOut)
{
  gsOpts;

  PathName gsPath;
  session->GetGhostscript(gsPath.GetData(), nullptr);

  // make GS command line
  CommandLineBuilder commandLine;
  commandLine.AppendOption("-sDEVICE=", "bmp16m");
  MIKTEX_ASSERT(ppfileGsOut != nullptr);
  commandLine.AppendOption("-sOutputFile=", "-");
  if (resolution > 0)
  {
    commandLine.AppendOption("-r", std::to_string(resolution));
  }
  PAPERSIZEINFO paperSizeInfo;
  if (GetPaperSizeInfo(paperSize, paperSizeInfo))
  {
    commandLine.AppendOption("-dDEVICEWIDTHPOINTS=", std::to_string(paperSizeInfo.width));
    commandLine.AppendOption("-dDEVICEHEIGHTPOINTS=", std::to_string(paperSizeInfo.height));
  }
  commandLine.AppendOption("-q");
  commandLine.AppendOption("-dBATCH");
  commandLine.AppendOption("-dNOPAUSE");
  commandLine.AppendOption("-dSAFER");
  commandLine.AppendArgument("-");

  trace_mtprint->WriteLine("mtprint", commandLine.ToString().c_str());

  // start Ghostscript
  Process::Start(gsPath.Get(), commandLine.ToString(), pfileGsIn, nullptr, ppfileGsOut, nullptr, nullptr);
}

void PrintUtility::Spool(const char * lpszFileName, PrintMethod printMethod, const DVIPSOPTS & dvipsOpts, const GSOPTS & gsOpts, const string & printerName)
{
  // get printer resolution and paper size
  unsigned resolution;
  Printer::GetPrinterCaps(printerName.c_str(), resolution);
  trace_mtprint->WriteFormattedLine("mtprint", "resolution: %u", resolution);
  DEVMODEW * pdm = Printer::GetDevMode(printerName.c_str());
  short paperSize = pdm->dmPaperSize;
  free(pdm);
  pdm = nullptr;

  if (printMethod == PrintMethodPostScript)
  {
    StartDvips(lpszFileName, dvipsOpts, resolution, printerName.c_str(), paperSize, nullptr, nullptr);
    return;
  }

  MIKTEX_ASSERT(printMethod == PrintMethodPostScriptBMP);

  FILE * pfileDvipsOutRd = nullptr;
  FILE * pfileDvipsErrRd = nullptr;

  StartDvips(lpszFileName, dvipsOpts, resolution, printerName.c_str(), paperSize, &pfileDvipsOutRd, &pfileDvipsErrRd);

  FileStream dvipsOut(pfileDvipsOutRd);
  FileStream dvipsErr(pfileDvipsErrRd);

  FILE * pfileGsRead = nullptr;

  StartGhostscript(gsOpts, resolution, paperSize, pfileDvipsOutRd, &pfileGsRead);

  FileStream gsOut(pfileGsRead);

  Printer::PRINTINFO pi;
  pi.printerName = printerName;
  pi.pCallback = this;
  BitmapPrinter printer(pi, dryRun);
  printer.Print(pfileGsRead);
}

void PrintUtility::ShowVersion()
{
  cout
    << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, MIKTEX_COMPONENT_VERSION_STR) << endl
    << "Copyright (C) 2003-2016 Christian Schenk" << endl
    << "This is free software; see the source for copying conditions.  There is NO" << endl
    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

void PrintUtility::Run(int argc, const char ** argv)
{
  // initialize printerName with the default printer
  char szPrinterName[300];
  size_t size = 300;
  if (Utils::GetDefPrinter(szPrinterName, &size))
  {
    printerName = szPrinterName;
  }

  PrintMethod printMethod = PrintMethodPostScriptBMP;
  DVIPSOPTS dvipsOpts;
  GSOPTS gsOpts;

  PoptWrapper popt(argc, argv, aoption);

  popt.SetOtherOptionHelp(T_("[OPTION...] FILE..."));

  // process command-line options
  int option;
  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {
    case OPT_EVEN_ONLY:
      dvipsOpts.evenPagesOnly = true;
      break;
    case OPT_HHELP:
      session->ShowManualPageAndWait(0, MIKTEXHELP_MTPRINT);
      return;
    case OPT_ODD_ONLY:
      dvipsOpts.oddPagesOnly = true;
      break;
    case OPT_PAGE_RANGE:
    {
      DVIPSOPTS::PAGERANGE pagerange;
      string optArg = popt.GetOptArg();
      if (sscanf_s(optArg.c_str(), "%d-%d", &pagerange.firstPage, &pagerange.lastPage) != 2)
      {
        MIKTEX_FATAL_ERROR_2("Invalid page range.", "range", optArg);
      }
      dvipsOpts.pageRanges.push_back(pagerange);
      break;
    }
    case OPT_PRINTER:
      printerName = popt.GetOptArg();
      break;
    case OPT_PRINT_METHOD:
    {
      string method = popt.GetOptArg();
      if (method == "ps")
      {
        printMethod = PrintMethodPostScript;
      }
      else if (method == "psbmp")
      {
        printMethod = PrintMethodPostScriptBMP;
      }
      else
      {
        MIKTEX_FATAL_ERROR_2("Unknown print method.", "method", method);
      }
      break;
    }
    case OPT_PRINT_NOTHING:
      dryRun = true;
      break;
    case OPT_VERSION:
      ShowVersion();
      return;
    }
  }

  if (option != -1)
  {
    cerr << popt.BadOption(POPT_BADOPTION_NOALIAS) << ": " << popt.Strerror(option) << endl;
    throw 1;
  }

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    // TODO: send stdin to the printer
    return;
  }

  if (printMethod != PrintMethodPostScript)
  {
    dvipsOpts.runAsFilter = true;
  }

  TraceStream::SetTraceFlags(DEFAULT_TRACE_STREAMS);

  for (const string & fileName : leftovers)
  {
    Spool(fileName.c_str(), printMethod, dvipsOpts, gsOpts, printerName);
  }
}

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, const MAINCHAR ** argv)
{
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char *> newargv;
    newargv.reserve(argc + 1);
    for (int idx = 0; idx < argc; ++idx)
    {
#if defined(_UNICODE)
      utf8args.push_back(StringUtil::WideCharToUTF8(argv[idx]));
#else
      utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#endif
      newargv.push_back(utf8args[idx].c_str());
    }
    newargv.push_back(nullptr);
    PrintUtility app;
    app.Init(newargv[0]);
    app.Run(argc, &newargv[0]);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Application::Sorry(THE_NAME_OF_THE_GAME, e);
    return 1;
  }
  catch (const exception & e)
  {
    Application::Sorry(THE_NAME_OF_THE_GAME, e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
