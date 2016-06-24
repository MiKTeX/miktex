/* makefmt.cpp: make TeX format files

   Copyright (C) 1998-2016 Christian Schenk

   This file is part of the MiKTeX Maker Library.

   The MiKTeX Maker Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Maker Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Maker Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "internal.h"

enum {
  OPT_AAA = 1,
  OPT_DESTNAME,
  OPT_ENGINE,
  OPT_ENGINE_OPTION,
  OPT_JOB_TIME,
  OPT_NO_DUMP,
  OPT_PRELOAD
};

enum class Engine
{
  LuaTeX,
  TeX,
  pdfTeX,
  XeTeX,
  Omega,
};

class PdfConfigValues :
  public HasNamedValues
{
public:
  bool TryGetValue(const string & valueName, string & value) override
  {
    unordered_map<string, string>::const_iterator it = values.find(valueName);
    if (it == values.end())
    {
      return false;
    }
    value = it->second;
    return true;
  }
public:
  string GetValue(const string & valueName) override
  {
    string value;
    if (!TryGetValue(valueName, value))
    {
      MIKTEX_FATAL_ERROR_2(T_("The pdfTeX configuration file pdftexconfig.tex is incomplete."), "valueName", valueName);
    }
    return value;
  }
public:
  string & operator[] (const string & valueName)
  {
    return values[valueName];
  }
private:
  unordered_map<string, string> values;
};

class MakeFmt : public MakeUtility
{
public:
  virtual void Run(int argc, const char ** argv);

private:
  virtual void Usage();

private:
  virtual void CreateDestinationDirectory();

private:
  BEGIN_OPTION_MAP(MakeFmt)
    OPTION_ENTRY(OPT_ENGINE, SetEngine(lpszOptArg))
    OPTION_ENTRY(OPT_ENGINE_OPTION, AppendEngineOption(lpszOptArg))
    OPTION_ENTRY_SET(OPT_DESTNAME, destinationName)
    OPTION_ENTRY_SET(OPT_JOB_TIME, jobTime)
    OPTION_ENTRY_SET(OPT_PRELOAD, preloadedFormat)
    OPTION_ENTRY_TRUE(OPT_NO_DUMP, noDumpPrimitive)
  END_OPTION_MAP();

private:
  void SetEngine(const char * lpszEngine)
  {
    if (Utils::EqualsIgnoreCase(lpszEngine, "luatex"))
    {
      engine = Engine::LuaTeX;
    }
    else if (Utils::EqualsIgnoreCase(lpszEngine, "tex"))
    {
      engine = Engine::TeX;
    }
    else if (Utils::EqualsIgnoreCase(lpszEngine, "pdftex"))
    {
      engine = Engine::pdfTeX;
    }
    else if (Utils::EqualsIgnoreCase(lpszEngine, "xetex"))
    {
      engine = Engine::XeTeX;
    }
    else if (Utils::EqualsIgnoreCase(lpszEngine, "omega"))
    {
      engine = Engine::Omega;
    }
    else
    {
      FatalError(T_("Unknown engine: %s"), lpszEngine);
    }
  }

private:
  void AppendEngineOption(const char * lpszOption)
  {
    engineOptions.Append(lpszOption);
  }

private:
  const char * GetEngineName()
  {
    switch (engine)
    {
    case Engine::LuaTeX:
      return "luatex";
    case Engine::TeX:
      return "tex";
    case Engine::pdfTeX:
      return "pdftex";
    case Engine::XeTeX:
      return "xetex";
    case Engine::Omega:
      return "omega";
    }
    MIKTEX_UNEXPECTED();
  }

private:
  const char * GetEngineExeName()
  {
    switch (engine)
    {
    case Engine::LuaTeX:
      return MIKTEX_LUATEX_EXE;
    case Engine::TeX:
      return MIKTEX_TEX_EXE;
    case Engine::pdfTeX:
      return MIKTEX_PDFTEX_EXE;
    case Engine::XeTeX:
      return MIKTEX_XETEX_EXE;
    case Engine::Omega:
      return MIKTEX_OMEGA_EXE;
    }
    MIKTEX_UNEXPECTED();
  }

private:
  void FindInputFile(const PathName & inputName, PathName & inputFile);

private:
  bool IsPdf() const
  {
    return engine == Engine::LuaTeX || engine == Engine::pdfTeX;
  }

private:
  bool IsExtended() const
  {
    return engine == Engine::LuaTeX || engine == Engine::pdfTeX || engine == Engine::XeTeX;
  }

private:
  PdfConfigValues ParsePdfConfigFiles() const;

private:
  void ParsePdfConfigFile(const PathName & cfgFile, PdfConfigValues & values) const;

private:
  void InstallPdftexConfigTeX() const;

private:
  Engine engine = Engine::TeX;

private:
  PathName destinationName;

private:
  string preloadedFormat;

private:
  string jobTime;

private:
  bool noDumpPrimitive = false;

private:
  Argv engineOptions;
};

void MakeFmt::Usage()
{
  cout
    << T_("Usage:") << " " << Utils::GetExeName() << " " << T_("[OPTION]... NAME") << endl
    << endl
    << T_("This program makes a format file.") << endl
    << endl
    << T_("NAME is the name of the format, such as 'tex'.") << endl
    << endl
    << T_("Options:") << endl
    << "--debug, -d " << T_("Print debugging information.") << endl
    << "--dest-name NAME " << T_("Destination file name.") << endl
    << "--disable-installer " << T_("Disable the package installer.") << endl
    << "--enable-installer " << T_("Enable the package installer.") << endl
    << "--engine=ENGINE " << T_("Set the engine.") << endl
    << "--engine-option=OPTION " << T_("Add an engine option.") << endl
    << "--help, -h " << T_("Print this help screen and exit.") << endl
    << "--job-time=FILE " << T_("Job time is file's modification time.") << endl
    << "--no-dump " << T_("Don't issue the \\dump command.") << endl
    << "--preload FORMAT " << T_("Format to be preloaded.") << endl
    << "--print-only, -n " << T_("Print what commands would be executed.") << endl
    << "--verbose, -v " << T_("Print information on what is being done.") << endl
    << "--version, -V " << T_("Print the version number and exit.") << endl;
}

namespace {
  const struct option aLongOptions[] =
  {
    COMMON_OPTIONS,
    "dest-name",        required_argument,      nullptr,      OPT_DESTNAME,
    "engine",           required_argument,      nullptr,      OPT_ENGINE,
    "engine-option",    required_argument,      nullptr,      OPT_ENGINE_OPTION,
    "job-time",         required_argument,      nullptr,      OPT_JOB_TIME,
    "no-dump",          no_argument,            nullptr,      OPT_NO_DUMP,
    "preload",          required_argument,      nullptr,      OPT_PRELOAD,
    nullptr,            no_argument,            nullptr,      0,
  };
}

#define DEFAULT_DESTDIR                         \
  MIKTEX_PATH_TEXMF_PLACEHOLDER                 \
  MIKTEX_PATH_DIRECTORY_DELIMITER_STRING        \
  MIKTEX_PATH_FMT_DIR

void MakeFmt::CreateDestinationDirectory()
{
  PathName defDestDir;
  defDestDir = MIKTEX_PATH_TEXMF_PLACEHOLDER;
  defDestDir /= MIKTEX_PATH_FMT_DIR;
  defDestDir /= GetEngineName();
  destinationDirectory = CreateDirectoryFromTemplate(session->GetConfigValue(MIKTEX_REGKEY_MAKEFMT, MIKTEX_REGVAL_DESTDIR, defDestDir.Get()));
}

void MakeFmt::FindInputFile(const PathName & inputName, PathName & inputFile)
{
  if (!session->FindFile(inputName.Get(), FileType::TEX, inputFile))
  {
    FatalError(T_("The input file could not be found."));
  }

  // remove the directory specification from the path; alternatevly,
  // we could convert backslashes into forward slashes
  inputFile.RemoveDirectorySpec();
}

PdfConfigValues MakeFmt::ParsePdfConfigFiles() const
{
  vector<PathName> cfgFiles;
  if (!session->FindFile(MIKTEX_PATH_PDFTEX_CFG, MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, cfgFiles))
  {
    MIKTEX_FATAL_ERROR(T_("The pdfTeX configuration file (pdftex.cfg) could not be found."));
  }
  PdfConfigValues values;
  for (vector<PathName>::const_reverse_iterator it = cfgFiles.rbegin(); it != cfgFiles.rend(); ++it)
  {
    ParsePdfConfigFile(*it, values);
  }
  return values;
}

void MakeFmt::ParsePdfConfigFile(const PathName & cfgFile, PdfConfigValues & values) const
{
  AutoFILE pFile(File::Open(cfgFile, FileMode::Open, FileAccess::Read));
  string line;
  while (Utils::ReadUntilDelim(line, '\n', pFile.Get()))
  {
    Tokenizer tok(line.c_str(), " \t\r\n");
    if (tok.GetCurrent() == nullptr)
    {
      continue;
    }
    string valueName = tok.GetCurrent();
    tok.SetDelim("\r\n");
    ++tok;
    if (tok.GetCurrent() == nullptr)
    {
      continue;
    }
    string value = tok.GetCurrent();
    values[valueName] = value;
  }
}

void MakeFmt::InstallPdftexConfigTeX() const
{
  PdfConfigValues pdfConfigValues = ParsePdfConfigFiles();
  session->ConfigureFile(MIKTEX_PATH_PDFTEXCONFIG_TEX, &pdfConfigValues);
}

void MakeFmt::Run(int argc, const char ** argv)
{
  // get options and file name
  int optionIndex = 0;
  GetOptions(argc, argv, aLongOptions, optionIndex);
  if (argc - optionIndex != 1)
  {
    FatalError(T_("Invalid command-line."));
  }
  name = argv[optionIndex];

  if (destinationName.Empty())
  {
    destinationName = name;
    destinationName.SetExtension(nullptr);
  }

  // pretend to be the engine / format
  if (!(destinationName == GetEngineName()))
  {
    session->PushAppName(GetEngineName());
  }
  session->PushAppName(destinationName.Get());

  // find the TeX input file
  PathName inputFile;
  FindInputFile(name, inputFile);

  // create destination directory
  CreateDestinationDirectory();

  // make the format file name
  PathName formatFile(destinationName);
  formatFile.SetExtension(MIKTEX_FORMAT_FILE_SUFFIX);

  // make fully qualified destination file name
  PathName pathDest(destinationDirectory,
    destinationName,
    MIKTEX_FORMAT_FILE_SUFFIX);

  Verbose(T_("Creating the %s format file..."), Q_(destinationName));

  // create a temporary working directory
  unique_ptr<TemporaryDirectory> wrkDir = TemporaryDirectory::Create();

  // make command line
  CommandLineBuilder arguments;
  arguments.AppendOption("--initialize");
  arguments.AppendOption("--interaction=", "nonstopmode");
  arguments.AppendOption("--halt-on-error");
  if (destinationName != GetEngineName())
  {
    arguments.AppendOption("--alias=", destinationName);
  }
  arguments.AppendOption("--job-name=", destinationName);
  if (!jobTime.empty())
  {
    arguments.AppendOption("--job-time=", jobTime);
  }
  arguments.AppendArguments(engineOptions);
  if (!preloadedFormat.empty())
  {
    arguments.AppendArgument(string("&") + preloadedFormat);
  }
  if (engine != Engine::LuaTeX && IsExtended() && preloadedFormat.empty())
  {
    arguments.AppendOption("--enable-etex");
  }
  switch (GetEnableInstaller())
  {
  case TriState::False:
    arguments.AppendOption("--disable-installer");
    break;
  case TriState::True:
    arguments.AppendOption("--enable-installer");
    break;
  default:
    break;
  }
#if defined(MIKTEX_STANDALONE)
  if (IsPdf())
  {
    InstallPdftexConfigTeX();
  }
#endif
  arguments.AppendArgument(inputFile);
  if (!noDumpPrimitive)
  {
    arguments.AppendArgument("\\dump");
  }

  // start the engine
  if (!RunProcess(GetEngineExeName(), arguments.ToString(), wrkDir->GetPathName()))
  {
    FatalError(T_("%s failed on %s."), GetEngineExeName(), Q_(name));
  }

  // install format file
  Install(wrkDir->GetPathName() / formatFile, pathDest);
}

MKTEXAPI(makefmt) (int argc, const char ** argv)
{
  try
  {
    MakeFmt app;
    app.Init(Session::InitInfo(argv[0]));
    app.Run(argc, argv);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Application::Sorry("makefmt", e);
    return 1;
  }
  catch (const exception & e)
  {
    Application::Sorry("makefmt", e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
