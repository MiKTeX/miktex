/* texmfapp.cpp:

   Copyright (C) 1996-2020 Christian Schenk

   This file is part of the MiKTeX TeXMF Library.

   The MiKTeX TeXMF Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX TeXMF Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX TeXMF Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/AutoResource>
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/Paths>
#include <miktex/Core/StreamReader>

#include <miktex/Trace/Trace>

#if defined(MIKTEX_TEXMF_SHARED)
#  define C4PEXPORT MIKTEXDLLEXPORT
#else
#  define C4PEXPORT
#endif
#define C1F0C63F01D5114A90DDF8FC10FF410B
#include "miktex/C4P/C4P.h"

#if defined(MIKTEX_TEXMF_SHARED)
#  define MIKTEXMFEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXMFEXPORT
#endif
#define B8C7815676699B4EA2DE96F0BD727276
#include "miktex/TeXAndFriends/TeXMFApp.h"

#include "internal.h"

#include "miktex/texmfapp.defaults.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::TeXAndFriends;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

class TeXMFApp::impl
{
public:
  int optBase;
public:
  string memoryDumpFileName;
public:
  unique_ptr<TraceStream> trace_time;
public:
  clock_t clockStart;
public:
  bool timeStatistics;
public:
  bool parseFirstLine;
public:
  bool showFileLineErrorMessages;
public:
  bool haltOnError;
public:
  bool isInitProgram;
public:
  bool isUnicodeApp;
public:
  bool recordFileNames;
public:
  bool disableExtensions;
public:
  bool setJobTime;
public:
  int interactionMode;
public:
  string jobName;
public:
  IStringHandler* stringHandler = nullptr;
public:
  IErrorHandler* errorHandler = nullptr;
public:
  ITeXMFMemoryHandler* memoryHandler = nullptr;
public:
  UserParams userParams;
};

TeXMFApp::TeXMFApp() :
  pimpl(make_unique<impl>())
{
}

TeXMFApp::~TeXMFApp() noexcept
{
}

STATICFUNC(void) TraceExecutionTime(TraceStream* trace_time, clock_t clockStart)
{
  clock_t clockSinceStart = clock() - clockStart;
  trace_time->WriteLine("libtexmf", fmt::format(T_("gross execution time: {0} ms"), clockSinceStart));
  cerr << fmt::format(T_("gross execution time: {0} ms\n"), clockSinceStart) << endl;
#if defined(MIKTEX_WINDOWS)
  HINSTANCE hinstKernel;
  hinstKernel = LoadLibraryW(L"kernel32.dll");
  if (hinstKernel == nullptr)
  {
    return;
  }
  FARPROC pfGetProcessTimes;
  pfGetProcessTimes = GetProcAddress(hinstKernel, "GetProcessTimes");
  if (pfGetProcessTimes == nullptr)
  {
    return;
  }
  FILETIME ftCreate, ftExit, ftKernel, ftUser;
  if (!GetProcessTimes(GetCurrentProcess(), &ftCreate, &ftExit, &ftKernel, &ftUser))
  {
    return;
  }
  LARGE_INTEGER tUser64;
  LARGE_INTEGER tKernel64;
  DWORD tUser, tKernel;
  tUser64.LowPart = ftUser.dwLowDateTime;
  tUser64.HighPart = ftUser.dwHighDateTime;
  tKernel64.LowPart = ftKernel.dwLowDateTime;
  tKernel64.HighPart = ftKernel.dwHighDateTime;
  tUser = static_cast<DWORD>(tUser64.QuadPart / 10000);
  tKernel = static_cast<DWORD>(tKernel64.QuadPart / 10000);
  trace_time->WriteLine("libtexmf", fmt::format(T_("user mode: {0} ms, kernel mode: {1} ms, total: {2}"), tUser, tKernel, tUser + tKernel));
  cerr
    << fmt::format(T_("user mode: {0} ms, kernel mode: {1} ms, total: {2}"), tUser, tKernel, tUser + tKernel)
    << endl;
#endif // MIKTEX_WINDOWS
}

void TeXMFApp::Init(vector<char*>& args)
{
  WebAppInputLine::Init(args);

  pimpl->trace_time = TraceStream::Open(MIKTEX_TRACE_TIME);

  pimpl->userParams.clear();

  pimpl->clockStart = clock();
  pimpl->disableExtensions = false;
  pimpl->haltOnError = false;
  pimpl->interactionMode = -1;
  pimpl->isInitProgram = false;
  pimpl->isUnicodeApp = AmI("xetex");
  pimpl->parseFirstLine = false;
  pimpl->recordFileNames = false;
  pimpl->setJobTime = false;
  pimpl->showFileLineErrorMessages = false;
  pimpl->timeStatistics = false;
}

void TeXMFApp::Finalize()
{
  if (pimpl->trace_time != nullptr)
  {
    pimpl->trace_time->Close();
    pimpl->trace_time = nullptr;
  }
  pimpl->memoryDumpFileName = "";
  pimpl->jobName = "";
  WebAppInputLine::Finalize();
}

void TeXMFApp::OnTeXMFStartJob()
{
  MIKTEX_ASSERT(!TheNameOfTheGame().empty());
  shared_ptr<Session> session = GetSession();
  string appName;
  for (const char& ch : TheNameOfTheGame())
  {
    if (ch != '-')         // pdf-e-tex => pdfetex
    {
      appName += ch;
    }
  }
  session->PushBackAppName(appName);
  pimpl->parseFirstLine = session->GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_PARSE_FIRST_LINE, ConfigValue(AmITeX())).GetBool();
  pimpl->showFileLineErrorMessages = session->GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_CSTYLEERRORS).GetBool();
  pimpl->clockStart = clock();
}

void TeXMFApp::OnTeXMFFinishJob()
{
  if (pimpl->recordFileNames)
  {
    string fileName;
    if (pimpl->jobName.length() > 2 && pimpl->jobName.front() == '"' && pimpl->jobName.back() == '"')
    {
      fileName = pimpl->jobName.substr(1, pimpl->jobName.length() - 2);
    }
    else
    {
      fileName = pimpl->jobName;
    }
    shared_ptr<Session> session = GetSession();
    PathName recorderPath = GetAuxDirectory();
    if (recorderPath.Empty())
    {
      recorderPath = GetOutputDirectory();
    }
    recorderPath /= fileName;
    recorderPath.AppendExtension(".fls");
    session->SetRecorderPath(recorderPath);
  }
  if (pimpl->timeStatistics)
  {
    TraceExecutionTime(pimpl->trace_time.get(), pimpl->clockStart);
  }
}

enum {
  OPT_AUX_DIRECTORY,
  OPT_BUF_SIZE,
  OPT_C_STYLE_ERRORS,
  OPT_DISABLE_8BIT_CHARS,
  OPT_DONT_PARSE_FIRST_LINE,
  OPT_ENABLE_8BIT_CHARS,
  OPT_ERROR_LINE,
  OPT_EXTRA_MEM_BOT,
  OPT_EXTRA_MEM_TOP,
  OPT_HALF_ERROR_LINE,
  OPT_HALT_ON_ERROR,
  OPT_INITIALIZE,
  OPT_INTERACTION,
  OPT_JOB_NAME,
  OPT_JOB_TIME,
  OPT_MAIN_MEMORY,
  OPT_MAX_PRINT_LINE,
  OPT_MAX_STRINGS,
  OPT_NO_C_STYLE_ERRORS,
  OPT_OUTPUT_DIRECTORY,
  OPT_PARAM_SIZE,
  OPT_PARSE_FIRST_LINE,
  OPT_POOL_FREE,
  OPT_POOL_SIZE,
  OPT_QUIET,
  OPT_RECORDER,
  OPT_STACK_SIZE,
  OPT_STRICT,
  OPT_STRING_VACANCIES,
  OPT_TCX,
  OPT_TIME_STATISTICS,
  OPT_UNDUMP,
};

void TeXMFApp::AddOptions()
{
  WebAppInputLine::AddOptions();

  bool invokedAsInitProgram = false;
  if (StringUtil::Contains(GetInitProgramName().c_str(), Utils::GetExeName().c_str()))
  {
    invokedAsInitProgram = true;
  }

  pimpl->optBase = (int)GetOptions().size();

  if (IsFeatureEnabled(Feature::EightBitChars))
  {
    AddOption(T_("enable-8bit-chars\0Make all characters printable by default."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ENABLE_8BIT_CHARS);
    AddOption(T_("disable-8bit-chars\0Make only 7-bit characters printable."), FIRST_OPTION_VAL + pimpl->optBase + OPT_DISABLE_8BIT_CHARS);
  }

  AddOption(T_("aux-directory\0Use DIR as the directory to write auxiliary files to."), FIRST_OPTION_VAL + pimpl->optBase + OPT_AUX_DIRECTORY, POPT_ARG_STRING, "DIR");
  AddOption(T_("buf-size\0Set buf_size to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_BUF_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("c-style-errors\0Enable file:line:error style messages."), FIRST_OPTION_VAL + pimpl->optBase + OPT_C_STYLE_ERRORS);
  AddOption(T_("dont-parse-first-line\0Do not parse the first line of the input line to look for a dump name and/or extra command-line options."), FIRST_OPTION_VAL + pimpl->optBase + OPT_DONT_PARSE_FIRST_LINE);
  AddOption(T_("error-line\0Set error_line to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ERROR_LINE, POPT_ARG_STRING, "N");

  if (AmITeX())
  {
    AddOption(T_("extra-mem-bot\0Set extra_mem_bot to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_EXTRA_MEM_BOT, POPT_ARG_STRING, "N");
  }

  if (AmITeX())
  {
    AddOption(T_("extra-mem-top\0Set extra_mem_top to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_EXTRA_MEM_TOP, POPT_ARG_STRING, "N");
  }

  AddOption(T_("half-error-line\0Set half_error_line to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_HALF_ERROR_LINE, POPT_ARG_STRING, "N");
  AddOption(T_("halt-on-error\0Stop after the first error."), FIRST_OPTION_VAL + pimpl->optBase + OPT_HALT_ON_ERROR);

  if (!invokedAsInitProgram)
  {
    AddOption(T_("initialize\0Be the INI variant of the program."), FIRST_OPTION_VAL + pimpl->optBase + OPT_INITIALIZE);
  }

  AddOption(T_("interaction\0Set the interaction mode; MODE must be one of: batchmode, nonstopmode, scrollmode, errorstopmode."), FIRST_OPTION_VAL + pimpl->optBase + OPT_INTERACTION, POPT_ARG_STRING, "MODE");
  AddOption(T_("job-name\0Set the job name and hence the name(s) of the output file(s)."), FIRST_OPTION_VAL + pimpl->optBase + OPT_JOB_NAME, POPT_ARG_STRING, "NAME");
  AddOption(T_("job-time\0Set the job time.  Take FILE's timestamp as the reference."), FIRST_OPTION_VAL + pimpl->optBase + OPT_JOB_TIME, POPT_ARG_STRING, "FILE");
  AddOption(T_("main-memory\0Set main_memory to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_MAIN_MEMORY, POPT_ARG_STRING, "N");
  AddOption(T_("max-print-line\0Set max_print_line to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_MAX_PRINT_LINE, POPT_ARG_STRING, "N");
  AddOption(T_("max-strings\0Set max_strings to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_MAX_STRINGS, POPT_ARG_STRING, "N");
  AddOption(T_("no-c-style-errors\0Disable file:line:error style messages."), FIRST_OPTION_VAL + pimpl->optBase + OPT_NO_C_STYLE_ERRORS);
  AddOption(T_("output-directory\0Use DIR as the directory to write output files to."), FIRST_OPTION_VAL + pimpl->optBase + OPT_OUTPUT_DIRECTORY, POPT_ARG_STRING, "DIR");
  AddOption(T_("param-size\0Set param_size to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_PARAM_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("parse-first-line\0Parse the first line of the input line to look for a dump name and/or extra command-line options."), FIRST_OPTION_VAL + pimpl->optBase + OPT_PARSE_FIRST_LINE, POPT_ARG_NONE);

  if (AmITeX())
  {
    AddOption(T_("pool-free\0Set pool_free to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_POOL_FREE, POPT_ARG_STRING, "N");
  }

  AddOption(T_("pool-size\0Set pool_size to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_POOL_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("quiet\0Suppress all output (except errors)."), FIRST_OPTION_VAL +pimpl-> optBase + OPT_QUIET);
  AddOption(T_("recorder\0Turn on the file name recorder to leave a trace of the files opened for input and output in a file with extension .fls."), FIRST_OPTION_VAL + pimpl->optBase + OPT_RECORDER);
  AddOption(T_("stack-size\0Set stack_size to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_STACK_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("strict\0Disable MiKTeX extensions."), FIRST_OPTION_VAL + pimpl->optBase + OPT_STRICT, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN);

  AddOption(T_("string-vacancies\0Set string_vacancies to N."), FIRST_OPTION_VAL + pimpl->optBase + OPT_STRING_VACANCIES, POPT_ARG_STRING, "N");
  AddOption(T_("time-statistics\0Show processing time statistics."), FIRST_OPTION_VAL + pimpl->optBase + OPT_TIME_STATISTICS);
  AddOption(T_("undump\0Use NAME instead of program name when loading internal tables."), FIRST_OPTION_VAL + pimpl->optBase + OPT_UNDUMP, POPT_ARG_STRING, "NAME");

  if (IsFeatureEnabled(Feature::TCX))
  {
    AddOption(T_("tcx\0Use the TCXNAME translation table to set the mapping of inputcharacters and re-mapping of output characters."), FIRST_OPTION_VAL + pimpl->optBase + OPT_TCX, POPT_ARG_STRING, "TCXNAME");
  }

  // old option names
  if (!invokedAsInitProgram)
  {
    AddOption("ini", "initialize");
  }
  AddOption("silent", "quiet");
  if (IsFeatureEnabled(Feature::TCX))
  {
    AddOption("translate-file", "tcx");
  }

  // supported Web2C options
  if (IsFeatureEnabled(Feature::EightBitChars))
  {
    AddOption("8bit", "enable-8bit-chars");
  }
  AddOption("file-line-error", "c-style-errors");
  AddOption("file-line-error-style", "c-style-errors");
  AddOption("jobname", "job-name");
  AddOption("no-file-line-error", "no-c-style-errors");
  AddOption("no-parse-first-line", "dont-parse-first-line");
  AddOption("progname", "alias");

  // unsupported Web2C options
  AddOption("default-translate-file\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("maketex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("mktex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("no-maketex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("no-mktex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
}

bool TeXMFApp::ProcessOption(int opt, const string& optArg)
{
  bool done = true;

  shared_ptr<Session> session = GetSession();

  switch (opt - FIRST_OPTION_VAL - pimpl->optBase)
  {

  case OPT_ENABLE_8BIT_CHARS:
    Enable8BitChars(true);
    break;

  case OPT_DISABLE_8BIT_CHARS:
    Enable8BitChars(false);
    break;

  case OPT_AUX_DIRECTORY:
  {
    PathName auxDirectory(optArg);
    auxDirectory.MakeFullyQualified();
    SetAuxDirectory(auxDirectory);
    if (!Directory::Exists(auxDirectory))
    {
      if (session->GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_CREATEAUXDIRECTORY).GetString() == "t")
      {
        Directory::Create(auxDirectory);
      }
      else
      {
        MIKTEX_FATAL_ERROR_2(T_("The specified auxiliary directory does not exist."), "directory", auxDirectory.ToString());
      }
    }
    session->AddInputDirectory(auxDirectory, true);
    break;
  }

  case OPT_BUF_SIZE:
    pimpl->userParams["buf_size"] = std::stoi(optArg);
    break;

  case OPT_C_STYLE_ERRORS:
    pimpl->showFileLineErrorMessages = true;
    break;

  case OPT_DONT_PARSE_FIRST_LINE:
    pimpl->parseFirstLine = false;
    break;

  case OPT_ERROR_LINE:
    pimpl->userParams["error_line"] = std::stoi(optArg);
    break;

  case OPT_EXTRA_MEM_BOT:
    pimpl->userParams["extra_mem_bot"] = std::stoi(optArg);
    break;

  case OPT_EXTRA_MEM_TOP:
    pimpl->userParams["extra_mem_top"] = std::stoi(optArg);
    break;

  case OPT_HALF_ERROR_LINE:
    pimpl->userParams["half_error_line"] = std::stoi(optArg);
    break;

  case OPT_HALT_ON_ERROR:
    pimpl->haltOnError = true;
    break;

  case OPT_INITIALIZE:
    pimpl->isInitProgram = true;
    break;

  case OPT_INTERACTION:
    if (optArg == "batchmode")
    {
      pimpl->interactionMode = 0;
    }
    else if (optArg == "nonstopmode")
    {
      pimpl->interactionMode = 1;
    }
    else if (optArg == "scrollmode")
    {
      pimpl->interactionMode = 2;
    }
    else if (optArg == "errorstopmode")
    {
      pimpl->interactionMode = 3;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Invalid interaction mode."), "interaction", optArg);
    }
    break;

  case OPT_JOB_NAME:
    if (AmI("xetex"))
    {
      pimpl->jobName = optArg;
    }
    else
    {
      pimpl->jobName = Q_(optArg);
    }
    break;

  case OPT_JOB_TIME:
  {
    if (optArg.empty())
    {
      MIKTEX_FATAL_ERROR(T_("Missing timestamp."));
    }
    time_t jobTime;
    if (isdigit(optArg[0]))
    {
      jobTime = Utils::ToTimeT(optArg);
    }
    else
    {
      time_t creationTime;
      time_t lastAccessTime;
      time_t lastWriteTime;
      File::GetTimes(PathName(optArg), creationTime, lastAccessTime, lastWriteTime);
      jobTime = lastWriteTime;
    }
    GetProgram()->SetStartUpTime(jobTime, false);
    pimpl->setJobTime = true;
  }
  break;

  case OPT_MAIN_MEMORY:
    pimpl->userParams["main_memory"] = std::stoi(optArg);
    break;

  case OPT_MAX_PRINT_LINE:
    pimpl->userParams["max_print_line"] = std::stoi(optArg);
    break;

  case OPT_MAX_STRINGS:
    pimpl->userParams["max_strings"] = std::stoi(optArg);
    break;

  case OPT_TIME_STATISTICS:
    pimpl->timeStatistics = true;
    break;

  case OPT_NO_C_STYLE_ERRORS:
    pimpl->showFileLineErrorMessages = false;
    break;

  case OPT_OUTPUT_DIRECTORY:
  {
    PathName outputDirectory(optArg);
    outputDirectory.MakeFullyQualified();
    SetOutputDirectory(outputDirectory);
    if (!Directory::Exists(outputDirectory))
    {
      if (session->GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_CREATEOUTPUTDIRECTORY).GetString() == "t")
      {
        Directory::Create(outputDirectory);
      }
      else
      {
        MIKTEX_FATAL_ERROR_2(T_("The specified output directory does not exist."), "directory", outputDirectory.GetData());
      }
    }
    if (GetAuxDirectory().Empty())
    {
      SetAuxDirectory(outputDirectory);
    }
    session->AddInputDirectory(outputDirectory, true);
    break;
  }

  case OPT_PARAM_SIZE:
    pimpl->userParams["param_size"] = std::stoi(optArg);
    break;

  case OPT_PARSE_FIRST_LINE:
    pimpl->parseFirstLine = true;
    break;

  case OPT_POOL_FREE:
    pimpl->userParams["pool_free"] = std::stoi(optArg);
    break;

  case OPT_POOL_SIZE:
    pimpl->userParams["pool_size"] = std::stoi(optArg);
    break;

  case OPT_QUIET:
    SetQuietFlag(true);
    break;

  case OPT_RECORDER:
    session->StartFileInfoRecorder(false);
    pimpl->recordFileNames = true;
    break;

  case OPT_STACK_SIZE:
    pimpl->userParams["stack_size"] = std::stoi(optArg);
    break;

  case OPT_STRICT:
    pimpl->disableExtensions = true;
    session->EnableFontMaker(false);
    break;

  case OPT_STRING_VACANCIES:
    pimpl->userParams["string_vacancies"] = std::stoi(optArg);
    break;

  case OPT_TCX:
    SetTcxFileName(PathName(optArg));
    break;

  case OPT_UNDUMP:
    pimpl->memoryDumpFileName = optArg;
    break;

  default:
    done = WebAppInputLine::ProcessOption(opt, optArg);
    break;
  }

  return done;
}

Argv TeXMFApp::ParseFirstLine(const PathName& path)
{
  StreamReader reader(path);

  string firstLine;

  if (!reader.ReadLine(firstLine))
  {
    return Argv();
  }

  reader.Close();

  if (!(firstLine.substr(0, 2) == "%&"))
  {
    return Argv();
  }

  return Argv(firstLine.c_str() + 2);
}

bool inParseFirstLine = false;

void TeXMFApp::CheckFirstLine(const PathName& fileName)
{
  inParseFirstLine = true;
  MIKTEX_AUTO(inParseFirstLine = false);

  PathName path;

  shared_ptr<Session> session = GetSession();

  if (!session->FindFile(fileName.ToString(), GetInputFileType(), path))
  {
    return;
  }

  Argv argv = ParseFirstLine(path);

  if (argv.GetArgc() == 0)
  {
    return;
  }

  int optidx;

  if (argv[0][0] != '-')
  {
    optidx = 1;
    if (pimpl->memoryDumpFileName.empty())
    {
      string memoryDumpFileName = argv[0];
      PathName fileName(memoryDumpFileName);
      if (!fileName.HasExtension())
      {
        fileName.SetExtension(GetMemoryDumpFileExtension());
      }
      PathName path;
      if (session->FindFile(fileName.ToString(), GetMemoryDumpFileType(), path))
      {
        pimpl->memoryDumpFileName = memoryDumpFileName;
      }
    }
  }
  else
  {
    optidx = 0;
  }

  int opt;

  if (optidx < argv.GetArgc())
  {
    for (PoptWrapper popt(argv.GetArgc() - optidx, const_cast<const char**>(argv.GetArgv()) + optidx, &(GetOptions())[0], POPT_CONTEXT_KEEP_FIRST); (opt = popt.GetNextOpt()) >= 0; )
    {
      ProcessOption(opt, popt.GetOptArg());
    }
  }
}

bool TeXMFApp::OpenMemoryDumpFile(const PathName& fileName_, FILE** ppFile, void* pBuf, size_t size, bool renew) const
{
  MIKTEX_ASSERT(ppFile != nullptr);

  if (pBuf != nullptr)
  {
    MIKTEX_ASSERT_BUFFER(pBuf, size);
  }

  shared_ptr<Session> session = GetSession();

  PathName fileName(fileName_);

  if (!fileName.HasExtension())
  {
    fileName.SetExtension(GetMemoryDumpFileExtension());
  }

  PathName path;

  string dumpName = fileName.GetFileNameWithoutExtension().ToString();
#if 0
  PathName::Convert(szDumpName, szDumpName, ConvertPathNameOption::MakeLower);
#endif

  Session::FindFileOptionSet findFileOptions;

  findFileOptions += Session::FindFileOption::Create;

  if (renew)
  {
    findFileOptions += Session::FindFileOption::Renew;
  }

  if (!session->FindFile(fileName.ToString(), GetMemoryDumpFileType(), findFileOptions, path))
  {
    MIKTEX_FATAL_ERROR_2(T_("The memory dump file could not be found."), "fileName", fileName.ToString());
  }

#if 1
  if (!renew)
  {
    time_t modificationTime = File::GetLastWriteTime(path);
    time_t lastAdminMaintenance = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_ADMIN_MAINTENANCE, ConfigValue("0")).GetTimeT();
    renew = lastAdminMaintenance > modificationTime;
    if (!renew && !session->IsAdminMode())
    {
      time_t lastUserMaintenance = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_LAST_USER_MAINTENANCE, ConfigValue("0")).GetTimeT();
      renew = lastUserMaintenance > modificationTime;
    }
    if (renew)
    {
      // RECURSION
      return OpenMemoryDumpFile(fileName_, ppFile, pBuf, size, true);
    }
  }
#endif

  FileStream stream(session->OpenFile(path, FileMode::Open, FileAccess::Read, false));

  if (pBuf != nullptr)
  {
    if (stream.Read(pBuf, size) != size)
    {
      MIKTEX_UNEXPECTED();
    }
  }

  session->PushAppName(dumpName);

  *ppFile = stream.Detach();

  return true;
}

void TeXMFApp::ProcessCommandLineOptions()
{
  if (StringUtil::Contains(GetInitProgramName().c_str(), Utils::GetExeName().c_str()))
  {
    pimpl->isInitProgram = true;
  }

  WebAppInputLine::ProcessCommandLineOptions();

  if (GetQuietFlag())
  {
    pimpl->showFileLineErrorMessages = true;
    pimpl->interactionMode = 0;      // batch_mode
  }

  if (pimpl->parseFirstLine
    && GetProgram()->GetArgC() > 1
    && GetProgram()->GetArgV()[1][0] != '&'
    && GetProgram()->GetArgV()[1][0] != '*' // <fixme/>
    && GetProgram()->GetArgV()[1][0] != '\\'
    && (pimpl->memoryDumpFileName.empty() || GetTcxFileName().Empty()))
  {
    CheckFirstLine(PathName(GetProgram()->GetArgV()[1]));
  }
}

bool TeXMFApp::IsVirgin() const
{
  string exeName = Utils::GetExeName();
  if (StringUtil::Contains(GetProgramName().c_str(), exeName.c_str())
    || StringUtil::Contains(GetVirginProgramName().c_str(), exeName.c_str()))
  {
    return true;
  }
#if 1
  size_t prefixLen = strlen(MIKTEX_PREFIX);
  if (exeName.compare(0, prefixLen, MIKTEX_PREFIX) == 0)
  {
    exeName = exeName.substr(prefixLen);
    if (StringUtil::Contains(GetProgramName().c_str(), exeName.c_str())
      || StringUtil::Contains(GetVirginProgramName().c_str(), exeName.c_str()))
    {
      return true;
    }
  }
#endif
  return false;
}

PathName TeXMFApp::GetDefaultMemoryDumpFileName() const
{
  PathName name;
  if (!pimpl->memoryDumpFileName.empty())
  {
    name = pimpl->memoryDumpFileName;
  }
  else if (IsVirgin())
  {
    name = GetMemoryDumpFileName();
  }
  else
  {
    string exeName = Utils::GetExeName();
    size_t prefixLen = strlen(MIKTEX_PREFIX);
    if (exeName.compare(0, prefixLen, MIKTEX_PREFIX) == 0)
    {
      name = exeName.substr(prefixLen);
    }
    else
    {
      name = exeName;
    }
  }
  name.AppendExtension(GetMemoryDumpFileExtension());
  return name;
}

bool IsFileNameArgument(const char* arg)
{
  for (size_t l = 0; arg[l] != 0; ++l)
  {
    char ch = arg[l];
    if (ch == '<'
      || ch == '>'
      || ch == '"'
      || ch == '|'
      || ch == '*')
    {
      return false;
    }
  }
  return true;
}

void TeXMFApp::InitializeBuffer() const
{
  IInputOutput* inout = GetInputOutput();
 
  int fileNameArgIdx = -1;
  PathName fileName;

  shared_ptr<Session> session = Session::Get();

  auto argc = GetProgram()->GetArgC();
  auto argv = GetProgram()->GetArgV();

  if (AmITeX())
  {
    /* test command-line for one of:
    (a) tex FILENAME
    (b) tex &FORMAT FILENAME
    (c) initex FILENAME \dump
    (d) initex &FORMAT FILENAME \dump
    */
    PathName path;
    if (argc == 2 && IsFileNameArgument(argv[1]) && session->FindFile(argv[1], GetInputFileType(), path))
    {
      fileNameArgIdx = 1;
    }
    else if (argc == 3 && argv[1][0] == '&' && IsFileNameArgument(argv[2]) && session->FindFile(argv[2], GetInputFileType(), path))
    {
      fileNameArgIdx = 2;
    }
    else if (argc == 3 && strcmp(argv[2], "\\dump") == 0 && IsFileNameArgument(argv[1]) && session->FindFile(argv[1], GetInputFileType(), path))
    {
      fileNameArgIdx = 1;
    }
    else if (argc == 4 && argv[1][0] == '&' && strcmp(argv[3], "\\dump") == 0 && IsFileNameArgument(argv[2]) && session->FindFile(argv[2], GetInputFileType(), path))
    {
      fileNameArgIdx = 2;
    }
    if (fileNameArgIdx >= 0)
    {
#if defined(MIKTEX_WINDOWS)
        fileName = Q_(path.ToUnix());
#else
        fileName = Q_(path);
#endif
    }
  }

  //MIKTEX_ASSERT(inout->first() == 1);
  C4P::C4P_signed32& last = inout->last();
  last = 1;
  char32_t* buffer32 = AmI("xetex") ? inout->buffer32() : nullptr;
  char*buffer = !IsUnicodeApp() ? inout->buffer() : nullptr;
  for (int idx = 1; idx < argc; ++idx)
  {
    if (idx > 1)
    {
      if (AmI("xetex"))
      {
        buffer32[last++] = U' ';
      }
      else
      {
        buffer[last++] = ' ';
      }
    }
    const char* lpszOptArg;
    if (idx == fileNameArgIdx)
    {
      lpszOptArg = fileName.GetData();
    }
    else
    {
      lpszOptArg = argv[idx];
    }
    if (AmI("xetex"))
    {
      for (const char32_t& ch : StringUtil::UTF8ToUTF32(lpszOptArg))
      {
        buffer32[last++] = ch;
      }
    }
    else
    {
      for (const char* lpsz = lpszOptArg; *lpsz != 0; ++lpsz)
      {
        buffer[last++] = *lpsz;
      }
    }
  }

  // clear the command-line
  GetProgram()->MakeCommandLine(vector<string>());
}

