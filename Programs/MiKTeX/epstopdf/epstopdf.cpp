/* epstopdf.cpp: epstopdf

   Copyright (C) 2000-2016 Christian Schenk
   Copyright (C) 1998-2001 by Sebastian Rahtz et al.

   This file is part of EPStoPDF.

   EPStoPDF is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   EPStoPDF is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with EPStoPDF; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

   /* This program reimplements the epstopdf Perl script written by
      Sebastian Rahtz. */

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <memory>
#include <string>

#include <miktex/App/Application>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/FileStream>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/StringUtil>
#include <miktex/Wrappers/PoptWrapper>

#include "epstopdf-version.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace MiKTeX;
using namespace std;

#define PROGRAM_NAME "epstopdf"

#if ! defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX EPS-to-PDF Converter")
#endif

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#define VA_START(arglist, lpszFormat   )        \
va_start(arglist, lpszFormat);                  \
try                                             \
{

#define VA_END(arglist)                         \
}                                               \
catch(...)                                      \
{                                               \
  va_end(arglist);                              \
  throw;                                        \
}                                               \
va_end(arglist);

const char * DEFAULT_TRACE_STREAMS =
MIKTEX_TRACE_ERROR ","
MIKTEX_TRACE_PROCESS ","
PROGRAM_NAME;

class EpsToPdfApp :
  public Application
{
public:
  EpsToPdfApp() :
    traceStream(TraceStream::Open(PROGRAM_NAME))
  {
  }

public:
  void Run(int argc, const char ** argv);

private:
  void Trace(const char * lpszFormat, ...);

private:
  void Verbose(const char * lpszFormat, ...);

private:
  void PrintOnly(const char * lpszFormat, ...);

private:
  void Warning(const char * lpszFormat, ...);

private:
  bool GetLine(string & line);

private:
  void PutFormattedLine(const char * lpszFormat, ...);

private:
  void PutLine(const string & line);

private:
  void CorrectBoundingBox(double llx, double lly, double urx, double ury);

private:
  bool BoundingBoxWithValues(const string & line, double & llx, double & lly, double & urx, double & ury);

private:
  bool BoundingBoxWithAtEnd(const string & line);

private:
  void ScanHeader();

private:
  int ReadDosBinary4();

private:
  void GetFirstLine(string & line);

private:
  void PrepareInput(bool runAsFilter, const PathName & inputFile);

private:
  void PrepareOutput(bool runAsFilter, bool runGhostscript, const char * lpszGSExe, const CommandLineBuilder & gsOptions, const PathName & outFile);

private:
  unique_ptr<Process> gsProcess;

private:
  bool exactBoundingBox = false;

private:
  bool runAsFilter = false;

private:
  bool runGhostscript = true;

private:
  bool hiResBoundingBox = false;

private:
  string boundingBoxName;

private:
  FileStream inStream;

private:
  long stopReadingAt;

private:
  FileStream outStream;

private:
  bool boundingBoxCorrected = false;

private:
  unique_ptr<TraceStream> traceStream;

private:
  bool verbose = false;

private:
  bool printOnly = false;

private:
  double enlarge = 0.0;

private:
  string pdfVersion;

private:
  static struct poptOption aoption[];
};

enum Option
{
  OPT_AAA = 1000,
  OPT_ANTIALIASING,
  OPT_COMPRESS,
  OPT_DEBUG,
  OPT_ENLARGE,
  OPT_EXACT,
  OPT_FILTER,
  OPT_GS,
  OPT_GSOPT,
  OPT_HIRES,
  OPT_NOANTIALIASING,
  OPT_NOCOMPRESS,
  OPT_NODEBUG,
  OPT_NOEXACT,
  OPT_NOFILTER,
  OPT_NOGS,
  OPT_NOHIRES,
  OPT_NOPDFVERS,
  OPT_OUTFILE,
  OPT_PDF_VERSION,
  OPT_PRINT_ONLY,
  OPT_TRACE,
  OPT_VERBOSE,
  OPT_VERSION
};

struct poptOption EpsToPdfApp::aoption[] = {

  {
    "antialias", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ANTIALIASING,
    T_("Enable anti-aliasing of bitmaps."),
    nullptr
  },

  {
    "compress", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMPRESS,
    T_("Enable PDF compression."),
    nullptr
  },

  {
    "debug", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_DEBUG,
    T_("Write trace messages."),
    nullptr
  },

  {
    "enlarge", 0,
    POPT_ARG_STRING, nullptr,
    OPT_ENLARGE,
    T_("Enlarge bounding box by N PostScript points."),
    "N"
  },

