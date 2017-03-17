/* cjklatex.cpp: call latex after preprocessing of the .tex file by
   the cjk conversion tool

   Written in the years 2004-2017 by Christian Schenk.

   This file is based on public domain work (cjklatex.c, 2001) by
   Fabrice Popineau.

   To the extent possible under law, the author(s) have dedicated all
   copyright and related and neighboring rights to this file to the
   public domain worldwide.  This file is distributed without any
   warranty.  You should have received a copy of the CC0 Public Domain
   Dedication along with this file.  If not, see
   http://creativecommons.org/publicdomain/zero/1.0/. */

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <climits>
#include <cstdarg>
#include <cstdio>

#include <getopt.h>

#include "cjklatex-version.h"

#include <miktex/App/Application>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Util/StringUtil>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

enum Option
{
  OPT__FIRST = 1000,
  OPT_CONV,
  OPT_HELP,
  OPT_LATEX,
  OPT_NOCLEANUP,
  OPT_PRINT_ONLY,
  OPT_VERBOSE,
  OPT_VERSION,
};

const struct option aoption[] =
{
  "conv",           required_argument,      nullptr,      OPT_CONV,
  "latex",          required_argument,      nullptr,      OPT_LATEX,
  "help",           no_argument,            nullptr,      OPT_HELP,
  "nocleanup",      no_argument,            nullptr,      OPT_NOCLEANUP,
  "print-only",     no_argument,            nullptr,      OPT_PRINT_ONLY,
  "verbose",        no_argument,            nullptr,      OPT_VERBOSE,
  "version",        no_argument,            nullptr,      OPT_VERSION,
  nullptr,          no_argument,            nullptr,      0,
};

struct CONVERTER {
  const char* lpszName;
  const char* lpszProgram;
};

const CONVERTER Converters[] =
{
  "bg5",    "bg5conv",
  "bg5+",   "extconv",
  "bg5p",   "extconv",
  "cef",    "cefconv",
  "cef5",   "cef5conv",
  "cefs",   "cefsconv",
  "cjk",    nullptr,
  "gbk",    "extconv",
  "sjis",   "sjisconv",
};

class CJKLaTeXApp :
  public Application
{
private:
  void Error(const char* lpszFormat, ...);

private:
  void PrintOnly(const char* lpszFormat, ...);

private:
  const char* GetConverterProgram(const char* lpszName);

private:
  void ParseInvocationName(string& converterProgram, string& engine);

private:
  void RunConverter(const PathName& inputFile, const PathName& intermediateFile);

private:
  void RunEngine(const PathName& inputFile);

private:
  void ShowVersion();

private:
  void Help();

private:
  void ProcessOptions(int argc, char** argv);

public:
  void Run(int argc, char** argv);

private:
  string engine;

private:
  string converterProgram;

private:
  bool cleanUp = true;

private:
  bool printOnly = false;

private:
  shared_ptr<Session> session;
};

void CJKLaTeXApp::ShowVersion()
{
  cout << Utils::MakeProgramVersionString(Utils::GetExeName(), MIKTEX_COMPONENT_VERSION_STR)
    << T_("\n\
Written by Fabrice Popineau in 2001.  Further developed by\n\
Christian Schenk in the years 2004-2017.\n\
\n\
To the extent possible under law, the author(s) have dedicated all\n\
copyright and related and neighboring rights to this program to the\n\
public domain worldwide.  This program is distributed without any\n\
warranty.  You should have received a copy of the CC0 Public Domain\n\
Dedication along with this file.  If not, see\n\
http://creativecommons.org/publicdomain/zero/1.0/.")
<< endl;
}

void CJKLaTeXApp::Help()
{
  cout << T_("Usage: ") << Utils::GetExeName() << T_(" [OPTION...] FILE\n\
\n\
This program runs a converter on FILE and then LaTeX on the result.\n\
\n\
Options:\n\
--conv=FILTER             Set the conversion filter. One of:\n\
                            bg5+\n\
                            bg5\n\
                            bg5p\n\
                            cef5\n\
                            cef\n\
                            cefs\n\
                            gbk\n\
                            sjis\n\
--nocleanup               Keep intermediate files.\n\
--latex=PROGRAM           Use PROGRAM instead of latex.\n\
--help                    Print this help screen and exit.\n\
--verbose                 Print info about what is being done.\n\
--version                 Print the version number and exit.")
<< endl;
}

void CJKLaTeXApp::Error(const char* lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  cerr << Utils::GetExeName() << ": "
    << StringUtil::FormatStringVA(lpszFormat, arglist)
    << endl;
  va_end(arglist);
  throw 1;
}

void CJKLaTeXApp::PrintOnly(const char* lpszFormat, ...)
{
  if (!printOnly)
  {
    return;
  }
  va_list arglist;
  va_start(arglist, lpszFormat);
  cout << StringUtil::FormatStringVA(lpszFormat, arglist) << endl;
  va_end(arglist);
}