void TeXMFApp::TouchJobOutputFile(FILE* file) const
{
  MIKTEX_ASSERT(file != nullptr);
  shared_ptr<Session> session = GetSession();
  if (pimpl->setJobTime && session->IsOutputFile(file))
  {
    time_t time = GetProgram()->GetStartUpTime();
    File::SetTimes(file, time, time, time);
  }
}

bool TeXMFApp::CStyleErrorMessagesP() const
{
  return pimpl->showFileLineErrorMessages;
}

bool TeXMFApp::ParseFirstLineP() const
{
  return pimpl->parseFirstLine;
}

bool TeXMFApp::HaltOnErrorP() const
{
  return pimpl->haltOnError;
}

bool TeXMFApp::IsInitProgram() const
{
  return pimpl->isInitProgram;
}

bool TeXMFApp::IsUnicodeApp() const
{
  return pimpl->isUnicodeApp;
}

int TeXMFApp::GetInteraction() const
{
  return pimpl->interactionMode;
}

void TeXMFApp::SetStringHandler(IStringHandler* stringHandler)
{
  pimpl->stringHandler = stringHandler;
}

IStringHandler* TeXMFApp::GetStringHandler() const
{
  return pimpl->stringHandler;
}

void TeXMFApp::SetErrorHandler(IErrorHandler* errorHandler)
{
  pimpl->errorHandler = errorHandler;
}