  {
    "exact", 0,
    POPT_ARG_NONE, nullptr,
    OPT_EXACT,
    T_("Scan the EPS file for %%ExactBoundingBox."),
    nullptr
  },

  {
    "filter", 0,
    POPT_ARG_NONE, nullptr,
    OPT_FILTER,
    T_("Run as a filter, i.e., read the EPS file from the standard input stream and write the output file to the standard output stream."),
    nullptr
  },

  {
    "gs", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_GS,
    T_("Run Ghostscript."),
    nullptr
  },

  {
    "gsopt", 0,
    POPT_ARG_STRING, nullptr,
    OPT_GSOPT,
    T_("Add OPTIONS to the Ghostscript command-line."),
    T_("OPTIONS")
  },

  {
    "hires", 0,
    POPT_ARG_NONE, nullptr,
    OPT_HIRES,
    T_("Scan the EPS file for %%HiResBoundingBox."),
    nullptr
  },

  {
    "noantialias", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NOANTIALIASING,
    T_("Disable anti-aliasing of bitmaps."),
    nullptr
  },

  {
    "nocompress", 0,
    POPT_ARG_NONE, nullptr,
    OPT_NOCOMPRESS,
    T_("Disable PDF compression."),
    nullptr
  },

  {
    "nodebug", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NODEBUG,
    T_("Do not print debug information."),
    nullptr
  },

  {
    "noexact", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NOEXACT,
    T_("Do not scan the EPS file for %%ExactBoundingBox."),
    nullptr
  },

  {
    "nofilter", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NOFILTER,
    T_("Do not run as a filter."),
    nullptr
  },

  {
    "nogs", 0,
    POPT_ARG_NONE, nullptr,
    OPT_NOGS,
    T_("Correct the bounding box but do not run Ghostscript."),
    nullptr
  },

  {
    "nohires", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NOHIRES,
    T_("Do not scan the EPS file for %%HiResBoundingBox."),
    nullptr
  },

  {
    "nopdfvers", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NOPDFVERS,
    T_("Do not set the PDF version."),
    nullptr
  },

  {
    "outfile", 0,
    POPT_ARG_STRING, nullptr,
    OPT_OUTFILE,
    T_("Write the result to FILE."),
    T_("FILE"),
  },

  {
    "print-only", 'n',
    POPT_ARG_NONE, nullptr,
    OPT_PRINT_ONLY,
    T_("Print what would be done."),
    nullptr
  },

  {
    "pdf-version", 0,
    POPT_ARG_STRING, nullptr,
    OPT_PDF_VERSION,
    T_("Set the PDF version."),
    T_("PDFVER")
  },

  {
    "pdfvers", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_PDF_VERSION,
    T_("Set the PDF version."),
    T_("PDFVER")
  },

  {
    "trace", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_TRACE,
    T_("Turn on tracing.  TRACESTREAMS, if specified, is a comma-separated list of trace stream names (see the MiKTeX manual)."),
    T_("TRACESTREAMS"),
  },

  {
    "verbose", 0,
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Turn on verbose output mode."),
    nullptr
  },

