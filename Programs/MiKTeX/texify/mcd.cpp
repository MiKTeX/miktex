/* mcd.cpp: MiKTeX compiler driver

   Copyright (C) 1998-2020 Christian Schenk

   Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2001,
   2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This file is part of the MiKTeX Compiler Driver.

   The MiKTeX Compiler Driver is free software; you can redistribute
   it and/or modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2, or (at your option) any later version.

   The MiKTeX Compiler Driver is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with The MiKTeX Compiler Driver; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

/* This program imitates the shell script texi2dvi, which is a part of
   the GNU Texinfo package.  Texi2dvi was originally written by
   Noah Friedman.

   Most of the source comments are stolen from the texi2dvi script. */

#if defined(MIKTEX_WINDOWS)
#  include <windows.h>
#endif

#include <cctype>
#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "mcd-version.h"

#include <miktex/App/Application>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/DirectoryLister>
#include <miktex/Core/Environment>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileStream>
#include <miktex/Core/FileType>
#include <miktex/Core/MemoryMappedFile>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/StreamReader>
#include <miktex/Core/StreamWriter>
#include <miktex/Core/TemporaryDirectory>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/StringUtil>
#include <miktex/Util/inliners.h>
#include <miktex/Wrappers/PoptWrapper>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <regex.h>

using namespace std;
using namespace std::string_literals;

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define ALWAYS_UNUSED(x)

#define SUPPORT_OPT_SRC_SPECIALS

/* Patch (#464378) by Thomas Schimming (http://sf.net/users/schimmin/):

   The BibTeX handler is extended to work in a chapterbib compliant
   mode, that is, bibtex is run on all 1st level AUX files referenced
   in the main AUX file.  After the special treatment (if use of the
   chapterbib package is detected), the regular bibtex treatment
   occurs (as before).

   The code uses some heuristics to determine when it's necessary to
   run this extension.  However, this can still be improved as for
   example changes in bibstyle do not correctly trigger a re-bibtex
   yet.

   Ultimately, the behavior should probably be included into bibtex,
   however in this way, an "orthodox" latex distribution is
   maintained... */

#define SF464378__CHAPTERBIB

#define WITH_TEXINFO

#define PROGRAM_NAME "texify"

#if !defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX Compiler Driver")
#endif

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

vector<string> DEFAULT_TRACE_OPTIONS = {
  TraceStream::MakeOption("", "", TraceLevel::Info),
  TraceStream::MakeOption(PROGRAM_NAME, "", TraceLevel::Trace),
};

class ProcessOutputTrash :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* chunk, size_t n) override
  {
    ALWAYS_UNUSED(chunk);
    ALWAYS_UNUSED(n);
    return true;
  }
};

class ProcessOutputSaver :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void* chunk, size_t n) override
  {
    output.append(reinterpret_cast<const char*>(chunk), n);
    return true;
  }
public:
  const string& GetOutput() const
  {
    return output;
  }
private:
  string output;
};

void CopyFiles(const vector<string>& fileNames, const PathName& destDir)
{
  for (const string& fileName : fileNames)
  {
    File::Copy(PathName(fileName), PathName(destDir, PathName(fileName)));
  }
}

vector<char> ReadFile(const PathName& fileName)
{
  size_t fileSize = File::GetSize(fileName);
  vector<char> vec(fileSize + 1);
  FileStream stream(File::Open(fileName, FileMode::Open, FileAccess::Read));
  fileSize = stream.Read(&vec[0], fileSize);
  stream.Close();
  std::replace(vec.begin(), vec.end(), '\0', '\t');
  vec[fileSize] = 0;
  return vec;
}

bool Contains(const PathName& fileName, regex_t* preg)
{
  vector<char> file = ReadFile(fileName);
  return regexec(preg, &file[0], 0, nullptr, 0) == 0;
}

bool Contains(const PathName& fileName, const char* text)
{
  unique_ptr<MemoryMappedFile> mmapFile(MemoryMappedFile::Create());
  const char* ptr = reinterpret_cast<char*>(mmapFile->Open(fileName, false));
  size_t size = mmapFile->GetSize();
  const char* p = text;
  for (size_t i = 0; *p != 0 && i < size; ++i, ++ptr)
  {
    if (*ptr == *p)
    {
      ++p;
    }
    else
    {
      p = text;
    }
  }
  mmapFile->Close();
  return *p == 0;
}

string FlattenStringVector(const vector<string>& vec, char sep)
{
  string str = "";
  for (const string& s : vec)
  {
    if (!str.empty())
    {
      str += sep;
    }
    bool mustQuote = (s.find(sep) != string::npos);
    if (mustQuote)
    {
      str += '"';
    }
    str += s;
    if (mustQuote)
    {
      str += '"';
    }
  }
  return str;
}

bool IsPrefixOf(const char* prefix, const string& str)
{
  return str.compare(0, StrLen(prefix), prefix) == 0;
}

enum class MacroLanguage
{
  None, LaTeX,
#if defined(WITH_TEXINFO)
  Texinfo
#endif
};

enum class Engine
{
  NotSet, TeX, pdfTeX, XeTeX, LuaTeX, LuaHBTeX,
};

enum class OutputType {
  None, DVI, PDF, PS
};

enum class SyncTeXOption
{
  Uncompressed = -1,
  Disabled = 0,
  Compressed = 1
};

class Options
{
public:
  Options();

public:
  virtual ~Options();

public:
  PathName startDirectory;

public:
  regex_t regex_bibdata;

public:
  regex_t regex_bibstyle;

public:
  regex_t regex_citation_undefined;

public:
  regex_t regex_no_file_bbl;

#if defined(WITH_TEXINFO)
public:
  regex_t regex_texinfo_version;
#endif

#ifdef SF464378__CHAPTERBIB
public:
  regex_t regex_chapterbib;
public:
  regex_t regex_input_aux;
#endif /* SF464378__CHAPTERBIB */

public:
  bool batch = false;

public:
  bool clean = false;

public:
  bool expand = false;

public:
  bool quiet = false;

public:
  bool verbose = false;

public:
  OutputType outputType = OutputType::DVI;

public:
  bool runViewer = false;

#if defined(SUPPORT_OPT_SRC_SPECIALS)
public:
  bool sourceSpecials = false;
#endif

public:
  SyncTeXOption synctex = SyncTeXOption::Disabled;

public:
  int maxIterations = 5;

public:
  vector<string> includeDirectories;

public:
  string jobName;

#if defined(WITH_TEXINFO)
public:
  vector<string> texinfoCommands;
#endif

public:
  MacroLanguage macroLanguage = MacroLanguage::None;

public:
  string sourceSpecialsWhere;

public:
  string bibtexProgram;

public:
  string latexProgram;

public:
  string pdflatexProgram;

public:
  string xelatexProgram;

public:
  string lualatexProgram;

public:
  string luahblatexProgram;

public:
  string makeindexProgram;

public:
  string makeinfoProgram;

public:
  string texProgram;

public:
  string pdftexProgram;

public:
  string xetexProgram;

public:
  string luatexProgram;

public:
  string luahbtexProgram;

public:
  string texindexProgram;

public:
  vector<string> makeindexOptions;

public:
  vector<string> texOptions;

public:
  vector<string> viewerOptions;

public:
  string traceStreams;

private:
  string SetProgramName(const string& envName, const string& defaultProgram)
  {
    string programName;
    if (!Utils::GetEnvironmentString(envName, programName))
    {
      programName = defaultProgram;
    }
    return programName;
  }

public:
  Engine engine = Engine::NotSet;

public:
  bool SetEngine(const string& engineName)
  {
    if (Utils::EqualsIgnoreCase(engineName, "tex"))
    {
      engine = Engine::TeX;
    }
    else if (Utils::EqualsIgnoreCase(engineName, "pdftex"))
    {
      engine = Engine::pdfTeX;
    }
    else if (Utils::EqualsIgnoreCase(engineName, "xetex"))
    {
      engine = Engine::XeTeX;
    }
    else if (Utils::EqualsIgnoreCase(engineName, "luatex"))
    {
      engine = Engine::LuaTeX;
    }
    else if (Utils::EqualsIgnoreCase(engineName, "luahbtex"))
    {
      engine = Engine::LuaHBTeX;
    }
    else
    {
      return false;
    }
    return true;
  }
};