IErrorHandler* TeXMFApp::GetErrorHandler() const
{
  return pimpl->errorHandler;
}

void TeXMFApp::SetTeXMFMemoryHandler(ITeXMFMemoryHandler* memoryHandler)
{
  pimpl->memoryHandler = memoryHandler;
}

ITeXMFMemoryHandler* TeXMFApp::GetTeXMFMemoryHandler() const
{
  return pimpl->memoryHandler;
}

TeXMFApp::UserParams& TeXMFApp::GetUserParams() const
{
  return pimpl->userParams;
}

void TeXMFApp::OnKeybordInterrupt(int)
{
  signal(SIGINT, SIG_IGN);
  ((TeXMFApp*)GetApplication())->GetErrorHandler()->interrupt() = 1;
  signal(SIGINT, OnKeybordInterrupt);
}

string TeXMFApp::GetTeXString(int stringStart, int stringLength) const
{
  if (IsUnicodeApp())
  {
    return StringUtil::UTF16ToUTF8(u16string(GetStringHandler()->strpool16() + stringStart, stringLength));
  }
  else
  {    
    return string(GetStringHandler()->strpool() + stringStart, stringLength);
  }
}

int TeXMFApp::MakeTeXString(const char* lpsz) const
{
  MIKTEX_ASSERT_STRING(lpsz);
  IStringHandler* stringHandler = GetStringHandler();
  std::size_t len;
  if (IsUnicodeApp())
  {
    u16string s = StringUtil::UTF8ToUTF16(lpsz);
    len = s.length();
    CheckPoolPointer(stringHandler->poolptr(), len);
    memcpy(stringHandler->strpool16() + stringHandler->poolptr(), s.c_str(), len * sizeof(char16_t));
  }
  else
  {
    len = StrLen(lpsz);
    CheckPoolPointer(stringHandler->poolptr(), len);
    memcpy(stringHandler->strpool() + stringHandler->poolptr(), lpsz, len * sizeof(char));
  }
  stringHandler->poolptr() += static_cast<C4P::C4P_signed32>(len);
  return stringHandler->makestring();
}