  {
    "version", 0,
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Show version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

void EpsToPdfApp::Trace(const char * lpszFormat, ...)
{
  va_list arglist;
  VA_START(arglist, lpszFormat);
  traceStream->WriteLine(PROGRAM_NAME, StringUtil::FormatString(lpszFormat, arglist).c_str());
  VA_END(arglist);
}

void EpsToPdfApp::Verbose(const char * lpszFormat, ...)
{
  if (!verbose || printOnly)
  {
    return;
  }
  va_list arglist;
  VA_START(arglist, lpszFormat);
  cout << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
}

void EpsToPdfApp::PrintOnly(const char * lpszFormat, ...)
{
  if (!printOnly)
  {
    return;
  }
  va_list arglist;
  VA_START(arglist, lpszFormat);
  cout << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
}

void EpsToPdfApp::Warning(const char * lpszFormat, ...)
{
  va_list arglist;
  VA_START(arglist, lpszFormat);
  cerr << T_("warning") << ": " << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
}

bool EpsToPdfApp::GetLine(string & line)
{
  if (stopReadingAt > 0)
  {
    long pos = inStream.GetPosition();
    if (pos >= stopReadingAt)
    {
      return false;
    }
  }
  bool done = Utils::ReadUntilDelim(line, '\n', inStream.Get());
  if (done)
  {
    string::size_type l = line.length();
    for (string::const_reverse_iterator it = line.rbegin(); it != line.rend(); ++it)
    {
      if (*it != '\n' && *it != '\r')
      {
        break;
      }
      --l;
    }
    line.erase(l);
  }
  return done;
}


void EpsToPdfApp::PutFormattedLine(const char * lpszFormat, ...)
{
  if (!printOnly)
  {
    va_list marker;
    VA_START(marker, lpszFormat);
    vfprintf(outStream.Get(), lpszFormat, marker);
    VA_END(marker);
    fputc('\n', outStream.Get());
  }
}

void
EpsToPdfApp::PutLine(const string & line)
{
  if (!printOnly)
  {
    fprintf(outStream.Get(), "%s\n", line.c_str());
  }
}

void EpsToPdfApp::CorrectBoundingBox(double llx, double lly, double urx, double ury)
{
  Trace(T_("Old BoundingBox: %f %f %f %f"), llx, lly, urx, ury);
  llx -= enlarge;
  lly -= enlarge;
  urx += enlarge;
  ury += enlarge;
  int width = static_cast<int>(ceil(urx - llx));
  int height = static_cast<int>(ceil(ury - lly));
  int xoffset = static_cast<int>(-llx);
  int yoffset = static_cast<int>(-lly);
  Trace(T_("New BoundingBox: 0 0 %d %d"), width, height);
  Trace(T_("Offset: %d %d"), xoffset, yoffset);
  PutFormattedLine(T_("%%%%BoundingBox: 0 0 %d %d"), width, height);
  PutFormattedLine(T_("<< /PageSize [%d %d] >> setpagedevice"), width, height);
  PutFormattedLine(T_("gsave %d %d translate"), xoffset, yoffset);
}

bool EpsToPdfApp::BoundingBoxWithValues(const string & line, double & llx, double & lly, double & urx, double & ury)
{
  if (line.compare(0, boundingBoxName.length(), boundingBoxName) != 0)
  {
    return false;
  }
#if _MSC_VER < 1400
#  define sscanf_s sscanf
#endif
  if (sscanf_s(line.c_str() + boundingBoxName.length(), " %lf %lf %lf %lf", &llx, &lly, &urx, &ury) != 4)
  {
    return false;
  }
  return true;
}

bool EpsToPdfApp::BoundingBoxWithAtEnd(const string & line)
{
  if (line.compare(0, boundingBoxName.length(), boundingBoxName) != 0)
  {
    return false;
  }
  const char * lpsz = line.c_str() + boundingBoxName.length();
  while (isspace(*lpsz))
  {
    ++lpsz;
  }
  return Utils::Equals(lpsz, "(atend)");
}

void EpsToPdfApp::ScanHeader()
{
  string line;

  while (GetLine(line))
  {
    if (line.compare(0, 13, "%%EndComments") == 0)
    {
      PutLine(line);
      break;
    }

    double llx, lly, urx, ury;

    if (BoundingBoxWithValues(line, llx, lly, urx, ury))
    {
      CorrectBoundingBox(llx, lly, urx, ury);
      boundingBoxCorrected = true;
      break;
    }

    if (BoundingBoxWithAtEnd(line))
    {
      Trace(T_("%s (atend)"), boundingBoxName.c_str());
      if (runAsFilter)
      {
        Warning(T_("Cannot look for BoundingBox in the trailer with option --filter."));
        break;
      }
      long pos = inStream.GetPosition();
      Trace(T_("Current file position: %d"), pos);
      while (GetLine(line))
      {
        if (line.compare(0, 15, "%%BeginDocument") == 0)
        {
          while (GetLine(line))
          {
            if (line.compare(0, 13, "%%EndDocument") == 0)
            {
              break;
            }
          }
        }
        if (BoundingBoxWithValues(line, llx, lly, urx, ury))
        {
          CorrectBoundingBox(llx, lly, urx, ury);
          boundingBoxCorrected = true;
          break;
        }
      }
      inStream.Seek(pos, SeekOrigin::Begin);
      break;
    }

    PutLine(line);
  }
}

int EpsToPdfApp::ReadDosBinary4()
{
  unsigned char buf[4];
  if (inStream.Read(buf, 4) != 4)
  {
    FatalError(T_("Not a valid EPS file."));
  }
  return ((static_cast<int>(buf[3]) & 0xff) << 24)
    | ((static_cast<int>(buf[2]) & 0xff) << 16)
    | ((static_cast<int>(buf[1]) & 0xff) << 8)
    | ((static_cast<int>(buf[0]) & 0xff));
}

void EpsToPdfApp::GetFirstLine(string & line)
{
  unsigned char buf[4];
  if (inStream.Read(buf, 4) != 4)
  {
    FatalError(T_("Not a valid EPS file."));
  }
  if (buf[0] == '%' && buf[1] == '!' && buf[2] == 'P' && buf[3] == 'S')
  {
    line = "%!PS";
    string line1;
    stopReadingAt = 0;
    if (GetLine(line1))
    {
      line += line1;
    }
  }
  else if (buf[0] != 0xc5 || buf[1] != 0xd0 || buf[2] != 0xd3 || buf[3] != 0xc6)
  {
    FatalError(T_("Invalid binary DOS header."));
  }
  else
  {
    int startPS = ReadDosBinary4();
    int lengthPS = ReadDosBinary4();
    stopReadingAt = startPS + lengthPS;
    inStream.Seek(startPS, SeekOrigin::Begin);
    if (!GetLine(line))
    {
      FatalError(T_("Not a valid EPS file."));
    }
  }
}

void EpsToPdfApp::PrepareInput(bool runAsFilter, const PathName & inputFile)
{
  if (runAsFilter)
  {
    if (stdin == nullptr)
    {
      FatalError(T_("No standard input stream."));
    }
    inStream.Attach(stdin);
  }
  else
  {
    inStream.Attach(File::Open(inputFile, FileMode::Open, FileAccess::Read));
  }
}

void EpsToPdfApp::PrepareOutput(bool runAsFilter, bool runGhostscript, const char * lpszGSExe, const CommandLineBuilder & gsOptions, const PathName & outFile)
{
  if (runGhostscript)
  {
    CommandLineBuilder cmdLine(gsOptions);
    cmdLine.AppendOption("-q");
    cmdLine.AppendOption("-sDEVICE=", "pdfwrite");
    cmdLine.AppendOption("-dSAFER");
#if 1                           // 642845
    cmdLine.AppendOption("-dAutoRotatePages=", "/None");
#endif
    if (!pdfVersion.empty())
    {
      cmdLine.AppendOption("-dCompatibilityLevel=", pdfVersion);
    }
    if (runAsFilter)
    {
      cmdLine.AppendOption("-sOutputFile=", "-");
    }
    else
    {
      cmdLine.AppendOption("-sOutputFile=", outFile);
    }
    cmdLine.AppendOption("-");
    cmdLine.AppendOption("-c");
    cmdLine.AppendArgument("quit");
    PrintOnly("%s %s\n", Q_(lpszGSExe), cmdLine.ToString().c_str());
    if (!printOnly)
    {
      ProcessStartInfo processStartInfo;
      processStartInfo.FileName = lpszGSExe;
      processStartInfo.Arguments = cmdLine.ToString();
      processStartInfo.StandardInput = nullptr;
      processStartInfo.RedirectStandardError = false;
      processStartInfo.RedirectStandardInput = true;
      processStartInfo.RedirectStandardOutput = false;
      gsProcess = auto_ptr<Process>(Process::Start(processStartInfo));
      outStream.Attach(gsProcess->get_StandardInput());
    }
  }
  else if (runAsFilter)
  {
    if (stdout == nullptr)
    {
      FatalError(T_("No standard output stream."));
    }
    outStream.Attach(stdout);
    outStream.SetBinary();
  }
  else
  {
    if (!printOnly)
    {
      outStream.Attach(File::Open(outFile, FileMode::Create, FileAccess::Write, false));
    }
  }
}

void EpsToPdfApp::Run(int argc, const char ** argv)
{
  PathName outFile;

  PoptWrapper popt(argc, argv, aoption);

  int option;

  bool antiAliasing = false;
  bool doCompress = true;

  Session::InitInfo initInfo(argv[0]);

  CommandLineBuilder gsOptions;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_ANTIALIASING:
      antiAliasing = true;
      break;
    case OPT_COMPRESS:
      doCompress = true;
      break;
    case OPT_DEBUG:
      initInfo.SetTraceFlags(DEFAULT_TRACE_STREAMS);
      break;
    case OPT_ENLARGE:
      enlarge = std::stoi(optArg);
      break;
    case OPT_EXACT:
      exactBoundingBox = true;
      break;
    case OPT_FILTER:
      runAsFilter = true;
      break;
    case OPT_GS:
      runGhostscript = true;
      break;
    case OPT_GSOPT:
      gsOptions.AppendUnquoted(optArg);
      break;
    case OPT_HIRES:
      hiResBoundingBox = true;
      break;
    case OPT_NOANTIALIASING:
      antiAliasing = false;
      break;
    case OPT_NOCOMPRESS:
      doCompress = false;
      break;
    case OPT_NODEBUG:
      // unsupported option
      break;
    case OPT_NOEXACT:
      exactBoundingBox = false;
      break;
    case OPT_NOFILTER:
      runAsFilter = false;
      break;
    case OPT_NOGS:
      runGhostscript = false;
      break;
    case OPT_NOHIRES:
      hiResBoundingBox = false;
      break;
    case OPT_NOPDFVERS:
      pdfVersion = "";
      break;
    case OPT_OUTFILE:
      outFile = optArg;
      break;
    case OPT_PDF_VERSION:
      pdfVersion = optArg;
      break;
    case OPT_PRINT_ONLY:
      printOnly = true;
      break;
    case OPT_TRACE:
      if (optArg.empty())
      {
        initInfo.SetTraceFlags(DEFAULT_TRACE_STREAMS);
      }
      else
      {
        initInfo.SetTraceFlags(optArg);
      }
      break;
    case OPT_VERBOSE:
      verbose = true;
      break;
    case OPT_VERSION:
      cout
        << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
        << "Copyright (C) 2000-2016 Christian Schenk" << endl
        << "Copyright (C) 1998-2001 by Sebastian Rahtz et al." << endl
        << "This is free software; see the source for copying conditions.  There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
      return;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    FatalError("%s", msg.c_str());
  }

  Init(initInfo);

  PathName inputFile;

  vector<string> leftovers = popt.GetLeftovers();

  if (runAsFilter)
  {
    if (!leftovers.empty())
    {
      FatalError(T_("Input file cannot be specified together with --filter option."));
    }
    Trace(T_("Input file: standard input"));
  }
  else
  {
    if (leftovers.empty())
    {
      FatalError(T_("Missing input file."));
    }
    else if (leftovers.size() != 1)
    {
      FatalError(T_("Too many input files."));
    }
    if (!session->FindFile(leftovers[0].c_str(), ".", inputFile))
    {
      FatalError(T_("The input file does not exist."));
    }
    Trace(T_("Input filename: %s"), inputFile.Get());
  }

  if (runAsFilter && verbose)
  {
    FatalError(T_("Options --filter and --verbose cannot be used together."));
  }

  if (antiAliasing)
  {
    gsOptions.AppendOption(T_("-dTextAlphaBits="), "4");
    gsOptions.AppendOption(T_("-dGraphicsAlphaBits="), "4");
  }

  if (!doCompress)
  {
    gsOptions.AppendOption(T_("-dUseFlateCompression="), T_("false"));
  }

  boundingBoxName = T_("%%BoundingBox:");
  if (hiResBoundingBox && exactBoundingBox)
  {
    FatalError(T_("Options --hires and --exact cannot be used together."));
  }
  if (hiResBoundingBox)
  {
    boundingBoxName = T_("%%HiResBoundingBox:");
  }
  if (exactBoundingBox)
  {
    boundingBoxName = T_("%%ExactBoundingBox:");
  }

  if (outFile.GetLength() > 0 && runAsFilter)
  {
    FatalError(T_("Input file cannot be specified together with --filter option."));
  }

  if (outFile.Empty())
  {
    if (runGhostscript)
    {
      outFile = inputFile;
      outFile.SetExtension(".pdf");
    }
    else
    {
      outFile = inputFile;
      outFile.SetExtension(0);
      outFile.Append("2", false);
      outFile.SetExtension(".eps");
    }
  }

  if (!runAsFilter)
  {
    Verbose(T_("Making %s from %s..."), Q_(outFile), Q_(inputFile));
  }

  char szGSExe[BufferSizes::MaxPath];

  if (runGhostscript)
  {
    session->GetGhostscript(szGSExe, 0);
  }

  PrepareInput(runAsFilter, inputFile);
  PrepareOutput(runAsFilter, runGhostscript, szGSExe, gsOptions, outFile);

  string line;

  GetFirstLine(line);

  PutLine(line);

  ScanHeader();

  while (GetLine(line))
  {
    PutLine(line);
  }

  if (boundingBoxCorrected)
  {
    PutLine(T_("\ngrestore"));
  }

  outStream.Close();

  if (!boundingBoxCorrected)
  {
    FatalError(T_("BoundingBox not found."));
  }

  if (gsProcess != nullptr)
  {
    gsProcess->WaitForExit();
    gsProcess = nullptr;
  }

  Finalize();
}

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR ** argv)
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
#elif defined(MIKTEX_WINDOWS)
      utf8args.push_back(StringUtil::AnsiToUTF8(argv[idx]));
#else
      utf8args.push_back(argv[idx]);
#endif
      newargv.push_back(utf8args[idx].c_str());
    }
    newargv.push_back(nullptr);
    EpsToPdfApp app;
    app.Run(argc, &newargv[0]);
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
