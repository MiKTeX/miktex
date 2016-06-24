/* texmfapp.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

#include "StdAfx.h"

#include "internal.h"

STATICFUNC(void) TraceExecutionTime(TraceStream * trace_time, clock_t clockStart)
{
  clock_t clockSinceStart = clock() - clockStart;
  trace_time->WriteFormattedLine("libtexmf", T_("gross execution time: %u ms"), static_cast<unsigned>(clockSinceStart));
  cerr << StringUtil::FormatString(T_("gross execution time: %u ms\n"), static_cast<unsigned>(clockSinceStart)) << endl;
#if defined(MIKTEX_WINDOWS)
  HINSTANCE hinstKernel;
  hinstKernel = LoadLibraryA("kernel32.dll");
  if (hinstKernel == 0)
  {
    return;
  }
  FARPROC pfGetProcessTimes;
  pfGetProcessTimes = GetProcAddress(hinstKernel, "GetProcessTimes");
  if (pfGetProcessTimes == 0)
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
  trace_time->WriteFormattedLine("libtexmf", T_("user mode: %u ms, kernel mode: %u ms, total: %u"), static_cast<unsigned>(tUser), static_cast<unsigned>(tKernel), static_cast<unsigned>(tUser + tKernel));
  cerr
    << StringUtil::FormatString(T_("user mode: %u ms, kernel mode: %u ms, total: %u"), static_cast<unsigned>(tUser), static_cast<unsigned>(tKernel), static_cast<unsigned>(tUser + tKernel))
    << endl;
#endif // MIKTEX_WINDOWS
}

void TeXMFApp::Init(const char * lpszProgramInvocationName)
{
  WebAppInputLine::Init(lpszProgramInvocationName);

  trace_time = TraceStream::Open(MIKTEX_TRACE_TIME);
  trace_mem = TraceStream::Open(MIKTEX_TRACE_MEM);

  clockStart = clock();
  disableExtensions = false;
  haltOnError = false;
  interactionMode = -1;
  isInitProgram = false;
  isTeXProgram = false;
  param_buf_size = -1;
  param_error_line = -1;
  param_extra_mem_bot = -1;
  param_extra_mem_top = -1;
  param_half_error_line = -1;
  param_max_print_line = -1;
  param_max_strings = -1;
  param_main_memory = -1;
  param_param_size = -1;
  param_pool_free = -1;
  param_pool_size = -1;
  param_stack_size = -1;
  param_strings_free = -1;
  param_string_vacancies = -1;
  parseFirstLine = false;
  recordFileNames = false;
  setJobTime = false;
  showFileLineErrorMessages = false;
  timeStatistics = false;
}

void TeXMFApp::Finalize()
{
  if (trace_time != nullptr)
  {
    trace_time->Close();
    trace_time = nullptr;
  }
  if (trace_mem != nullptr)
  {
    trace_mem->Close();
    trace_mem = nullptr;
  }
  memoryDumpFileName = "";
  jobName = "";
  WebAppInputLine::Finalize();
}

void TeXMFApp::OnTeXMFStartJob()
{
  MIKTEX_ASSERT_STRING(TheNameOfTheGame());
  string appName;
  for (const char * lpsz = TheNameOfTheGame(); *lpsz != 0; ++lpsz)
  {
    if (*lpsz != '-')         // pdf-e-tex => pdfetex
    {
      appName += *lpsz;
    }
  }
  session->PushBackAppName(appName);
  parseFirstLine = session->GetConfigValue(nullptr, MIKTEX_REGVAL_PARSE_FIRST_LINE, isTeXProgram);
  showFileLineErrorMessages = session->GetConfigValue(nullptr, MIKTEX_REGVAL_C_STYLE_ERRORS, false);
  EnablePipes(session->GetConfigValue(nullptr, MIKTEX_REGVAL_ENABLE_PIPES, false));
  clockStart = clock();
}

void TeXMFApp::OnTeXMFFinishJob()
{
  if (recordFileNames)
  {
    string fileName;
    if (jobName.length() > 2 && jobName.front() == '"' && jobName.back() == '"')
    {
      fileName = jobName.substr(1, jobName.length() - 2);
    }
    else
    {
      fileName = jobName;
    }
    session->SetRecorderPath(PathName(outputDirectory.Empty() ? nullptr : outputDirectory.Get(), fileName.c_str(), ".fls"));
  }
  if (timeStatistics)
  {
    TraceExecutionTime(trace_time.get(), clockStart);
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
  OPT_STRINGS_FREE,
  OPT_STRING_VACANCIES,
  OPT_TCX,
  OPT_TIME_STATISTICS,
  OPT_UNDUMP,
};

void TeXMFApp::AddOptions()
{
  WebAppInputLine::AddOptions();

  bool invokedAsInitProgram = false;
  if (StringUtil::Contains(GetInitProgramName(), Utils::GetExeName().c_str()))
  {
    invokedAsInitProgram = true;
  }

  optBase = static_cast<int>(GetOptions().size());

  if (IsFeatureEnabled(Feature::EightBitChars))
  {
    AddOption(T_("enable-8bit-chars\0Make all characters printable by default."), FIRST_OPTION_VAL + optBase + OPT_ENABLE_8BIT_CHARS);
    AddOption(T_("disable-8bit-chars\0Make only 7-bit characters printable by."), FIRST_OPTION_VAL + optBase + OPT_DISABLE_8BIT_CHARS);
  }

  AddOption(T_("aux-directory\0Use DIR as the directory to write auxiliary files to."), FIRST_OPTION_VAL + optBase + OPT_AUX_DIRECTORY, POPT_ARG_STRING, "DIR");
  AddOption(T_("buf-size\0Set buf_size to N."), FIRST_OPTION_VAL + optBase + OPT_BUF_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("c-style-errors\0Enable file:line:error style messages."), FIRST_OPTION_VAL + optBase + OPT_C_STYLE_ERRORS);
  AddOption(T_("dont-parse-first-line\0Do not parse the first line of the input line to look for a dump name and/or extra command-line options."), FIRST_OPTION_VAL + optBase + OPT_DONT_PARSE_FIRST_LINE);
  AddOption(T_("error-line\0Set error_line to N."), FIRST_OPTION_VAL + optBase + OPT_ERROR_LINE, POPT_ARG_STRING, "N");

  if (isTeXProgram)
  {
    AddOption(T_("extra-mem-bot\0Set extra_mem_bot to N."), FIRST_OPTION_VAL + optBase + OPT_EXTRA_MEM_BOT, POPT_ARG_STRING, "N");
  }

  if (isTeXProgram)
  {
    AddOption(T_("extra-mem-top\0Set extra_mem_top to N."), FIRST_OPTION_VAL + optBase + OPT_EXTRA_MEM_TOP, POPT_ARG_STRING, "N");
  }

  AddOption(T_("half-error-line\0Set half_error_line to N."), FIRST_OPTION_VAL + optBase + OPT_HALF_ERROR_LINE, POPT_ARG_STRING, "N");
  AddOption(T_("halt-on-error\0Stop after the first error."), FIRST_OPTION_VAL + optBase + OPT_HALT_ON_ERROR);

  if (!invokedAsInitProgram)
  {
    AddOption(T_("initialize\0Be the INI variant of the program."), FIRST_OPTION_VAL + optBase + OPT_INITIALIZE);
  }

  AddOption(T_("interaction\0Set the interaction mode; MODE must be one of: batchmode, nonstopmode, scrollmode, errorstopmode."), FIRST_OPTION_VAL + optBase + OPT_INTERACTION, POPT_ARG_STRING, "MODE");
  AddOption(T_("job-name\0Set the job name and hence the name(s) of the output file(s)."), FIRST_OPTION_VAL + optBase + OPT_JOB_NAME, POPT_ARG_STRING, "NAME");
  AddOption(T_("job-time\0Set the job time.  Take FILE's timestamp as the reference."), FIRST_OPTION_VAL + optBase + OPT_JOB_TIME, POPT_ARG_STRING, "FILE");
  AddOption(T_("main-memory\0Set main_memory to N."), FIRST_OPTION_VAL + optBase + OPT_MAIN_MEMORY, POPT_ARG_STRING, "N");
  AddOption(T_("max-print-line\0Set max_print_line to N."), FIRST_OPTION_VAL + optBase + OPT_MAX_PRINT_LINE, POPT_ARG_STRING, "N");
  AddOption(T_("max-strings\0Set max_strings to N."), FIRST_OPTION_VAL + optBase + OPT_MAX_STRINGS, POPT_ARG_STRING, "N");
  AddOption(T_("no-c-style-errors\0Disable file:line:error style messages."), FIRST_OPTION_VAL + optBase + OPT_NO_C_STYLE_ERRORS);
  AddOption(T_("output-directory\0Use DIR as the directory to write output files to."), FIRST_OPTION_VAL + optBase + OPT_OUTPUT_DIRECTORY, POPT_ARG_STRING, "DIR");
  AddOption(T_("param-size\0Set param_size to N."), FIRST_OPTION_VAL + optBase + OPT_PARAM_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("parse-first-line\0Parse the first line of the input line to look for a dump name and/or extra command-line options."), FIRST_OPTION_VAL + optBase + OPT_PARSE_FIRST_LINE, POPT_ARG_NONE);

  if (isTeXProgram)
  {
    AddOption(T_("pool-free\0Set pool_free to N."), FIRST_OPTION_VAL + optBase + OPT_POOL_FREE, POPT_ARG_STRING, "N");
  }

  AddOption(T_("pool-size\0Set pool_size to N."), FIRST_OPTION_VAL + optBase + OPT_POOL_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("quiet\0Suppress all output (except errors)."), FIRST_OPTION_VAL + optBase + OPT_QUIET);
  AddOption(T_("recorder\0Turn on the file name recorder to leave a trace of the files opened for input and output in a file with extension .fls."), FIRST_OPTION_VAL + optBase + OPT_RECORDER);
  AddOption(T_("stack-size\0Set stack_size to N."), FIRST_OPTION_VAL + optBase + OPT_STACK_SIZE, POPT_ARG_STRING, "N");
  AddOption(T_("strict\0Disable MiKTeX extensions."), FIRST_OPTION_VAL + optBase + OPT_STRICT, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN);

  if (isTeXProgram)
  {
    AddOption(T_("strings-free\0Set strings_free to N."), FIRST_OPTION_VAL + optBase + OPT_STRINGS_FREE, POPT_ARG_STRING, "N");
  }

  AddOption(T_("string-vacancies\0Set string_vacancies to N."), FIRST_OPTION_VAL + optBase + OPT_STRING_VACANCIES, POPT_ARG_STRING, "N");
  AddOption(T_("time-statistics\0Show processing time statistics."), FIRST_OPTION_VAL + optBase + OPT_TIME_STATISTICS);
  AddOption(T_("undump\0Use NAME instead of program name when loading internal tables."), FIRST_OPTION_VAL + optBase + OPT_UNDUMP, POPT_ARG_STRING, "NAME");

  if (IsFeatureEnabled(Feature::TCX))
  {
    AddOption(T_("tcx\0Use the TCXNAME translation table to set the mapping of inputcharacters and re-mapping of output characters."), FIRST_OPTION_VAL + optBase + OPT_TCX, POPT_ARG_STRING, "TCXNAME");
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
  AddOption("default-translate-file", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("maketex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("mktex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("no-maketex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption("no-mktex\0", OPT_UNSUPPORTED, POPT_ARG_STRING);
}

bool TeXMFApp::ProcessOption(int opt, const string & optArg)
{
  bool done = true;

  switch (opt - FIRST_OPTION_VAL - optBase)
  {

  case OPT_ENABLE_8BIT_CHARS:
    Enable8BitChars(true);
    break;

  case OPT_DISABLE_8BIT_CHARS:
    Enable8BitChars(false);
    break;

  case OPT_AUX_DIRECTORY:
    auxDirectory = optArg;
    auxDirectory.MakeAbsolute();
    if (!Directory::Exists(auxDirectory))
    {
      if (session->GetConfigValue(nullptr, MIKTEX_REGVAL_CREATE_AUX_DIRECTORY, texmfapp::texmfapp::CreateAuxDirectory().c_str()) == "t")
      {
        Directory::Create(auxDirectory);
      }
      else
      {
        MIKTEX_FATAL_ERROR_2(T_("The specified auxiliary directory does not exist."), "directory", auxDirectory.ToString());
      }
    }
    session->AddInputDirectory(auxDirectory.Get(), true);
    break;

  case OPT_BUF_SIZE:
    param_buf_size = std::stoi(optArg);
    break;

  case OPT_C_STYLE_ERRORS:
    showFileLineErrorMessages = true;
    break;

  case OPT_DONT_PARSE_FIRST_LINE:
    parseFirstLine = false;
    break;

  case OPT_ERROR_LINE:
    param_error_line = std::stoi(optArg);
    break;

  case OPT_EXTRA_MEM_BOT:
    param_extra_mem_bot = std::stoi(optArg);
    break;

  case OPT_EXTRA_MEM_TOP:
    param_extra_mem_top = std::stoi(optArg);
    break;

  case OPT_HALF_ERROR_LINE:
    param_half_error_line = std::stoi(optArg);
    break;

  case OPT_HALT_ON_ERROR:
    haltOnError = true;
    break;

  case OPT_INITIALIZE:
    isInitProgram = true;
    break;

  case OPT_INTERACTION:
    if (optArg == "batchmode")
    {
      interactionMode = 0;
    }
    else if (optArg == "nonstopmode")
    {
      interactionMode = 1;
    }
    else if (optArg == "scrollmode")
    {
      interactionMode = 2;
    }
    else if (optArg == "errorstopmode")
    {
      interactionMode = 3;
    }
    else
    {
      MIKTEX_FATAL_ERROR_2(T_("Invalid interaction mode."), "interaction", optArg);
    }
    break;

  case OPT_JOB_NAME:
    if (!IsNameManglingEnabled)
    {
      if (!AmI("xetex"))
      {
	jobName = Q_(optArg);
      }
      else
      {
	jobName = optArg;
      }
    }
    else
    {
      jobName = MangleNameOfFile(optArg.c_str()).Get();
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
      jobTime = std::stoi(optArg);
    }
    else
    {
      time_t creationTime;
      time_t lastAccessTime;
      time_t lastWriteTime;
      File::GetTimes(optArg, creationTime, lastAccessTime, lastWriteTime);
      jobTime = lastWriteTime;
    }
    SetStartUpTime(jobTime);
    setJobTime = true;
  }
  break;

  case OPT_MAIN_MEMORY:
    param_main_memory = std::stoi(optArg);
    break;

  case OPT_MAX_PRINT_LINE:
    param_max_print_line = std::stoi(optArg);
    break;

  case OPT_MAX_STRINGS:
    param_max_strings = std::stoi(optArg);
    break;

  case OPT_TIME_STATISTICS:
    timeStatistics = true;
    break;

  case OPT_NO_C_STYLE_ERRORS:
    showFileLineErrorMessages = false;
    break;

  case OPT_OUTPUT_DIRECTORY:
    outputDirectory = optArg;
    outputDirectory.MakeAbsolute();
    if (!Directory::Exists(outputDirectory))
    {
      if (session->GetConfigValue(nullptr, MIKTEX_REGVAL_CREATE_OUTPUT_DIRECTORY, texmfapp::texmfapp::CreateOutputDirectory().c_str()) == "t")
      {
        Directory::Create(outputDirectory);
      }
      else
      {
        MIKTEX_FATAL_ERROR_2(T_("The specified output directory does not exist."), "directory", outputDirectory.Get());
      }
    }
    if (auxDirectory[0] == 0)
    {
      auxDirectory = outputDirectory;
    }
    session->AddInputDirectory(outputDirectory.Get(), true);
    break;

  case OPT_PARAM_SIZE:
    param_param_size = std::stoi(optArg);
    break;

  case OPT_PARSE_FIRST_LINE:
    parseFirstLine = true;
    break;

  case OPT_POOL_FREE:
    param_pool_free = std::stoi(optArg);
    break;

  case OPT_POOL_SIZE:
    param_pool_size = std::stoi(optArg);
    break;

  case OPT_QUIET:
    SetQuietFlag(true);
    break;

  case OPT_RECORDER:
    session->StartFileInfoRecorder(false);
    recordFileNames = true;
    break;

  case OPT_STACK_SIZE:
    param_stack_size = std::stoi(optArg);
    break;

  case OPT_STRICT:
    disableExtensions = true;
    session->EnableFontMaker(false);
    break;

  case OPT_STRING_VACANCIES:
    param_string_vacancies = std::stoi(optArg);
    break;

  case OPT_TCX:
    SetTcxFileName(optArg.c_str());
    break;

  case OPT_UNDUMP:
    memoryDumpFileName = optArg;
    break;

  default:
    done = WebAppInputLine::ProcessOption(opt, optArg);
    break;
  }

  return done;
}

bool TeXMFApp::ParseFirstLine(const PathName & path, MiKTeX::Core::Argv & argv)
{
  StreamReader reader(path);

  string firstLine;

  if (!reader.ReadLine(firstLine))
  {
    return false;
  }

  reader.Close();

  if (!(firstLine.substr(0, 2) == "%&"))
  {
    return false;
  }

  argv.Build("foo", firstLine.c_str() + 2);

  return argv.GetArgc() > 1;
}

bool inParseFirstLine = false;

void TeXMFApp::ParseFirstLine(const PathName & fileName)
{
  AutoRestore<bool> autoRestoreInParseFirstLine(inParseFirstLine);
  inParseFirstLine = true;

  PathName path;

  if (!session->FindFile(fileName.Get(), GetInputFileType(), path))
  {
    return;
  }

  Argv argv;

  if (!ParseFirstLine(path, argv))
  {
    return;
  }

  int optidx;

  if (argv.GetArgc() > 1 && argv[1][0] != '-')
  {
    optidx = 2;
    if (memoryDumpFileName.empty())
    {
      string memoryDumpFileName = argv[1];
      PathName fileName(memoryDumpFileName);
      if (fileName.GetExtension() == nullptr)
      {
	fileName.SetExtension(GetMemoryDumpFileExtension());
      }
      PathName path;
      if (session->FindFile(fileName.Get(), GetMemoryDumpFileType(), path))
      {
	this->memoryDumpFileName = memoryDumpFileName;
      }
    }
  }
  else
  {
    optidx = 1;
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

bool TeXMFApp::OpenMemoryDumpFile(const PathName & fileName_, FILE ** ppFile, void * pBuf, size_t size, bool renew) const
{
  MIKTEX_ASSERT(ppFile);

  if (pBuf != nullptr)
  {
    MIKTEX_ASSERT_BUFFER(pBuf, size);
  }

  PathName fileName(fileName_);

  if (fileName.GetExtension() == nullptr)
  {
    fileName.SetExtension(GetMemoryDumpFileExtension());
  }

  char szProgName[BufferSizes::MaxPath];
  GetProgName(szProgName);
#if 0
  PathName::Convert(szProgName, szProgName, ConvertPathNameOption::MakeLower);
#endif

  PathName path;

  char szDumpName[BufferSizes::MaxPath];
  fileName.GetFileNameWithoutExtension(szDumpName);
#if 0
  PathName::Convert(szDumpName, szDumpName, ConvertPathNameOption::MakeLower);
#endif

  Session::FindFileOptionSet findFileOptions;

  findFileOptions += Session::FindFileOption::Create;

  if (renew)
  {
    findFileOptions += Session::FindFileOption::Renew;
  }

  if (!session->FindFile(fileName.Get(), GetMemoryDumpFileType(), findFileOptions, path))
  {
    MIKTEX_FATAL_ERROR_2(T_("The memory dump file could not be found."), "fileName", fileName.ToString());
  }

  FileStream stream(session->OpenFile(path.Get(), FileMode::Open, FileAccess::Read, false));

  if (pBuf != nullptr)
  {
    if (stream.Read(pBuf, size) != size)
    {
      MIKTEX_UNEXPECTED();
    }
  }

  session->PushAppName(szDumpName);

  *ppFile = stream.Detach();

  return true;
}

void TeXMFApp::ProcessCommandLineOptions()
{
  if (StringUtil::Contains(GetInitProgramName(), Utils::GetExeName().c_str()))
  {
    isInitProgram = true;
  }

  WebAppInputLine::ProcessCommandLineOptions();

  if (GetQuietFlag())
  {
    showFileLineErrorMessages = true;
    interactionMode = 0;      // batch_mode
  }

  if (showFileLineErrorMessages && interactionMode < 0)
  {
    interactionMode = 2;      // scrollmode
  }

  if (parseFirstLine
    && GetArgC() > 1
    && GetArgV()[1][0] != '&'
    && GetArgV()[1][0] != '*' // <fixme/>
    && GetArgV()[1][0] != '\\'
    && (memoryDumpFileName.empty() || GetTcxFileName().Empty()))
  {
    ParseFirstLine(GetArgV()[1]);
  }
}

bool TeXMFApp::IsVirgin() const
{
  string exeName = Utils::GetExeName();
  if (StringUtil::Contains(GetProgramName(), exeName.c_str())
    || StringUtil::Contains(GetVirginProgramName(), exeName.c_str()))
  {
    return true;
  }
#if 1
  size_t prefixLen = strlen(MIKTEX_PREFIX);
  if (exeName.compare(0, prefixLen, MIKTEX_PREFIX) == 0)
  {
    exeName = exeName.substr(prefixLen);
    if (StringUtil::Contains(GetProgramName(), exeName.c_str())
      || StringUtil::Contains(GetVirginProgramName(), exeName.c_str()))
    {
      return true;
    }
  }
#endif
  return false;
}

void TeXMFApp::GetDefaultMemoryDumpFileName(char * lpszPath) const
{
  MIKTEX_ASSERT_PATH_BUFFER(lpszPath);
  PathName name;
  if (!memoryDumpFileName.empty())
  {
    name = memoryDumpFileName;
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
  name.SetExtension(GetMemoryDumpFileExtension());
  StringUtil::CopyString(lpszPath, BufferSizes::MaxPath, name.Get());
}

bool IsFileNameArgument(const char * lpszArg)
{
  for (size_t l = 0; lpszArg[l] != 0; ++l)
  {
    if (l >= BufferSizes::MaxPath)
    {
      return false;
    }
    char ch = lpszArg[l];
    if (ch == '<'
      || ch == '>'
      || ch == '"'
      || ch == '|'
      || ch == '*'
      || ch == '?')
    {
      return false;
    }
  }
  return true;
}

template<typename CharType> int InitializeBuffer_(CharType * pBuffer, FileType inputFileType, bool isTeXProgram)
{
  int fileNameArgIdx = -1;
  PathName fileName;
  bool mangleFileName = isTeXProgram;

  shared_ptr<Session> session = Session::Get();

  if (mangleFileName)
  {
    /* test command-line for one of:
       (a) tex FILENAME
       (b) tex &FORMAT FILENAME
       (c) initex FILENAME \dump
       (d) initex &FORMAT FILENAME \dump
    */
    PathName path;
    if (c4pargc == 2 && IsFileNameArgument(c4pargv[1]) && session->FindFile(c4pargv[1], inputFileType, path))
    {
      fileNameArgIdx = 1;
    }
    else if (c4pargc == 3 && c4pargv[1][0] == '&' && IsFileNameArgument(c4pargv[2]) && session->FindFile(c4pargv[2], inputFileType, path))
    {
      fileNameArgIdx = 2;
    }
    else if (c4pargc == 3 && strcmp(c4pargv[2], "\\dump") == 0 && IsFileNameArgument(c4pargv[1]) && session->FindFile(c4pargv[1], inputFileType, path))
    {
      fileNameArgIdx = 1;
    }
    else if (c4pargc == 4 && c4pargv[1][0] == '&' && strcmp(c4pargv[3], "\\dump") == 0 && IsFileNameArgument(c4pargv[2]) && session->FindFile(c4pargv[2], inputFileType, path))
    {
      fileNameArgIdx = 2;
    }
    if (fileNameArgIdx >= 0)
    {
      if (!IsNameManglingEnabled)
      {
#if defined(MIKTEX_WINDOWS)
	fileName = Q_(path.ToLongPathName().ToUnix());
#else
	fileName = Q_(path);
#endif
      }
      else
      {
	fileName = WebAppInputLine::MangleNameOfFile(c4pargv[fileNameArgIdx]);
      }
    }
  }

  // first = 1;
  unsigned last = 1;
  for (int idx = 1; idx < c4pargc; ++idx)
  {
    if (idx > 1)
    {
      pBuffer[last++] = ' ';
    }
    const char * lpszOptArg;
    if (idx == fileNameArgIdx)
    {
      lpszOptArg = fileName.Get();
    }
    else
    {
      lpszOptArg = c4pargv[idx];
    }
    if (sizeof(CharType) == sizeof(char))
    {
      size_t len = StrLen(lpszOptArg);
      for (size_t j = 0; j < len; ++j)
      {
	pBuffer[last++] = lpszOptArg[j];
      }
    }
    else
    {
      wstring optarg = StringUtil::UTF8ToWideChar(lpszOptArg);
      size_t len = optarg.length();
      for (size_t j = 0; j < len; ++j)
      {
	pBuffer[last++] = optarg[j];
      }

    }
  }

  // clear the command-line
  MakeCommandLine(vector<string>());

  return last;
}