int TeXMFApp::GetJobName(int fallbackJobName) const
{
  if (pimpl->jobName.empty())
  {
    if (GetLastInputFileName().Empty())
    {
      pimpl->jobName = GetTeXString(fallbackJobName);
      MIKTEX_EXPECT(pimpl->jobName.find(' ') == string::npos);
      return fallbackJobName;
    }
    PathName name = GetLastInputFileName().GetFileNameWithoutExtension();
    if (AmI("xetex"))
    {
      pimpl->jobName = name.ToString();
    }
    else
    {
      pimpl->jobName = Quoter<char>(name).GetData();
    }
  }
  // FIXME: conserve strpool space
  return MakeTeXString(pimpl->jobName.c_str());
}

int TeXMFApp::GetTeXStringStart(int stringNumber) const
{
  if (IsUnicodeApp())
  {
    MIKTEX_ASSERT(stringNumber >= 65536);
    stringNumber -= 65536;
  }
  IStringHandler* stringHandler = GetStringHandler();
  MIKTEX_ASSERT(stringNumber >= 0 && stringNumber < stringHandler->strptr());
  return stringHandler->strstart()[stringNumber];
}

int TeXMFApp::GetTeXStringLength(int stringNumber) const
{
  if (IsUnicodeApp())
  {
    MIKTEX_ASSERT(stringNumber >= 65536);
    stringNumber -= 65536;
  }
  IStringHandler* stringHandler = GetStringHandler();
  MIKTEX_ASSERT(stringNumber >= 0 && stringNumber < stringHandler->strptr());
  return stringHandler->strstart()[stringNumber + 1] - stringHandler->strstart()[stringNumber];
}

void TeXMFApp::InvokeEditor(int editFileName, int editFileNameLength, int editLineNumber, int transcriptFileName, int transcriptFileNameLength) const
{
  Application::InvokeEditor(PathName(GetTeXString(editFileName, editFileNameLength)), editLineNumber, GetInputFileType(), transcriptFileName == 0 ? PathName() : PathName(GetTeXString(transcriptFileName, transcriptFileNameLength)));
}

void TeXMFApp::CheckPoolPointer(int poolptr, std::size_t len) const
{
  if (poolptr + len >= GetStringHandler()->poolsize())
  {
    MIKTEX_FATAL_ERROR(MIKTEXTEXT("String pool overflow."));
  }
}