void CJKLaTeXApp::RunConverter(const PathName& inputFile, const PathName& intermediateFile)
{
  PathName converter;
  if (!session->FindFile(converterProgram, FileType::EXE, converter))
  {
    Error(T_("Converter %s not found."), converterProgram.c_str());
  }
#if defined(MIKTEX_WINDOWS)
  Utils::RemoveBlanksFromPathName(converter);
#endif
  CommandLineBuilder cmdLine;
  cmdLine.AppendArgument(converter);
  cmdLine.AppendStdinRedirection(inputFile);
  cmdLine.AppendStdoutRedirection(intermediateFile);
  PrintOnly("%s", cmdLine.ToString().c_str());
  if (!printOnly && !Process::ExecuteSystemCommand(cmdLine.ToString()))
  {
    Error(T_("Converter %s failed on %s."), converterProgram.c_str(), Q_(inputFile));
  }
}

void CJKLaTeXApp::RunEngine(const PathName& inputFile)
{
  PathName engineExe;
  if (!session->FindFile(engine, FileType::EXE, engineExe))
  {
    Error(T_("Engine %s not found."), engine.c_str());
  }
  CommandLineBuilder arguments;
  arguments.AppendArgument(inputFile.ToString());
  PrintOnly("%s %s", Q_(engineExe), arguments.ToString().c_str());
  if (!printOnly)
  {
    Process::Run(engineExe, arguments.ToString());
  }
}

const char* CJKLaTeXApp::GetConverterProgram(const char* lpszName)
{
  for (const CONVERTER& conv : Converters)
  {
    if (PathName::Compare(lpszName, conv.lpszName) == 0)
    {
      return conv.lpszProgram;
    }
  }
  return nullptr;
}

void CJKLaTeXApp::ParseInvocationName(string& converterProgram, string& engine)
{
  string invocationName = Utils::GetExeName();
  const char* lpszInvocationName = invocationName.c_str();
  size_t j = UINT_MAX;
  size_t len = 0;
  for (size_t i = 0; i < sizeof(Converters) / sizeof(Converters[0]); ++i)
  {
    size_t l = strlen(Converters[i].lpszName);
#if defined(MIKTEX_WINDOWS)
    if (_strnicmp(lpszInvocationName, Converters[i].lpszName, l) == 0
#else
    if (strncmp(lpszInvocationName, Converters[i].lpszName, l) == 0
#endif
      && l > len)
    {
      j = i;
      len = l;
    }
  }
  if (j == UINT_MAX)
  {
    Error(T_("Bad invocation."));
  }
  if (Converters[j].lpszProgram != nullptr)
  {
    converterProgram = Converters[j].lpszProgram;
  }
  engine = &lpszInvocationName[len];
}

void CJKLaTeXApp::ProcessOptions(int argc, char** argv)
{
  ParseInvocationName(converterProgram, engine);

  optind = 0;
  int optionChar;
  int optionIndex;

  while ((optionChar = getopt_long_only(argc, argv, "", aoption, &optionIndex)) != EOF)
  {
    switch (static_cast<Option>(optionChar))
    {

    case OPT_CONV:

    {
      const char* lpszConverterProgram = GetConverterProgram(optarg);
      if (lpszConverterProgram == nullptr)
      {
        Error(T_("The converter %s is unknown."), optarg);
      }
      converterProgram = lpszConverterProgram;
      break;
    }

    case OPT_HELP:

      Help();
      throw 0;

    case OPT_LATEX:

      engine = optarg;
      break;

    case OPT_PRINT_ONLY:

      printOnly = true;
      break;

    case OPT_NOCLEANUP:

      cleanUp = false;
      break;

    case OPT_VERBOSE:

      break;

    case OPT_VERSION:

      ShowVersion();
      throw 0;

    default:

      Error(T_("Invalid command-line."));
      break;
    }
  }

  EnableInstaller(TriState::True);
}

void CJKLaTeXApp::Run(int argc, char** argv)
{
  session = GetSession();
  ProcessOptions(argc, argv);
  for (int i = optind; i != argc; ++i)
  {
    PathName pathInputFile(argv[i]);
    pathInputFile.MakeAbsolute();
    PathName pathIntermediateFile(pathInputFile);
    pathIntermediateFile.AppendExtension(".cjk");
    RunConverter(pathInputFile, pathIntermediateFile);
    RunEngine(pathIntermediateFile);
    if (cleanUp && !printOnly)
    {
      File::Delete(pathIntermediateFile);
    }
  }
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
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<char*> newargv;
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
      // FIXME: eliminate const cast
      newargv.push_back(const_cast<char*>(utf8args[idx].c_str()));
    }
    newargv.push_back(nullptr);
    CJKLaTeXApp app;
    app.Init(newargv);
    app.Run(newargv.size() - 1, &newargv[0]);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException& ex)
  {
    Utils::PrintException(ex);
    return 1;
  }
  catch (const exception& ex)
  {
    Utils::PrintException(ex);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