unsigned long TeXMFApp::InitializeBuffer(unsigned char * pBuffer)
{
  MIKTEX_ASSERT(pBuffer != nullptr);
  return InitializeBuffer_<unsigned char>(pBuffer, GetInputFileType(), isTeXProgram);
}

unsigned long TeXMFApp::InitializeBuffer(unsigned short * pBuffer)
{
  MIKTEX_ASSERT(pBuffer != nullptr);
  return InitializeBuffer_<unsigned short>(pBuffer, GetInputFileType(), isTeXProgram);
}

unsigned long TeXMFApp::InitializeBuffer(C4P_signed32 * pBuffer)
{
  MIKTEX_ASSERT(pBuffer != nullptr);
  return InitializeBuffer_<C4P_signed32>(pBuffer, GetInputFileType(), isTeXProgram);
}

void TeXMFApp::InvokeEditor(const PathName & editFileName, int editLineNumber, const PathName & transcriptFileName) const
{
  string defaultEditor;

  PathName texworks;
  if (session->FindFile(MIKTEX_TEXWORKS_EXE, FileType::EXE, texworks))
  {
    defaultEditor = Q_(texworks);
    defaultEditor += " -p=%l \"%f\"";
  }
  else
  {
    defaultEditor = "notepad \"%f\"";
  }

  // read information from yap.ini
  PathName yapIni = session->GetSpecialPath(SpecialPath::UserConfigRoot);
  yapIni /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  yapIni /= MIKTEX_YAP_INI_FILENAME;
  if (File::Exists(yapIni))
  {
    unique_ptr<Cfg> yapConfig(Cfg::Create());
    yapConfig->Read(yapIni);
    string yapEditor;
    if (yapConfig->TryGetValue("Settings", "Editor", yapEditor))
    {
      defaultEditor = yapEditor;
    }
  }

  string templ = session->GetConfigValue(nullptr, MIKTEX_REGVAL_EDITOR, defaultEditor.c_str());

  const char * lpszCommandLineTemplate = templ.c_str();

  string fileName;

  bool quoted = false;

  for (; *lpszCommandLineTemplate != ' ' || (*lpszCommandLineTemplate != 0 && quoted); ++lpszCommandLineTemplate)
  {
    if (*lpszCommandLineTemplate == '"')
    {
      quoted = !quoted;
    }
    else
    {
      fileName += *lpszCommandLineTemplate;
    }
  }

  for (; *lpszCommandLineTemplate == ' '; ++lpszCommandLineTemplate)
  {

  }

  string arguments;

  while (*lpszCommandLineTemplate != 0)
  {
    if (lpszCommandLineTemplate[0] == '%' && lpszCommandLineTemplate[1] != 0)
    {
      switch (lpszCommandLineTemplate[1])
      {
      default:
	break;
      case '%':
	arguments += '%';
	break;
      case 'f':
      {
	PathName unmangled = UnmangleNameOfFile(editFileName.Get());
	PathName path;
	if (session->FindFile(unmangled.Get(), GetInputFileType(), path))
	{
	  arguments += path.Get();
	}
	else
	{
	  arguments += unmangled.Get();
	}
	break;
      }
      case 'h':
	// TODO
	break;
      case 't':
	arguments += transcriptFileName.Get();
	break;
      case 'l':
	arguments += std::to_string(editLineNumber);
	break;
      case 'm':
	// TODO
	break;
      }
      lpszCommandLineTemplate += 2;
    }
    else
    {
      arguments += *lpszCommandLineTemplate;
      ++lpszCommandLineTemplate;
    }
  }

  Process::Start(fileName, arguments.c_str());
}

void TeXMFApp::TouchJobOutputFile(FILE * pfile) const
{
  MIKTEX_ASSERT(pfile != nullptr);
  if (setJobTime && session->IsOutputFile(pfile))
  {
    time_t time = GetStartUpTime();
    File::SetTimes(pfile, time, time, time);
  }
}
