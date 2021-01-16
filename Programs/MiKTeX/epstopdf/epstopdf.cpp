/* epstopdf.cpp: epstopdf

   Copyright (C) 2000-2021 Christian Schenk
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
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

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

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#include "epstopdf-version.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace MiKTeX;
using namespace std;
using namespace std::string_literals;

#define PROGRAM_NAME "epstopdf"

#if ! defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX EPS-to-PDF Converter")
#endif

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

vector<string> DEFAULT_TRACE_OPTIONS = {
  TraceStream::MakeOption("", "", TraceLevel::Info),
  TraceStream::MakeOption(PROGRAM_NAME, "", TraceLevel::Trace),
};

class EpsToPdfApp :
  public Application
{
public:
  EpsToPdfApp() :
    traceStream(TraceStream::Open(PROGRAM_NAME))
  {
  }

public:
  void Run(int argc, const char** argv);

private:
  void MyTrace(const string& line);

private:
  void Verbose(const string& line);

private:
  void PrintOnly(const string& line);

private:
  void Warning(const string& line);

private:
  bool GetLine(string& line);

private:
  void PutLine(const string& line);

private:
  void ExamineLine(const string& line);

private:
  void EnsureFontIsAvailable(const string& fontName);

private:
  void ReadFontMapFile(const PathName& path);

private:
  void CorrectBoundingBox(double llx, double lly, double urx, double ury);

private:
  bool BoundingBoxWithValues(const string& line, double& llx, double& lly, double& urx, double& ury);

private:
  bool BoundingBoxWithAtEnd(const string& line);

private:
  void ScanHeader();

private:
  int ReadDosBinary4();

private:
  void GetFirstLine(string& line);

private:
  void PrepareInput(bool runAsFilter, const PathName& inputFile);

private:
  void PrepareOutput(bool runAsFilter, bool runGhostscript, const PathName& gsExe, const vector<string>& gsExtra, const PathName& outFile);

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
  long stopReadingAt = 0;

private:
  FileStream outStream;

private:
  bool boundingBoxCorrected = false;

private:
  map<string, string> fontMap;

private:
  const regex findfont_regex{ "/([A-Za-z0-9_\\-]+)\\s+findfont" };

private:
  const regex fontmap_regex{ "/([A-Za-z0-9_\\-]+)\\s+([\\(/A-Za-z0-9_\\.\\)\\-]+)\\s*;" };

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
  shared_ptr<Session> session;

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

void EpsToPdfApp::MyTrace(const string& line)
{
  traceStream->WriteLine(PROGRAM_NAME, line);
}

void EpsToPdfApp::Verbose(const string& line)
{
  if (!verbose || printOnly)
  {
    return;
  }
  cout << line << endl;
}

void EpsToPdfApp::PrintOnly(const string& line)
{
  if (!printOnly)
  {
    return;
  }
  cout << line << endl;
}

void EpsToPdfApp::Warning(const string& line)
{
  cerr << T_("warning") << ": " << line << endl;
}

bool EpsToPdfApp::GetLine(string& line)
{
  if (stopReadingAt > 0)
  {
    long pos = inStream.GetPosition();
    if (pos >= stopReadingAt)
    {
      return false;
    }
  }
  bool done = Utils::ReadLine(line, inStream.GetFile(), true);
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

void EpsToPdfApp::PutLine(const string& line)
{
  if (!printOnly)
  {
    fprintf(outStream.GetFile(), "%s\n", line.c_str());
  }
}

void EpsToPdfApp::ExamineLine(const string& line)
{
  smatch m;
  string s = line;
  while (regex_search(s, m, findfont_regex))
  {
    EnsureFontIsAvailable(m[1]);
    s = m.suffix();
  }
}

void EpsToPdfApp::EnsureFontIsAvailable(const string& fontName)
{
  map<string, string>::const_iterator it = fontMap.find(fontName);
  if (it == fontMap.end())
  {
    Warning(fmt::format(T_("required font {0} not found in font map"), fontName));
    return;
  }
  const string& s = it->second;
  MIKTEX_ASSERT(!s.empty());
  if (s[0] == '(' && s[s.length() - 1] == ')')
  {
    string fileName = s.substr(1, s.length() - 2);
    PathName result;
    if (session->FindFile(fileName, "%R/fonts//", result))
    {
      MyTrace(fmt::format(T_("found required font {0}: {1}"), fontName, result));
    }
    else
    {
      Warning(fmt::format(T_("required font not found: {0} ({1})"), fontName, fileName));
    }
  }
  else if (s[0] == '/')
  {
    MIKTEX_ASSERT(s.length() > 1);
    EnsureFontIsAvailable(s.substr(1));
  }
  else
  {
    MIKTEX_UNEXPECTED();
  }
}

void EpsToPdfApp::ReadFontMapFile(const PathName& path)
{
  std::ifstream reader = File::CreateInputStream(path);
  size_t n = 0;
  for (string line; std::getline(reader, line); )
  {
    smatch m;
    if (regex_match(line, m, fontmap_regex))
    {
      fontMap[m[1]] = m[2];
      n++;
    }
  }
  MyTrace(fmt::format(T_("read font map file {0} with {1} matches"), Q_(path), n));
}

void EpsToPdfApp::CorrectBoundingBox(double llx, double lly, double urx, double ury)
{
  MyTrace(fmt::format(T_("Old BoundingBox: {0:.2f} {1:.2f} {2:.2f} {3:.2f}"), llx, lly, urx, ury));
  llx -= enlarge;
  lly -= enlarge;
  urx += enlarge;
  ury += enlarge;
  int width = static_cast<int>(ceil(urx - llx));
  int height = static_cast<int>(ceil(ury - lly));
  int xoffset = static_cast<int>(-llx);
  int yoffset = static_cast<int>(-lly);
  MyTrace(fmt::format(T_("New BoundingBox: 0 0 {0} {1}"), width, height));
  MyTrace(fmt::format(T_("Offset: {0} {1}"), xoffset, yoffset));
  PutLine(fmt::format("%%%%BoundingBox: 0 0 {0} {1}", width, height));
  PutLine(fmt::format("<< /PageSize [{0} {1}] >> setpagedevice", width, height));
  PutLine(fmt::format("gsave {0} {1} translate", xoffset, yoffset));
}

bool EpsToPdfApp::BoundingBoxWithValues(const string& line, double& llx, double& lly, double& urx, double& ury)
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

bool EpsToPdfApp::BoundingBoxWithAtEnd(const string& line)
{
  if (line.compare(0, boundingBoxName.length(), boundingBoxName) != 0)
  {
    return false;
  }
  const char* lpsz = line.c_str() + boundingBoxName.length();
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
      MyTrace(fmt::format(T_("{0} (atend)"), boundingBoxName));
      if (runAsFilter)
      {
        Warning(T_("Cannot look for BoundingBox in the trailer with option --filter."));
        break;
      }
      long pos = inStream.GetPosition();
      MyTrace(fmt::format(T_("Current file position: {0}"), pos));
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

void EpsToPdfApp::GetFirstLine(string& line)
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

void EpsToPdfApp::PrepareInput(bool runAsFilter, const PathName& inputFile)
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

void EpsToPdfApp::PrepareOutput(bool runAsFilter, bool runGhostscript, const PathName& gsExe, const vector<string>& gsExtra, const PathName& outFile)
{
  if (runGhostscript)
  {
    vector<string>gsOptions{ gsExe.GetFileName().ToString() };
    gsOptions.insert(gsOptions.end(), gsExtra.begin(), gsExtra.end());
    gsOptions.push_back("-q");
    gsOptions.push_back("-sDEVICE="s + "pdfwrite");
    gsOptions.push_back("-dSAFER");
#if 1                           // 642845
    gsOptions.push_back("-dAutoRotatePages="s + "/None");
#endif
#if 1                           // https://github.com/MiKTeX/miktex/issues/393
    gsOptions.push_back("-dPDFSETTINGS="s + "/prepress");
    gsOptions.push_back("-dMaxSubsetPct="s + "100");
    gsOptions.push_back("-dSubsetFonts="s + "true");
    gsOptions.push_back("-dEmbedAllFonts="s + "true");
#endif
    if (!pdfVersion.empty())
    {
      gsOptions.push_back("-dCompatibilityLevel="s + pdfVersion);
    }
    if (runAsFilter)
    {
      gsOptions.push_back("-sOutputFile="s + "-");
    }
    else
    {
      gsOptions.push_back("-sOutputFile="s + outFile.ToString());
    }
    gsOptions.push_back("-");
    gsOptions.push_back("-c");
    gsOptions.push_back("quit");
    PrintOnly(CommandLineBuilder(gsOptions).ToString());
    if (!printOnly)
    {
      ProcessStartInfo processStartInfo;
      processStartInfo.FileName = gsExe.ToString();
      processStartInfo.Arguments = gsOptions;
      processStartInfo.StandardInput = nullptr;
      processStartInfo.RedirectStandardError = false;
      processStartInfo.RedirectStandardInput = true;
      processStartInfo.RedirectStandardOutput = false;
      gsProcess = Process::Start(processStartInfo);
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

void EpsToPdfApp::Run(int argc, const char** argv)
{
  Session::InitInfo initInfo(argv[0]);
  vector<const char*> newargv(&argv[0], &argv[argc + 1]);
  ExamineArgs(newargv, initInfo);

  PathName outFile;

  PoptWrapper popt(newargv.size() - 1, &newargv[0], aoption);

  int option;

  bool antiAliasing = false;
  bool doCompress = true;

  vector<string> gsOptions;

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
      initInfo.SetTraceFlags(StringUtil::Flatten(DEFAULT_TRACE_OPTIONS, ','));
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
      gsOptions.push_back(optArg);
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
        initInfo.SetTraceFlags(StringUtil::Flatten(DEFAULT_TRACE_OPTIONS, ','));
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
        << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << endl
	<< endl
        << MIKTEX_COMP_COPYRIGHT_STR << endl
	<< endl
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
    FatalError(msg);
  }

  Init(initInfo);
  session = GetSession();

  PathName inputFile;

  vector<string> leftovers = popt.GetLeftovers();

  for (const string& fileName : { "Fontmap.aliases"s, "Fontmap.MiKTeX"s })
  {
    vector<PathName> fontMapFiles;
    if (session->FindFile(fileName, "%R/ghostscript/base", fontMapFiles))
    {
      for (const PathName& fontMapFile : fontMapFiles)
      {
        ReadFontMapFile(fontMapFile);
      }
    }
  }

  if (runAsFilter)
  {
    if (!leftovers.empty())
    {
      FatalError(T_("Input file cannot be specified together with --filter option."));
    }
    MyTrace(T_("Input file: standard input"));
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
    if (!session->FindFile(leftovers[0], ".", inputFile))
    {
      FatalError(T_("The input file does not exist."));
    }
    MyTrace(fmt::format(T_("Input filename: {0}"), inputFile.GetData()));
  }

  if (runAsFilter && verbose)
  {
    FatalError(T_("Options --filter and --verbose cannot be used together."));
  }

  if (antiAliasing)
  {
    gsOptions.push_back("-dTextAlphaBits="s + "4");
    gsOptions.push_back("-dGraphicsAlphaBits="s + "4");
  }

  if (!doCompress)
  {
    gsOptions.push_back("-dUseFlateCompression="s + "false");
  }

  boundingBoxName = "%%BoundingBox:";
  if (hiResBoundingBox && exactBoundingBox)
  {
    FatalError(T_("Options --hires and --exact cannot be used together."));
  }
  if (hiResBoundingBox)
  {
    boundingBoxName = "%%HiResBoundingBox:";
  }
  if (exactBoundingBox)
  {
    boundingBoxName = "%%ExactBoundingBox:";
  }

  if (outFile.GetLength() > 0 && runAsFilter)
  {
    FatalError(T_("Input file cannot be specified together with --filter option."));
  }

  if (outFile.Empty())
  {
    outFile = inputFile;
    if (runGhostscript)
    {
      outFile.SetExtension(".pdf");
    }
    else
    {
      outFile.SetExtension(nullptr);
      outFile.Append("2", false);
      outFile.AppendExtension(".eps");
    }
  }

  if (!runAsFilter)
  {
    Verbose(fmt::format(T_("Making {0} from {1}..."), Q_(outFile), Q_(inputFile)));
  }

  PathName gsExe;

  if (runGhostscript)
  {
    gsExe = session->GetGhostscript(nullptr);
  }

  PrepareInput(runAsFilter, inputFile);
  PrepareOutput(runAsFilter, runGhostscript, gsExe, gsOptions, outFile);

  string line;

  GetFirstLine(line);

  PutLine(line);

  ScanHeader();

  while (GetLine(line))
  {
    ExamineLine(line);
    PutLine(line);
  }

  if (boundingBoxCorrected)
  {
    PutLine("\ngrestore");
  }

  outStream.Close();

  if (!boundingBoxCorrected)
  {
    FatalError(T_("BoundingBox not found."));
  }

  if (gsProcess != nullptr)
  {
    gsProcess->WaitForExit();
    if (gsProcess->get_ExitStatus() != ProcessExitStatus::Exited)
    {
      FatalError(T_("Ghostscript exited unexpectedly."));
    }
    auto gsExitCode = gsProcess->get_ExitCode();
    gsProcess = nullptr;
    if (gsExitCode != 0)
    {
      FatalError(fmt::format(T_("Ghostscript exited with error {0}"), gsExitCode));
    }
  }

  Finalize2(0);
}

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR** argv)
{
#if defined(MIKTEX_WINDOWS)
  ConsoleCodePageSwitcher cpSwitcher;
#endif
  EpsToPdfApp app;
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char*> newargv;
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
    app.Run(argc, &newargv[0]);
    return 0;
  }
  catch (const MiKTeXException& e)
  {
    app.Sorry(THE_NAME_OF_THE_GAME, e);
    e.Save();
    return 1;
  }
  catch (const exception& e)
  {
    app.Sorry(THE_NAME_OF_THE_GAME, e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