Options::Options()
{
  if (regcomp(&regex_bibdata, "^\\\\bibdata", REG_NOSUB | REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }

  if (regcomp(&regex_bibstyle, "^\\\\bibstyle", REG_NOSUB | REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }

  if (regcomp(&regex_citation_undefined, "Warning:.*Citation.*undefined", REG_NOSUB | REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }

  if (regcomp(&regex_no_file_bbl, "No file .*\\.bbl\\.", REG_NOSUB | REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }

#if defined(WITH_TEXINFO)
  if (regcomp(&regex_texinfo_version, "\\[(.*)version (....)-(..)-(..)", REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }
#endif

#if defined(SF464378__CHAPTERBIB)
  if (regcomp(&regex_chapterbib, "^Package: chapterbib", REG_NOSUB | REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }

  if (regcomp(&regex_input_aux, "^\\\\@input\\{.*\\.aux\\}", REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }
#endif /* SF464378__CHAPTERBIB */

  startDirectory.SetToCurrentDirectory();
  startDirectory.ConvertToUnix();

  bibtexProgram = SetProgramName("BIBTEX", MIKTEX_BIBTEX_EXE);
  latexProgram = SetProgramName("LATEX", "latex");
  makeindexProgram = SetProgramName("MAKEINDEX", MIKTEX_MAKEINDEX_EXE);
  makeinfoProgram = SetProgramName("MAKEINFO", "makeinfo");
  pdflatexProgram = SetProgramName("PDFLATEX", "pdflatex");
  pdftexProgram = SetProgramName("PDFTEX", MIKTEX_PDFTEX_EXE);
  texProgram = SetProgramName("TEX", MIKTEX_TEX_EXE);
  texindexProgram = SetProgramName("TEXINDEX", "texindex");
  xelatexProgram = SetProgramName("XELATEX", "xelatex");
  lualatexProgram = SetProgramName("LUALATEX", "lualatex");
  luahblatexProgram = SetProgramName("LUAHBLATEX", "luahblatex");
  xetexProgram = SetProgramName("XETEX", MIKTEX_XETEX_EXE);
  luatexProgram = SetProgramName("LUATEX", MIKTEX_LUATEX_EXE);
  luahbtexProgram = SetProgramName("LUAHBTEX", MIKTEX_LUAHBTEX_EXE);
}

Options::~Options()
{
  regfree(&regex_bibdata);
  regfree(&regex_bibstyle);
  regfree(&regex_citation_undefined);
  regfree(&regex_no_file_bbl);
#if defined(WITH_TEXINFO)
  regfree(&regex_texinfo_version);
#endif
#if defined(SF464378__CHAPTERBIB)
  regfree(&regex_chapterbib);
  regfree(&regex_input_aux);
#endif
}

class McdApp :
  public Application
{
public:
  McdApp() :
    traceStream(TraceStream::Open(PROGRAM_NAME))
  {
    forbiddenTexOptions.push_back("aux-directory");
    forbiddenTexOptions.push_back("job-name");
    forbiddenTexOptions.push_back("jobname");
    forbiddenTexOptions.push_back("output-directory");
  };

public:
  void Run(int argc, const char** argv);

public:
  void MyTrace(const string& s);

public:
  void Verbose(const string& s);

private:
  void Version();

private:
  unique_ptr<TraceStream> traceStream;

public:
  Options options;

private:
  vector<string> forbiddenTexOptions;
};

void McdApp::Verbose(const string& s)
{
  traceStream->WriteLine(PROGRAM_NAME, s);
  if (options.verbose)
  {
    cout << s << "\n";
  }
}

void McdApp::MyTrace(const string& s)
{
  traceStream->WriteLine(PROGRAM_NAME, s);
}

void McdApp::Version()
{
  cout
    << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << "\n"
    << "Copyright (C) 1998-2020 Christian Schenk" << "\n"
    << "Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2001," << "\n"
    << "2002, 2003, 2004, 2005 Free Software Foundation, Inc." << "\n"
    << "This is free software; see the source for copying conditions.  There is NO" << "\n"
    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << "\n";
}

class Driver
{
public:
  virtual ~Driver();

public:
  void Initialize(McdApp* app, Options* options, const char* fileName);

public:
  void Run();

private:
  void FatalUtilityError(const string& name)
  {
    app->FatalError(fmt::format("A required utility could not be found. Utility name: {}.", name));
  }

private:
  MacroLanguage GuessMacroLanguage(const PathName& fileName);

#if defined(WITH_TEXINFO)
private:
  void TexinfoPreprocess(const PathName& pathFrom, const PathName& pathTo);
#endif

#if defined(WITH_TEXINFO)
private:
  void TexinfoUncomment(const PathName& pathFrom, const PathName& pathTo);
#endif

private:
  void SetIncludeDirectories();

private:
  void ExpandMacros();

private:
  void InsertCommands();

private:
  bool RunMakeinfo(const PathName& pathFrom, const PathName& pathTo);

private:
  void RunBibTeX();

private:
  PathName GetTeXEnginePath(string& exeName);

private:
  void RunTeX();

private:
  void RunIndexGenerator(const vector<string>& idxFiles);

private:
  void RunViewer();

private:
  bool Ready();

#if defined(WITH_TEXINFO)
private:
  bool Check_texinfo_tex();
#endif

private:
  void InstallOutputFile();

private:
  void GetAuxFiles(vector<string>& auxFiles, vector<string>* idxFiles = nullptr);

private:
  void GetAuxFiles(const PathName& baseName, const char* extension, vector<string>& auxFiles);

private:
  void InstallProgram(const char* program);

  // the macro language
private:
  MacroLanguage macroLanguage = MacroLanguage::None;

  // file name given on the command-line
private:
  PathName givenFileName;

  // fully qualified path to the original input file
private:
  PathName originalInputFile;

  // super-temporary directory
private:
  unique_ptr<TemporaryDirectory> tempDirectory;

  // directory which contains the original input file
private:
  PathName originalInputDirectory;

  // name of the input file
private:
  PathName inputName;

  // job name (set output name)
private:
  PathName jobName;

  // the working directory; it will be a temporary directory if we are
  // in clean mode; contains the input file, if --expand was specified
  // on the command-line
private:
  PathName workingDirectory;

#if defined(WITH_TEXINFO)
  // the extra directory: contains the input file, if --texinfo was
  // specified on the command-line
private:
  PathName extraDirectory;
#endif

  // the aux directory: contains backed up auxiliary files
private:
  PathName auxDirectory;

  // fully qualified path to the input file
private:
  PathName pathInputFile;

  // contains auxiliary files from the last run
private:
  vector<string> previousAuxFiles;

private:
  McdApp* app = nullptr;

private:
  Options* options = nullptr;

protected:
  shared_ptr<Session> session = Session::Get();
};

Driver::~Driver()
{
  try
  {
    if (options != nullptr && options->clean)
    {
      Directory::SetCurrent(options->startDirectory);
    }
    if (tempDirectory != nullptr)
    {
      tempDirectory->Delete();
    }
  }
  catch (const exception &)
  {
  }
}

void Driver::Initialize(McdApp* app, Options* options, const char* fileName)
{
  this->app = app;
  this->options = options;

  app->MyTrace(T_("initializing driver..."));

  givenFileName = fileName;

  // Get the name of the current directory.  We want the full path
  // because in clean mode we are in tmp, in which case a relative
  // path has no meaning.
  originalInputFile = givenFileName;
  originalInputFile.MakeFullyQualified();
  originalInputFile.ConvertToUnix();

  app->MyTrace(fmt::format(T_("input file: {}"), Q_(originalInputFile)));

  if (options->jobName.empty())
  {
    jobName = givenFileName.GetFileNameWithoutExtension();
  }
  else
  {
    jobName = options->jobName;
  }

  inputName = givenFileName;
  inputName.RemoveDirectorySpec();

  // create a super-temp directory
  tempDirectory = TemporaryDirectory::Create();

  // create scratch directory
  workingDirectory = tempDirectory->GetPathName() / PathName("_src");
  Directory::Create(workingDirectory);
  workingDirectory.ConvertToUnix();
  app->MyTrace(fmt::format(T_("working directory: {}"), Q_(workingDirectory)));

#if defined(WITH_TEXINFO)
  // create extra directory
  extraDirectory = tempDirectory->GetPathName() / PathName("_xtr");
  Directory::Create(extraDirectory);
  extraDirectory.ConvertToUnix();
  app->MyTrace(fmt::format(T_("extra directory: {}"), Q_(extraDirectory)));
#endif

  // create aux directory
  auxDirectory = tempDirectory->GetPathName() / PathName("_aux");
  Directory::Create(auxDirectory);
  auxDirectory.ConvertToUnix();
  app->MyTrace(fmt::format(T_("aux directory: {}"), Q_(auxDirectory)));

  // If the user explicitly specified the language, use that.
  // Otherwise, if the first line is \input texinfo, assume it's
  // texinfo.  Otherwise, guess from the file extension.
  macroLanguage = options->macroLanguage;
  if (macroLanguage == MacroLanguage::None)
  {
    macroLanguage = GuessMacroLanguage(givenFileName);
  }

  // make fully qualified path to the given input file
  if (givenFileName.IsAbsolute())
  {
    pathInputFile = givenFileName;
  }
  else
  {
    pathInputFile = options->startDirectory / givenFileName;
  }

  originalInputDirectory = pathInputFile;
  originalInputDirectory.RemoveFileSpec();
  originalInputDirectory.ConvertToUnix();
}

MacroLanguage Driver::GuessMacroLanguage(const PathName& fileName)
{
  StreamReader reader(fileName);
  string firstLine;
  if (!reader.ReadLine(firstLine))
  {
    return MacroLanguage::None;
  }
  reader.Close();
#if defined(WITH_TEXINFO)
  if (firstLine.find("input texinfo") != string::npos)
  {
    return MacroLanguage::Texinfo;
  }
#endif
  if (fileName.HasExtension(".dtx")
    || fileName.HasExtension(".tex")
    || fileName.HasExtension(".latex")
    || fileName.HasExtension(".ltx"))
  {
    return MacroLanguage::LaTeX;
  }
  else
  {
#if defined(WITH_TEXINFO)
    return MacroLanguage::Texinfo;
#else
    return MacroLanguage::None;
#endif
  }
}

#if defined(WITH_TEXINFO)
/* _________________________________________________________________________

   Driver::TexinfoPreprocess

   A function that preprocesses Texinfo sources in order to keep the
   iftex sections only.  We want to remove non TeX sections, and
   comment (with `@c texi2dvi') TeX sections so that makeinfo does not
   try to parse them.  Nevertheless, while commenting TeX sections,
   don't comment @macro/@end macro so that makeinfo does propagate
   them.  Unfortunately makeinfo --iftex --no-ifhtml --no-ifinfo
   doesn't work well enough (yet) to use that, so work around with
   sed.
   _________________________________________________________________________ */

void Driver::TexinfoPreprocess(const PathName& pathFrom, const PathName& pathTo)
{
  StreamReader reader(pathFrom);
  StreamWriter writer(pathTo);
  bool at_tex = false;
  bool at_iftex = false;
  bool at_macro = false;
  bool at_html = false;
  bool at_ifhtml = false;
  bool at_ifnottex = false;
  bool at_ifinfo = false;
  bool at_menu = false;
  string line;
  while (reader.ReadLine(line))
  {
    if (IsPrefixOf("@tex", line))
    {
      at_tex = true;
    }
    else if (IsPrefixOf("@iftex", line))
    {
      at_iftex = true;
    }
    else if (IsPrefixOf("@macro", line))
    {
      at_macro = true;
    }
    else if (IsPrefixOf("@html", line))
    {
      at_html = true;
    }
    else if (IsPrefixOf("@ifnottex", line))
    {
      at_ifnottex = true;
    }
    else if (IsPrefixOf("@ifinfo", line))
    {
      at_ifinfo = true;
    }
    else if (IsPrefixOf("@menu", line))
    {
      at_menu = true;
    }

    bool commentingOut = (at_tex || (at_iftex && !at_macro));
    bool deleting = (at_html || at_ifhtml || at_ifnottex
      || (at_ifinfo
        && !at_menu
        && !IsPrefixOf("@node", line)));

    if (commentingOut)
    {
      writer.WriteLine(string("@c texi2dvi") + line);
    }
    else if (!deleting)
    {
      writer.WriteLine(line);
    }

    if (IsPrefixOf("@end tex", line))
    {
      at_tex = false;
    }
    else if (IsPrefixOf("@end iftex", line))
    {
      at_iftex = false;
    }
    else if (IsPrefixOf("@end macro", line))
    {
      at_macro = false;
    }
    else if (IsPrefixOf("@end html", line))
    {
      at_html = false;
    }
    else if (IsPrefixOf("@end ifnottex", line))
    {
      at_ifnottex = false;
    }
    else if (IsPrefixOf("@end ifinfo", line))
    {
      at_ifinfo = false;
    }
    else if (IsPrefixOf("@end menu", line))
    {
      at_menu = false;
    }
  }
}
#endif

#if defined(WITH_TEXINFO)
/* _________________________________________________________________________

   Driver::TexinfoUncomment

   Uncommenting is simple: Remove any leading `@c texi2dvi'.
   _________________________________________________________________________ */

void Driver::TexinfoUncomment(const PathName& pathFrom, const PathName& pathTo)
{
  StreamReader reader(pathFrom);
  StreamWriter writer(pathTo);
  string line;
  while (reader.ReadLine(line))
  {
    if (IsPrefixOf("@c texi2dvi", line))
    {
      writer.WriteLine(line.substr(11));
    }
    else
    {
      writer.WriteLine(line);
    }
  }
  writer.Close();
  reader.Close();
}
#endif

/* _________________________________________________________________________

   Driver::SetIncludeDirectories

   Source file might include additional sources.  Put `.' and
   directory where source file(s) reside in MIKTEX_CWD before anything
   else.  Include orig_pwd in case we are in clean mode, where we've
   cd'd to a temp directory.
   _________________________________________________________________________ */

void Driver::SetIncludeDirectories()
{
  session->AddInputDirectory(options->startDirectory, true);
  if (originalInputDirectory != options->startDirectory)
  {
    session->AddInputDirectory(originalInputDirectory, true);
  }
  for (const string& dir : options->includeDirectories)
  {
    session->AddInputDirectory(PathName(dir), true);
  }
}

/* _________________________________________________________________________

   Driver::RunMakeinfo

   Expand macro commands in the original source file using Makeinfo.
   Always use `end' footnote style, since the `separate' style
   generates different output (arguably this is a bug in -E).  Discard
   main info output, the user asked to run TeX, not makeinfo.
   _________________________________________________________________________ */

bool Driver::RunMakeinfo(const PathName& pathFrom, const PathName& pathTo)
{
  PathName pathExe;

  if (!session->FindFile(options->makeinfoProgram, FileType::EXE, pathExe))
  {
    FatalUtilityError(options->makeinfoProgram);
  }

  vector<string> args{ options->makeinfoProgram };

  args.push_back("--footnote-style="s + "end");
  args.push_back("-I");
  args.push_back(originalInputDirectory.ToString());

  for (const string& dir : options->includeDirectories)
  {
    args.push_back("-I");
    args.push_back(dir);
  }

  args.push_back("-o");
#if defined(MIKTEX_WINDOWS)
  args.push_back("nul");
#else
  args.push_back("/dev/null");
#endif

  args.push_back("--macro-expand="s + pathTo.ToString());

  args.push_back(pathFrom.ToString());

  int exitCode = 0;

  ProcessOutputTrash trash;
  Process::Run(pathExe, args, &trash, &exitCode, nullptr);

  return exitCode == 0;
}

#if defined(WITH_TEXINFO)
/* _________________________________________________________________________

   Driver::Check_texinfo_tex

   Check if texinfo.tex performs macro expansion by looking for its
   version.  The version is a date of the form YEAR-MO-DA.
   _________________________________________________________________________ */

// minimum texinfo.tex version to have macro expansion
const string txiprereq = "19990129";

bool Driver::Check_texinfo_tex()
{
  PathName pathExe;

  if (!session->FindFile(options->texProgram, FileType::EXE, pathExe))
  {
    FatalUtilityError(options->texProgram);
  }

  bool newer = false;

  unique_ptr<TemporaryDirectory> tmpdir = TemporaryDirectory::Create();

  PathName fileName = tmpdir->GetPathName() / PathName("txiversion.tex");
  StreamWriter writer(fileName);
  writer.WriteLine("\\input texinfo.tex @bye");
  writer.Close();

  int exitCode = 0;
  ProcessOutputSaver processOutput;
  if (!Process::Run(pathExe, vector<string>{ options->texProgram, fileName.ToString() }, &processOutput, &exitCode, tmpdir->GetPathName().GetData()))
  {
    MIKTEX_UNEXPECTED();
  }

  if (exitCode == 0)
  {
    regmatch_t regMatch[5];
    if (regexec(&options->regex_texinfo_version, processOutput.GetOutput().c_str(), 5, regMatch, 0) == 0)
    {
      string txiformat;
      for (int i = regMatch[1].rm_so; i < regMatch[1].rm_eo; ++i)
      {
        txiformat += processOutput.GetOutput()[i];
      }
      string version;
      version += processOutput.GetOutput()[regMatch[2].rm_so + 0];
      version += processOutput.GetOutput()[regMatch[2].rm_so + 1];
      version += processOutput.GetOutput()[regMatch[2].rm_so + 2];
      version += processOutput.GetOutput()[regMatch[2].rm_so + 3];
      version += processOutput.GetOutput()[regMatch[3].rm_so + 0];
      version += processOutput.GetOutput()[regMatch[3].rm_so + 1];
      version += processOutput.GetOutput()[regMatch[4].rm_so + 0];
      version += processOutput.GetOutput()[regMatch[4].rm_so + 1];
      app->Verbose(fmt::format(T_("texinfo.tex preloaded as {}, version is {}..."), txiformat, version));
      newer = std::stoi(txiprereq) <= std::stoi(version);
    }
  }

  return !newer;
}
#endif

void Driver::ExpandMacros()
{
#if defined(WITH_TEXINFO)
  if (macroLanguage != MacroLanguage::Texinfo)
  {
    return;
  }
#else
  return;
#endif

#if defined(WITH_TEXINFO)
  // Unless required by the user, makeinfo expansion is wanted only if
  // texinfo.tex is too old.
  bool expand = options->expand;
  if (!expand)
  {
    expand = Check_texinfo_tex();
  }
  if (!expand)
  {
    return;
  }

  PathName pathTmpFile1;
  PathName pathTmpFile2;

  pathTmpFile1.SetToTempFile();
  pathTmpFile2.SetToTempFile();

  PathName path(workingDirectory, inputName);

  app->Verbose(fmt::format(T_("macro-expanding {} to {}..."), Q_(givenFileName), Q_(path)));

  TexinfoPreprocess(originalInputFile, pathTmpFile1);

  bool expanded = RunMakeinfo(pathTmpFile1, pathTmpFile2);

  TexinfoUncomment(pathTmpFile2, path);

  File::Delete(pathTmpFile1);
  File::Delete(pathTmpFile2);

  if (expanded && File::Exists(path))
  {
    pathInputFile = path;
    pathInputFile.ConvertToUnix();
  }
  else
  {
    // If makeinfo failed (or was not even run), use the original
    // file as input.
    app->Verbose(fmt::format(T_("reverting to {}..."), Q_(givenFileName)));
  }
#endif
}

/* _________________________________________________________________________

   Driver::InsertCommands

   Used most commonly for @finalout, @smallbook, etc.
   _________________________________________________________________________ */

void Driver::InsertCommands()
{
#if defined(WITH_TEXINFO)
  if (!(options->texinfoCommands.size() > 0 && macroLanguage == MacroLanguage::Texinfo))
  {
    return;
  }
  string extra = FlattenStringVector(options->texinfoCommands, '\n');
  app->Verbose(fmt::format(T_("inserting extra commands: {}"), extra));
  PathName path(extraDirectory, inputName);
  StreamWriter writer(path);
  bool inserted = false;
  StreamReader reader(pathInputFile);
  string line;
  while (reader.ReadLine(line))
  {
    writer.WriteLine(line);
    if (!inserted && IsPrefixOf("@setfilename", line))
    {
      writer.WriteLine(extra);
      inserted = true;
    }
  }
  writer.Close();
  reader.Close();
  pathInputFile = path;
  pathInputFile.ConvertToUnix();
#endif
}

/* _________________________________________________________________________

   Driver::RunBibTeX

   Run bibtex on current file:
   - If its input (AUX) exists.
   - If AUX contains both '\bibdata' and '\bibstyle'.
   - If some citations are missing (LOG contains 'Citation') or the
     LOG complains of a missing .bbl.

   We run bibtex first, because I can see reasons for the indexes to
   change after bibtex is run, but I see no reason for the converse.

   Don't try to be too smart.  Running bibtex only if the bbl file
   exists and is older than the LaTeX file is wrong, since the
   document might include files that have changed.  Because there can
   be several AUX (if there are \include's), but a single LOG, looking
   for missing citations in LOG is easier, though we take the risk to
   match false messages.
   _________________________________________________________________________ */

void Driver::RunBibTeX()
{
  PathName pathExe;

  if (!session->FindFile(options->bibtexProgram, FileType::EXE, pathExe))
  {
    FatalUtilityError(options->bibtexProgram);
  }

  PathName logName(jobName);
  logName.AppendExtension(".log");
  PathName auxName(jobName);
  auxName.AppendExtension(".aux");

  int exitCode;

#if defined(SF464378__CHAPTERBIB)
  if ((File::Exists(auxName)
    && File::Exists(logName)
    && Contains(logName, &options->regex_chapterbib)
    && (Contains(logName, &options->regex_citation_undefined)
      || Contains(logName, &options->regex_no_file_bbl))))
  {
    app->Verbose(T_("ChapterBib detected. Preparing to run BibTeX on first-level aux files..."));

    // read the main .aux file
    vector<char> auxFile = ReadFile(auxName);

    regmatch_t regMatch[2];

    for (int offset = 0; ((regexec(&options->regex_input_aux, &auxFile[offset], 1, regMatch, offset != 0 ? REG_NOTBOL : 0) == 0) && (regMatch[0].rm_so > -1)); offset += regMatch[0].rm_eo)
    {
      // get SubAuxNameNoExt out of \@input{SubAuxNameNoExt.aux}
      string tmp(&auxFile[offset + regMatch[0].rm_so + 8], regMatch[0].rm_eo - regMatch[0].rm_so - 13);
      PathName subAuxNameNoExt(tmp);

      // append .aux extension
      PathName subAuxName(subAuxNameNoExt);
      subAuxName.AppendExtension(".aux");
      if (!(File::Exists(subAuxName)
        && Contains(subAuxName, &options->regex_bibdata)
        && Contains(subAuxName, &options->regex_bibstyle)))
      {
        continue;
      }

      PathName subDir;

      if (strchr(subAuxNameNoExt.GetData(), PathNameUtil::UnixDirectoryDelimiter) != 0)
      {
        // we have \@input{SubDir/SubAuxNameNoExt.aux}
        if (options->clean)
        {
          MIKTEX_FATAL_ERROR(T_("Sub-directories not supported when --clean is in effect."));
        }
        subDir = subAuxNameNoExt;
        subDir.RemoveFileSpec();
        subDir.MakeFullyQualified();
        subAuxNameNoExt.RemoveDirectorySpec();
      }

      vector<string> args{ options->bibtexProgram };

      args.push_back(subAuxNameNoExt.ToString());

      app->Verbose(fmt::format(T_("running {}..."), CommandLineBuilder(args).ToString()));

      exitCode = 0;

      ProcessOutputTrash trash;

      Process::Run(pathExe, args, (options->quiet ? &trash : nullptr), &exitCode, subDir.Empty() ? nullptr : subDir.GetData());
      if (exitCode != 0)
      {
        MIKTEX_FATAL_ERROR(T_("BibTeX failed for some reason."));
      }
    }
  }
#endif  // SF464378__CHAPTERBIB

  if (!(File::Exists(auxName)
    && Contains(auxName, &options->regex_bibdata)
    && Contains(auxName, &options->regex_bibstyle)
    && File::Exists(logName)
    && (Contains(logName, &options->regex_citation_undefined)
      || Contains(logName, &options->regex_no_file_bbl))))
  {
    return;
  }

  vector<string> args{ options->bibtexProgram };

  args.push_back(jobName.ToString());

  app->Verbose(fmt::format(T_("running {}..."), CommandLineBuilder(args).ToString()));

  ProcessOutputTrash trash;

  Process::Run(pathExe, args, (options->quiet ? &trash : nullptr), &exitCode, nullptr);

  if (exitCode != 0)
  {
    MIKTEX_FATAL_ERROR(T_("BibTeX failed for some reason."));
  }
}

/* _________________________________________________________________________

   Driver::RunIndexGenerator

   Run texindex (or makeindex) on current index files.  If they
   already exist, and after running TeX a first time the index files
   don't change, then there's no reason to run TeX again.  But we
   won't know that if the index files are out of date or nonexistent.
   _________________________________________________________________________ */

void Driver::RunIndexGenerator(const vector<string>& idxFiles)
{
#if defined(WITH_TEXINFO)
  const string indexGenerator = macroLanguage == MacroLanguage::Texinfo
    ? options->texindexProgram
    : options->makeindexProgram;
#else
  const string indexGenerator = options->makeindexProgram;
#endif

  PathName pathExe;

  if (!session->FindFile(indexGenerator, FileType::EXE, pathExe))
  {
    FatalUtilityError(indexGenerator);
  }

  vector<string> args{ indexGenerator };

  args.insert(args.end(), options->makeindexOptions.begin(), options->makeindexOptions.end());
  args.insert(args.end(), idxFiles.begin(), idxFiles.end());

  ProcessOutputTrash trash;

  int exitCode = 0;

  app->Verbose(fmt::format(T_("running {}..."), CommandLineBuilder(args).ToString()));

  Process::Run(pathExe, args, (options->quiet ? &trash : nullptr), &exitCode, nullptr);

  if (exitCode != 0)
  {
    MIKTEX_FATAL_ERROR(T_("MakeIndex failed for some reason."));
  }
}

void Driver::InstallProgram(const char* program)
{
  ALWAYS_UNUSED(program);
  PathName pathExe;
  if (!session->FindFile("initexmf", FileType::EXE, pathExe))
  {
    FatalUtilityError("initexmf");
  }
  ProcessOutputTrash trash;
  Process::Run(pathExe, vector<string>{ "initexmf", "--mklinks" }, (options->quiet ? &trash : nullptr));
}

PathName Driver::GetTeXEnginePath(string& exeName)
{
#if defined(WITH_TEXINFO)
  if (macroLanguage == MacroLanguage::Texinfo)
  {
    if (options->outputType == OutputType::PDF)
    {
      if (options->engine == Engine::XeTeX)
      {
        exeName = options->xetexProgram;
      }
      else if (options->engine == Engine::LuaTeX)
      {
        exeName = options->luatexProgram;
      }
      else
      {
        exeName = options->pdftexProgram;
      }
    }
    else
    {
      if (options->engine == Engine::pdfTeX)
      {
        exeName = options->pdftexProgram;
      }
      else
      {
        exeName = options->texProgram;

      }
    }
  }
  else
#endif
  {
    if (options->outputType == OutputType::PDF)
    {
      if (options->engine == Engine::XeTeX)
      {
        exeName = options->xelatexProgram;
      }
      else if (options->engine == Engine::LuaTeX)
      {
        exeName = options->lualatexProgram;
      }
      else if (options->engine == Engine::LuaHBTeX)
      {
        exeName = options->luahblatexProgram;
      }
      else
      {
        exeName = options->pdflatexProgram;
      }
    }
    else
    {
      exeName = options->latexProgram;
    }
  }
  PathName pathExe;
  if (!session->FindFile(exeName, FileType::EXE, pathExe))
  {
    InstallProgram(exeName.c_str());
    if (!session->FindFile(exeName, FileType::EXE, pathExe))
    {
      MIKTEX_FATAL_ERROR(T_("The TeX engine could not be found."));
    }
  }
  return pathExe;
}

void Driver::RunTeX()
{
  string exeName;
  PathName pathExe = GetTeXEnginePath(exeName);

  vector<string> args{ pathExe.GetFileNameWithoutExtension().ToString() };

  if (!options->jobName.empty())
  {
    args.push_back("--job-name="s + jobName.ToString());
  }

#if defined(SUPPORT_OPT_SRC_SPECIALS)
  if (options->sourceSpecials)
  {
    if (!options->sourceSpecialsWhere.empty())
    {
      args.push_back("--src-specials="s + options->sourceSpecialsWhere);
    }
    else
    {
      args.push_back("--src-specials");
    }
  }
#endif

  if (options->synctex != SyncTeXOption::Disabled)
  {
    args.push_back("--synctex="s + (options->synctex == SyncTeXOption::Compressed ? "1" : "-1"));
  }

  if (options->quiet)
  {
    args.push_back("--quiet");
  }
  if (options->batch && !options->quiet)
  {
    args.push_back("--interaction="s + "scrollmode");
  }
  args.insert(args.end(), options->texOptions.begin(), options->texOptions.end());
#if 0
  if (options->traceStreams.length() > 0)
  {
    args.push_back("--trace="s + options->traceStreams);
  }
#endif
  args.push_back(pathInputFile.ToString());

  app->Verbose(fmt::format(T_("running {}..."), CommandLineBuilder(args).ToString()));

  int exitCode = 0;
  Process::Run(pathExe, args, nullptr, &exitCode, nullptr);
  if (exitCode != 0)
  {
    PathName logName(jobName);
    logName.AppendExtension(".log");
    if (options->clean)
    {
      try
      {
        File::Copy(logName, PathName(options->startDirectory, logName));
      }
      catch (const exception &)
      {
        MIKTEX_FATAL_ERROR(T_("TeX engine failed for some reason."));
      }
    }
    MIKTEX_FATAL_ERROR(T_("TeX engine failed for some reason (see log file)."));
  }
}

/* _________________________________________________________________________

   Driver::Ready

   Decide if looping again is needed.

   LaTeX (and the package changebar) report in the LOG file if it
   should be rerun.  This is needed for files included from subdirs,
   since texi2dvi does not try to compare xref files in subdirs.
   Performing xref files test is still good since LaTeX does not
   report changes in xref files.
   _________________________________________________________________________ */

bool Driver::Ready()
{
  PathName logName(jobName);
  logName.AppendExtension(".log");

  if (Contains(logName, "Rerun to get"))
  {
    return false;
  }

  vector<string> auxFiles;

  GetAuxFiles(auxFiles);

  // If old and new lists don't at least have the same file list, then
  // one file or another has definitely changed.
  if (previousAuxFiles != auxFiles)
  {
    return false;
  }

  // File list is the same.  We must compare each file until we find
  // a difference.
  for (const string& aux : auxFiles)
  {
    PathName auxFile(auxDirectory, PathName(aux));
    app->Verbose(fmt::format(T_("comparing xref file {}..."), Q_(aux)));
    // We only need to keep comparing until we find one that
    // differs, because we'll have to run texindex & tex again no
    // matter how many more there might be.
    if (!File::Equals(PathName(aux), auxFile))
    {
      app->Verbose(fmt::format(T_("xref file {} differed..."), Q_(aux)));
      return false;
    }
  }

  return true;
}

void Driver::InstallOutputFile()
{
  const char* ext = options->outputType == OutputType::PDF ? ".pdf" : ".dvi";
  app->Verbose(fmt::format(T_("copying {} file from {} to {}..."), ext, Q_(workingDirectory), Q_(options->startDirectory)));
  PathName pathSource(workingDirectory, jobName);
  pathSource.AppendExtension(ext);
  PathName pathDest(options->startDirectory, jobName);
  pathDest.AppendExtension(ext);
  File::Copy(pathSource, pathDest);
  if (options->synctex != SyncTeXOption::Disabled)
  {
    const char* synctexExt = options->synctex == SyncTeXOption::Compressed ? ".synctex.gz" : ".synctex";
    PathName pathSyncTeXSource(workingDirectory, jobName);
    pathSyncTeXSource.AppendExtension(synctexExt);
    PathName pathSyncTeXDest(options->startDirectory, jobName);
    pathSyncTeXDest.AppendExtension(synctexExt);
    File::Copy(pathSyncTeXSource, pathSyncTeXDest);
  }
}

void Driver::GetAuxFiles(const PathName& baseName, const char* extension, vector<string>& vec)
{
  PathName pattern(baseName);
  pattern.AppendExtension(extension);

  PathName curDir;
  curDir.SetToCurrentDirectory();

  app->MyTrace(fmt::format(T_("collecting {} in {}..."), Q_(pattern), Q_(curDir)));

  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(curDir, pattern.GetData());

  DirectoryEntry2 entry;

  while (pLister->GetNext(entry))
  {
    // If file is empty, skip it.
    if (entry.isDirectory || entry.size == 0)
    {
      continue;
    }

    // If the file is not suitable to be an index or xref
    // file, don't process it.  The file can't be if its
    // first character is not a backslash or single quote.
    FileStream stream(File::Open(PathName(entry.name), FileMode::Open, FileAccess::Read));
    char buf[1];
    if (stream.Read(buf, 1) == 1 && (buf[0] == '\\' || buf[0] == '\''))
    {
      vec.push_back(entry.name);
    }
    stream.Close();
  }
}

/* _________________________________________________________________________

   Driver::GetAuxFiles

   Get list of xref files (indexes, tables and lists).  Find all files
   having root filename with a two-letter extension, saves the ones
   that are really Texinfo-related files.  .?o? catches LaTeX tables
   and lists.
   _________________________________________________________________________ */

void Driver::GetAuxFiles(vector<string>& auxFiles, vector<string>* idxFiles)
{
  auxFiles.clear();

  if (idxFiles != nullptr)
  {
    idxFiles->clear();
  }

  GetAuxFiles(jobName, ".?o?", auxFiles);
  GetAuxFiles(jobName, ".aux", auxFiles);

  vector<string> files;

  GetAuxFiles(jobName, ".??", files);

  auxFiles.insert(auxFiles.end(), files.begin(), files.end());

  if (idxFiles != nullptr)
  {
    idxFiles->insert(idxFiles->end(), files.begin(), files.end());
  }

  files.clear();

  GetAuxFiles(jobName, ".idx", files);

  auxFiles.insert(auxFiles.end(), files.begin(), files.end());

  if (idxFiles != nullptr)
  {
    idxFiles->insert(idxFiles->end(), files.begin(), files.end());
  }

  sort(auxFiles.begin(), auxFiles.end());
}

void Driver::RunViewer()
{
  const char* ext = options->outputType == OutputType::PDF ? ".pdf" : ".dvi";

  PathName pathFileName(jobName);
  pathFileName.AppendExtension(ext);

  PathName pathDest(options->startDirectory, pathFileName);

  if (options->viewerOptions.empty())
  {
    app->Verbose(fmt::format(T_("opening {}..."), Q_(pathDest)));
#if defined(MIKTEX_WINDOWS)
    if (ShellExecuteW(nullptr, L"open", pathDest.ToWideCharString().c_str(), 0, options->startDirectory.ToWideCharString().c_str(), SW_SHOW) <= reinterpret_cast<HINSTANCE>(32))
    {
      MIKTEX_FATAL_ERROR(T_("The viewer could not be started."));
    }
#else
    UNIMPLEMENTED();
#endif
  }
  else
  {
    wchar_t szExecutable[BufferSizes::MaxPath];
#if defined(MIKTEX_WINDOWS)
    if (FindExecutableW(pathDest.ToWideCharString().c_str(), options->startDirectory.ToWideCharString().c_str(), szExecutable) <= reinterpret_cast<HINSTANCE>(32))
    {
      MIKTEX_FATAL_ERROR(T_("The viewer could not be located."));
    }
#else
    UNIMPLEMENTED();
#endif
    vector<string> args{ PathName(szExecutable).GetFileNameWithoutExtension().ToString() };
    args.insert(args.end(), options->viewerOptions.begin(), options->viewerOptions.end());
    args.push_back(pathDest.ToString());
    app->Verbose(fmt::format(T_("running {}..."), CommandLineBuilder(args).ToString()));
    Process::Start(PathName(szExecutable), args, nullptr, nullptr, nullptr, nullptr, options->startDirectory.GetData());
  }
}

void Driver::Run()
{
  ExpandMacros();
  Application::CheckCancel();
  InsertCommands();

  SetIncludeDirectories();

  // If clean mode was specified, then cd to the temporary working
  // directory.
  if (options->clean)
  {
    app->Verbose(fmt::format("cd {}", Q_(workingDirectory)));
    Directory::SetCurrent(workingDirectory);
  }

  for (int i = 0; i < options->maxIterations; ++i)
  {
    Application::CheckCancel();
    vector<string> idxFiles;
    GetAuxFiles(previousAuxFiles, &idxFiles);
    if (!previousAuxFiles.empty())
    {
      app->Verbose(fmt::format(T_("backing up xref files: {}"), FlattenStringVector(previousAuxFiles, ' ')));
      CopyFiles(previousAuxFiles, auxDirectory);
    }
    RunBibTeX();
    if (idxFiles.size() > 0)
    {
      Application::CheckCancel();
      RunIndexGenerator(idxFiles);
    }
    Application::CheckCancel();
    RunTeX();
    if (Ready())
    {
      break;
    }
  }

  // If we were in clean mode, compilation was in a tmp directory.
  // Copy the DVI (or PDF) file into the directory where the
  // compilation has been done.  (The temp dir is about to get removed
  // anyway.)  We also return to the original directory so that
  // - the next file is processed in correct conditions
  // - the temporary file can be removed
  if (options->clean)
  {
    InstallOutputFile();
  }

  if (options->runViewer)
  {
    RunViewer();
  }
}

enum CommandLineOptions {
  OPT_AAA = 1000,
#if defined(WITH_TEXINFO)
  OPT_AT,
#endif
  OPT_BATCH,
  OPT_CLEAN,
  OPT_DEBUG,
  OPT_ENGINE,
  OPT_EXPAND,
  OPT_INCLUDE,
  OPT_JOB_NAME,
  OPT_LANGUAGE,
  OPT_MAX_ITER,
  OPT_MKIDX_OPTION,
  OPT_PDF,
  OPT_QUIET,
  OPT_RUN_VIEWER,
#if defined(SUPPORT_OPT_SRC_SPECIALS)
  OPT_SRC,
  OPT_SRC_SPECIALS,
#endif
  OPT_SYNCTEX,
#if defined(WITH_TEXINFO)
  OPT_TEXINFO,
#endif
  OPT_TEX_OPTION,
  OPT_TRACE,
  OPT_VERBOSE,
  OPT_VERSION,
  OPT_VIEWER_OPTION,
};

const struct poptOption optionTable[] = {
#if defined(WITH_TEXINFO)
  {
    "texiat", '@',
    POPT_ARG_NONE, nullptr,
    OPT_AT,
    T_("Use @input instead of \\input; for preloaded Texinfo."),
    nullptr,
  },
#endif

  {
    "batch", 'b',
    POPT_ARG_NONE, nullptr,
    OPT_BATCH,
    T_("No interaction."),
    nullptr,
  },

  {
    "clean", 'c',
    POPT_ARG_NONE, nullptr,
    OPT_CLEAN,
    T_("Remove all auxiliary files."),
    nullptr,
  },

  {
    "debug", 'D',
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_DEBUG,
    T_("Print debug information."),
    nullptr,
  },

  {
    "engine", 0,
    POPT_ARG_STRING, nullptr,
    OPT_ENGINE,
    T_("Use the specified TeX engine."),
    T_("ENGINE"),
  },

  {
    "expand", 'e',
    POPT_ARG_NONE, nullptr,
    OPT_EXPAND,
    T_("Force macro expansion using makeinfo."),
    nullptr,
  },

  {
    "include-directory", 'I',
    POPT_ARG_STRING, nullptr,
    OPT_INCLUDE,
    T_("Prepend DIR to the input search path."),
    T_("DIR"),
  },

  {
    "job-name", 0,
    POPT_ARG_STRING, nullptr,
    OPT_JOB_NAME,
    T_("Set the job name and hence the name(s) of the output file(s)."),
    T_("JOBNAME"),
  },

  {
    "language", 'l',
    POPT_ARG_STRING, nullptr,
#if defined(WITH_TEXINFO)
    OPT_LANGUAGE,
    T_("Specify the LANG of FILE: LaTeX or Texinfo."),
#else
    OPT_LANGUAGE | POPT_ARGFLAG_DOC_HIDDEN,
    T_(""),
#endif
    T_("LANG"),
  },

  {
    "pdf", 'p',
    POPT_ARG_NONE, nullptr,
    OPT_PDF,
    T_("Use pdftex or pdflatex for processing."),
    nullptr,
  },

  {
    "quiet", 'q',
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("No output unless errors (implies --batch)."),
    nullptr,
  },

  {
    "silent", 's',
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_QUIET,
    T_("No output unless errors (implies --batch)."),
    nullptr,
  },

  #if defined(WITH_TEXINFO)
  {
    "texinfo", 't',
    POPT_ARG_STRING, nullptr,
    OPT_TEXINFO,
    T_("Insert CMD after @setfilename in copy of input file."),
    "CMD",
  },
#endif

  {
    "version", 'v',
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Display version information and exit successfully."),
    nullptr,
  },

  {
    "verbose", 'V',
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Report on what is done."),
    nullptr,
  },


  // --- now the MiKTeX extensions

  {
    "max-iterations", 0,
    POPT_ARG_STRING, nullptr,
    OPT_MAX_ITER,
    T_("Limit number of iterations."),
    "N",
  },

  {
    "mkidx-option", 0,
    POPT_ARG_STRING, nullptr,
    OPT_MKIDX_OPTION,
    T_("Pass OPTION to the index generator."),
    T_("OPTION"),
  },

#if defined(SUPPORT_OPT_SRC_SPECIALS)
  {
    "src", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SRC,
    T_("Pass option --src-specials to the TeX engine."),
    nullptr,
  },

  {
    "src-specials", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_SRC_SPECIALS,
    T_("Pass option --src-specials[=SRCSPECIALS] to the TeX engine."),
    T_("SRCSPECIALS"),
  },
#endif

  {
    "synctex", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_SYNCTEX,
    T_("Pass option --synctex=N to the TeX engine."),
    "N",
  },

  {
    "tex-option", 0,
    POPT_ARG_STRING, nullptr,
    OPT_TEX_OPTION,
    T_("Pass OPTION to the TeX engine."),
    T_("OPTION"),
  },

  {
    "trace", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_TRACE,
    T_("Turn on tracing.  TRACESTREAMS, if specified, is a comma-separated list of trace stream names (see the MiKTeX manual)."),
    T_("TRACESTREAMS"),
  },

  {
    "run-viewer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_RUN_VIEWER,
    T_("Run a viewer on the resulting DVI/PDF file."),
    nullptr,
  },

  {
    "viewer-option", 0,
    POPT_ARG_STRING, nullptr,
    OPT_VIEWER_OPTION,
    T_("Pass OPTION to the viewer."),
    T_("OPTION"),
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

void McdApp::Run(int argc, const char** argv)
{
  Session::InitInfo initInfo(argv[0]);
  vector<const char*> newargv(&argv[0], &argv[argc + 1]);
  ExamineArgs(newargv, initInfo);

  PoptWrapper popt(newargv.size() - 1, &newargv[0], optionTable);

  int option;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
#if defined(WITH_TEXINFO)
    case OPT_AT:
      break;
#endif
    case OPT_BATCH:
      options.batch = true;
      break;
    case OPT_CLEAN:
      options.clean = true;
      break;
    case OPT_DEBUG:
      options.traceStreams = StringUtil::Flatten(DEFAULT_TRACE_OPTIONS, ',');
      break;
    case OPT_ENGINE:
      if (!options.SetEngine(optArg))
      {
        FatalError(fmt::format(T_("{}: unknown engine"), optArg.c_str()));
      }
      break;
    case OPT_EXPAND:
      options.expand = true;
      break;
    case OPT_INCLUDE:
    {
      PathName path;
      if (PathNameUtil::IsAbsolutePath(optArg))
      {
        path = optArg;
      }
      else
      {
        path = options.startDirectory / PathName(optArg);
      }
      path.ConvertToUnix();
      options.includeDirectories.push_back(path.GetData());
      break;
    }
    case OPT_JOB_NAME:
      options.jobName = optArg;
      break;
    case OPT_LANGUAGE:
      if (StringCompare(optArg.c_str(), "latex", true) == 0)
      {
        options.macroLanguage = MacroLanguage::LaTeX;
      }
#if defined(WITH_TEXINFO)
      else if (StringCompare(optArg.c_str(), "texinfo", true) == 0)
      {
        options.macroLanguage = MacroLanguage::Texinfo;
      }
#endif
      else
      {
        FatalError(fmt::format(T_("{}: unknown language"), optArg));
      }
      break;
    case OPT_PDF:
      options.outputType = OutputType::PDF;
      break;
    case OPT_QUIET:
      options.quiet = true;
      options.batch = true;
      break;
#if defined(WITH_TEXINFO)
    case OPT_TEXINFO:
      options.texinfoCommands.push_back(optArg);
      break;
#endif
    case OPT_VERSION:
      Version();
      return;
    case OPT_VERBOSE:
      options.verbose = true;
      break;
#if defined(SUPPORT_OPT_SRC_SPECIALS)
    case OPT_SRC:
      options.sourceSpecialsWhere = "";
      options.sourceSpecials = true;
      break;
    case OPT_SRC_SPECIALS:
      if (!optArg.empty())
      {
        options.sourceSpecialsWhere = optArg;
      }
      options.sourceSpecials = true;
      break;
#endif
    case OPT_SYNCTEX:
      if (optArg.empty())
      {
        options.synctex = SyncTeXOption::Compressed;
      }
      else
      {
        int synctex = std::stoi(optArg);
        if (synctex == 0)
        {
          options.synctex = SyncTeXOption::Disabled;
        }
        else
        {
          options.synctex = synctex < 0 ? SyncTeXOption::Uncompressed : SyncTeXOption::Compressed;
        }
      }
      break;
    case OPT_MKIDX_OPTION:
      options.makeindexOptions.push_back(optArg);
      break;
    case OPT_TEX_OPTION:
      for (const string& o : forbiddenTexOptions)
      {
        if (optArg.find(o) != string::npos)
        {
          FatalError(fmt::format(T_("TeX option \"--{}\" is not supported."), o));
        }
      }
      options.texOptions.push_back(optArg);
      break;
    case OPT_VIEWER_OPTION:
      options.viewerOptions.push_back(optArg);
      break;
    case OPT_RUN_VIEWER:
      options.runViewer = true;
      break;
    case OPT_MAX_ITER:
      options.maxIterations = std::stoi(optArg);
      break;
    case OPT_TRACE:
      if (optArg.empty())
      {
        options.traceStreams = StringUtil::Flatten(DEFAULT_TRACE_OPTIONS, ',');
      }
      else
      {
        options.traceStreams = optArg;
      }
      break;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    FatalError(msg);
  }

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    FatalError(T_("Missing file argument."));
  }

  if (options.traceStreams.length() > 0)
  {
    initInfo.SetTraceFlags(options.traceStreams);
  }

  Init(initInfo);

  for (const string& fileName : leftovers)
  {
    Verbose(fmt::format(T_("processing {}..."), Q_(fileName)));

    // See if the file exists.  If it doesn't we're in trouble since, // even though the user may be able to reenter a valid filename at
    // the tex prompt (assuming they're attending the terminal), this
    // script won't be able to find the right xref files and so forth.
    if (!File::Exists(PathName(fileName)))
    {
      FatalError(T_("The input file could not be found."));
    }

    Driver driver;
    driver.Initialize(this, &options, fileName.c_str());
    driver.Run();
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

int MAIN(int argc, MAINCHAR* argv[])
{
  McdApp app;
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

  catch (const MiKTeXException& ex)
  {
    Application::Sorry(THE_NAME_OF_THE_GAME, ex);
    ex.Save();
    return 1;
  }

  catch (const exception& ex)
  {
    Application::Sorry(THE_NAME_OF_THE_GAME, ex);
    return 1;
  }

  catch (int exitCode)
  {
    return exitCode;
  }
}
