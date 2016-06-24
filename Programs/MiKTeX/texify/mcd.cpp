/* mcd.cpp: MiKTeX compiler driver

   Copyright (C) 1998-2016 Christian Schenk

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
#include <cstdarg>
#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

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

#include <regex.h>

#include "mcd-version.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

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

   Ultimately, the behavior should probably be included into bibtex, however in this way, an "orthodox" latex distribution is
   maintained... */

#define SF464378__CHAPTERBIB

#define PROGRAM_NAME "texify"

#if !defined(THE_NAME_OF_THE_GAME)
#  define THE_NAME_OF_THE_GAME T_("MiKTeX Compiler Driver")
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

const char * DEFAULT_TRACE_STREAMS = MIKTEX_TRACE_ERROR "," MIKTEX_TRACE_PROCESS "," PROGRAM_NAME;

class ProcessOutputTrash :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n) override
  {
    ALWAYS_UNUSED(pOutput);
    ALWAYS_UNUSED(n);
    return true;
  }
};

class ProcessOutputSaver :
  public IRunProcessCallback
{
public:
  bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n) override
  {
    output.append(reinterpret_cast<const char*>(pOutput), n);
    return true;
  }
public:
  const string & GetOutput() const
  {
    return output;
  }
private:
  string output;
};

void CopyFiles(const vector<string> & vec, const PathName & destDir)
{
  for (const string & fileName : vec)
  {
    File::Copy(fileName, PathName(destDir, fileName));
  }
}

vector<char> ReadFile(const PathName & fileName)
{
  size_t fileSize = File::GetSize(fileName);
  vector<char> vec(fileSize + 1);
  FileStream stream(File::Open(fileName, FileMode::Open, FileAccess::Read));
  fileSize = stream.Read(&vec[0], fileSize);
  stream.Close();
  vec[fileSize] = 0;
  return vec;
}

bool Contains(const PathName & fileName, regex_t * preg)
{
  vector<char> file = ReadFile(fileName);
  return regexec(preg, &file[0], 0, nullptr, 0) == 0;
}

bool Contains(const PathName & fileName, const char * lpszText)
{
  unique_ptr<MemoryMappedFile> pMappedFile(MemoryMappedFile::Create());
  const char * ptr = reinterpret_cast<char*>(pMappedFile->Open(fileName, false));
  size_t size = pMappedFile->GetSize();
  const char * p = lpszText;
  for (size_t i = 0; *p != 0 && i < size; ++i, ++ptr)
  {
    if (*ptr == *p)
    {
      ++p;
    }
    else
    {
      p = lpszText;
    }
  }
  pMappedFile->Close();
  return *p == 0;
}

string FlattenStringVector(const vector<string> & vec, char sep)
{
  string str = "";
  for (const string & s : vec)
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

bool IsPrefixOf(const char * lpszPrefix, const string & str)
{
  return str.compare(0, StrLen(lpszPrefix), lpszPrefix) == 0;
}

enum class MacroLanguage
{
  None, LaTeX, Texinfo
};

enum class Engine
{
  NotSet, TeX, pdfTeX, XeTeX, LuaTeX,
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

public:
  regex_t regex_texinfo_version;

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

public:
  vector<string> texinfoCommands;

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
  string texindexProgram;

public:
  Argv makeindexOptions;

public:
  Argv texOptions;

public:
  Argv viewerOptions;

public:
  string traceStreams;

private:
  string SetProgramName(const string & envName, const string & defaultProgram)
  {
    string programName;
    if (!Utils::GetEnvironmentString(envName.c_str(), programName))
    {
      programName = defaultProgram;
    }
    return programName;
  }

public:
  Engine engine = Engine::NotSet;

public:
  void SetEngine(const string & engineName)
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
    else
    {
      MIKTEX_UNEXPECTED();
    }
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

  if (regcomp(&regex_texinfo_version, "\\[(.*)version (....)-(..)-(..)", REG_EXTENDED | REG_NEWLINE) != 0)
  {
    MIKTEX_UNEXPECTED();
  }

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

  startDirectory.ToUnix();

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
  xetexProgram = SetProgramName("XETEX", MIKTEX_XETEX_EXE);
  luatexProgram = SetProgramName("LUATEX", MIKTEX_LUATEX_EXE);
}

Options::~Options()
{
  regfree(&regex_bibdata);
  regfree(&regex_bibstyle);
  regfree(&regex_citation_undefined);
  regfree(&regex_no_file_bbl);
  regfree(&regex_texinfo_version);
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
  void Run(int argc, const char ** argv);

public:
  void Trace(const char * lpszFormat, ...);

public:
  void Verbose(const char * lpszFormat, ...);

private:
  void Version();

private:
  unique_ptr<TraceStream> traceStream;

public:
  Options options;

private:
  vector<string> forbiddenTexOptions;
};

void McdApp::Verbose(const char * lpszFormat, ...)
{
  va_list arglist;
  string s;
  VA_START(arglist, lpszFormat);
  s = StringUtil::FormatString(lpszFormat, arglist);
  VA_END(arglist);
  traceStream->WriteLine(PROGRAM_NAME, s.c_str());
  if (options.verbose)
  {
    cout << s << endl;
  }
}

void McdApp::Trace(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  traceStream->WriteLine(PROGRAM_NAME, StringUtil::FormatString(lpszFormat, arglist).c_str());
  va_end(arglist);
}

void McdApp::Version()
{
  cout
    << Utils::MakeProgramVersionString(THE_NAME_OF_THE_GAME, VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
    << "Copyright (C) 1998-2016 Christian Schenk" << endl
    << "Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2001," << endl
    << "2002, 2003, 2004, 2005 Free Software Foundation, Inc." << endl
    << "This is free software; see the source for copying conditions.  There is NO" << endl
    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

class Driver
{
public:
  virtual ~Driver();

public:
  void Initialize(McdApp * pApplication, Options * pOptions, const char * lpszFileName);

public:
  void Run();

private:
  void FatalUtilityError(const string & name)
  {
    pApplication->FatalError("A required utility could not be found. Utility name: %s.", name.c_str());
  }

private:
  MacroLanguage GuessMacroLanguage(const PathName & fileName);

private:
  void TexinfoPreprocess(const PathName & pathFrom, const PathName & pathTo);

private:
  void TexinfoUncomment(const PathName & pathFrom, const PathName & pathTo);

private:
  void SetIncludeDirectories();

private:
  void ExpandMacros();

private:
  void InsertCommands();

private:
  bool RunMakeinfo(const PathName & pathFrom, const PathName & pathTo);

private:
  void RunBibTeX();

private:
  PathName GetTeXEnginePath(string & exeName);

private:
  void RunTeX();

private:
  void RunIndexGenerator(const vector<string> & idxFiles);

private:
  void RunViewer();

private:
  bool Ready();

private:
  bool Check_texinfo_tex();

private:
  void InstallOutputFile();

private:
  void GetAuxFiles(vector<string> & auxFiles, vector<string> * pIdxFiles = nullptr);

private:
  void GetAuxFiles(const PathName & baseName, const char * lpszExtension, vector<string> & auxFiles);

private:
  void InstallProgram(const char * lpszProgram);

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

  // the extra directory: contains the input file, if --texinfo was
  // specified on the command-line
private:
  PathName extraDirectory;

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
  McdApp * pApplication = nullptr;

private:
  Options * pOptions = nullptr;

protected:
  shared_ptr<Session> pSession = Session::Get();
};

Driver::~Driver()
{
  try
  {
    if (pOptions != nullptr && pOptions->clean)
    {
      Directory::SetCurrent(pOptions->startDirectory);
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

void Driver::Initialize(McdApp * pApplication, Options * pOptions, const char * lpszFileName)
{
  this->pApplication = pApplication;
  this->pOptions = pOptions;

  pApplication->Trace(T_("initializing driver..."));

  givenFileName = lpszFileName;

  // Get the name of the current directory.  We want the full path
  // because in clean mode we are in tmp, in which case a relative
  // path has no meaning.
  originalInputFile = givenFileName;
  originalInputFile.MakeAbsolute();
  originalInputFile.ToUnix();

  pApplication->Trace(T_("input file: %s"), Q_(originalInputFile));

  if (pOptions->jobName.empty())
  {
    givenFileName.GetFileNameWithoutExtension(jobName);
  }
  else
  {
    jobName = pOptions->jobName;
  }

  inputName = givenFileName;
  inputName.RemoveDirectorySpec();

  // create a super-temp directory
  tempDirectory = TemporaryDirectory::Create();

  // create scratch directory
  workingDirectory = tempDirectory->GetPathName() / "_src";
  Directory::Create(workingDirectory);
  workingDirectory.ToUnix();
  pApplication->Trace(T_("working directory: %s"), Q_(workingDirectory));

  // create extra directory
  extraDirectory = tempDirectory->GetPathName() / "_xtr";
  Directory::Create(extraDirectory);
  extraDirectory.ToUnix();
  pApplication->Trace(T_("extra directory: %s"), Q_(extraDirectory));

  // create aux directory
  auxDirectory = tempDirectory->GetPathName() / "_aux";
  Directory::Create(auxDirectory);
  auxDirectory.ToUnix();
  pApplication->Trace(T_("aux directory: %s"), Q_(auxDirectory));

  // If the user explicitly specified the language, use that.
  // Otherwise, if the first line is \input texinfo, assume it's
  // texinfo.  Otherwise, guess from the file extension.
  macroLanguage = pOptions->macroLanguage;
  if (macroLanguage == MacroLanguage::None)
  {
    macroLanguage = GuessMacroLanguage(givenFileName);
  }

  // make fully qualified path to the given input file
  if (Utils::IsAbsolutePath(givenFileName.Get()))
  {
    pathInputFile = givenFileName;
  }
  else
  {
    pathInputFile.Set(pOptions->startDirectory, givenFileName);
  }

  originalInputDirectory = pathInputFile;
  originalInputDirectory.RemoveFileSpec();
  originalInputDirectory.ToUnix();
}

MacroLanguage Driver::GuessMacroLanguage(const PathName & fileName)
{
  StreamReader reader(fileName);
  string firstLine;
  if (!reader.ReadLine(firstLine))
  {
    return MacroLanguage::None;
  }
  reader.Close();
  if (firstLine.find("input texinfo") != string::npos)
  {
    return MacroLanguage::Texinfo;
  }
  if (fileName.HasExtension(".dtx")
    || fileName.HasExtension(".tex")
    || fileName.HasExtension(".latex")
    || fileName.HasExtension(".ltx"))
  {
    return MacroLanguage::LaTeX;
  }
  else
  {
    return MacroLanguage::Texinfo;
  }
}

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

void Driver::TexinfoPreprocess(const PathName & pathFrom, const PathName & pathTo)
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

/* _________________________________________________________________________

   Driver::TexinfoUncomment

   Uncommenting is simple: Remove any leading `@c texi2dvi'.
   _________________________________________________________________________ */

void Driver::TexinfoUncomment(const PathName & pathFrom, const PathName & pathTo)
{
  StreamReader reader(pathFrom);
  StreamWriter writer(pathTo);
  string line;
  while (reader.ReadLine(line))
  {
    if (IsPrefixOf("@c texi2dvi", line))
    {
      writer.WriteLine(line.c_str() + 11);
    }
    else
    {
      writer.WriteLine(line);
    }
  }
  writer.Close();
  reader.Close();
}

/* _________________________________________________________________________

   Driver::SetIncludeDirectories

   Source file might include additional sources.  Put `.' and
   directory where source file(s) reside in MIKTEX_CWD before anything
   else.  Include orig_pwd in case we are in clean mode, where we've
   cd'd to a temp directory.
   _________________________________________________________________________ */

void Driver::SetIncludeDirectories()
{
  pSession->AddInputDirectory(pOptions->startDirectory.Get(), true);
  if (originalInputDirectory != pOptions->startDirectory)
  {
    pSession->AddInputDirectory(originalInputDirectory.Get(), true);
  }
  for (const string & dir : pOptions->includeDirectories)
  {
    pSession->AddInputDirectory(dir.c_str(), true);
  }
}

/* _________________________________________________________________________

   Driver::RunMakeinfo

   Expand macro commands in the original source file using Makeinfo.
   Always use `end' footnote style, since the `separate' style
   generates different output (arguably this is a bug in -E).  Discard
   main info output, the user asked to run TeX, not makeinfo.
   _________________________________________________________________________ */

bool Driver::RunMakeinfo(const PathName & pathFrom, const PathName & pathTo)
{
  PathName pathExe;

  if (!pSession->FindFile(pOptions->makeinfoProgram.c_str(), FileType::EXE, pathExe))
  {
    FatalUtilityError(pOptions->makeinfoProgram);
  }

  CommandLineBuilder commandLine;

  commandLine.AppendOption("--footnote-style=", "end");
  commandLine.AppendOption("-I ", originalInputDirectory);

  for (const string & dir : pOptions->includeDirectories)
  {
    commandLine.AppendOption("-I ", dir);
  }

#if defined(MIKTEX_WINDOWS)
  commandLine.AppendOption("-o ", "nul");
#else
  commandLine.AppendOption("-o ", "/dev/null");
#endif

  commandLine.AppendOption("--macro-expand=", pathTo);

  commandLine.AppendArgument(pathFrom);

  int exitCode = 0;

  ProcessOutputTrash trash;
  Process::Run(pathExe, commandLine.ToString(), &trash, &exitCode, nullptr);

  return exitCode == 0;
}

/* _________________________________________________________________________

   Driver::Check_texinfo_tex

   Check if texinfo.tex performs macro expansion by looking for its
   version.  The version is a date of the form YEAR-MO-DA.
   _________________________________________________________________________ */

   // minimum texinfo.tex version to have macro expansion
const char * txiprereq = "19990129";

bool Driver::Check_texinfo_tex()
{
  PathName pathExe;

  if (!pSession->FindFile(pOptions->texProgram.c_str(), FileType::EXE, pathExe))
  {
    FatalUtilityError(pOptions->texProgram);
  }

  bool newer = false;

  unique_ptr<TemporaryDirectory> tmpdir = TemporaryDirectory::Create();

  PathName fileName = tmpdir->GetPathName() / "txiversion.tex";
  StreamWriter writer(fileName);
  writer.WriteLine("\\input texinfo.tex @bye");
  writer.Close();

  int exitCode = 0;
  ProcessOutputSaver processOutput;
  if (!Process::Run(pathExe, fileName.GetData(), &processOutput, &exitCode, tmpdir->GetPathName().GetData()))
  {
    MIKTEX_UNEXPECTED();
  }

  if (exitCode == 0)
  {
    regmatch_t regMatch[5];
    if (regexec(&pOptions->regex_texinfo_version, processOutput.GetOutput().c_str(), 5, regMatch, 0) == 0)
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
      pApplication->Verbose(T_("texinfo.tex preloaded as %s, version is %s..."), txiformat.c_str(), version.c_str());
      newer = (atoi(txiprereq) <= atoi(version.c_str()));
    }
  }

  return !newer;
}

void Driver::ExpandMacros()
{
  if (macroLanguage != MacroLanguage::Texinfo)
  {
    return;
  }

  // Unless required by the user, makeinfo expansion is wanted only if
  // texinfo.tex is too old.
  bool expand = pOptions->expand;
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

  pApplication->Verbose(T_("macro-expanding %s to %s..."), Q_(givenFileName), Q_(path));

  TexinfoPreprocess(originalInputFile, pathTmpFile1);

  bool expanded = RunMakeinfo(pathTmpFile1, pathTmpFile2);

  TexinfoUncomment(pathTmpFile2, path);

  File::Delete(pathTmpFile1);
  File::Delete(pathTmpFile2);

  if (expanded && File::Exists(path))
  {
    pathInputFile = path;
    pathInputFile.ToUnix();
  }
  else
  {
    // If makeinfo failed (or was not even run), use the original
    // file as input.
    pApplication->Verbose(T_("reverting to %s..."), Q_(givenFileName));
  }
}

/* _________________________________________________________________________

   Driver::InsertCommands

   Used most commonly for @finalout, @smallbook, etc.
   _________________________________________________________________________ */

void Driver::InsertCommands()
{
  if (!(pOptions->texinfoCommands.size() > 0 && macroLanguage == MacroLanguage::Texinfo))
  {
    return;
  }
  string extra = FlattenStringVector(pOptions->texinfoCommands, '\n');
  pApplication->Verbose(T_("inserting extra commands: %s"), extra.c_str());
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
  pathInputFile.ToUnix();
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

  if (!pSession->FindFile(pOptions->bibtexProgram.c_str(), FileType::EXE, pathExe))
  {
    FatalUtilityError(pOptions->bibtexProgram);
  }

  PathName logName(0, jobName, ".log");
  PathName auxName(0, jobName, ".aux");

  int exitCode;

#if defined(SF464378__CHAPTERBIB)
  if ((File::Exists(auxName)
    && File::Exists(logName)
    && Contains(logName, &pOptions->regex_chapterbib)
    && (Contains(logName, &pOptions->regex_citation_undefined)
      || Contains(logName, &pOptions->regex_no_file_bbl))))
  {
    pApplication->Verbose(T_("ChapterBib detected. Preparing to run BibTeX on first-level aux files..."));

    // read the main .aux file
    vector<char> auxFile = ReadFile(auxName);

    regmatch_t regMatch[2];

    for (int offset = 0; ((regexec(&pOptions->regex_input_aux, &auxFile[offset], 1, regMatch, offset != 0 ? REG_NOTBOL : 0) == 0) && (regMatch[0].rm_so > -1)); offset += regMatch[0].rm_eo)
    {
      // get SubAuxNameNoExt out of \@input{SubAuxNameNoExt.aux}
      string tmp(&auxFile[offset + regMatch[0].rm_so + 8], regMatch[0].rm_eo - regMatch[0].rm_so - 13);
      PathName subAuxNameNoExt(tmp);

      // append .aux extension
      PathName subAuxName(0, subAuxNameNoExt, ".aux");
      if (!(File::Exists(subAuxName)
        && Contains(subAuxName, &pOptions->regex_bibdata)
        && Contains(subAuxName, &pOptions->regex_bibstyle)))
      {
        continue;
      }

      PathName subDir;

      if (strchr(subAuxNameNoExt.Get(), PathName::UnixDirectoryDelimiter) != 0)
      {
        // we have \@input{SubDir/SubAuxNameNoExt.aux}
        if (pOptions->clean)
        {
          MIKTEX_FATAL_ERROR(T_("Sub-directories not supported when --clean is in effect."));
        }
        subDir = subAuxNameNoExt;
        subDir.RemoveFileSpec();
        subDir.MakeAbsolute();
        subAuxNameNoExt.RemoveDirectorySpec();
      }

      CommandLineBuilder commandLine;

      commandLine.AppendArgument(subAuxNameNoExt);

      pApplication->Verbose(T_("running %s %s..."), Q_(pOptions->bibtexProgram), Q_(commandLine.ToString()));

      exitCode = 0;

      ProcessOutputTrash trash;

      Process::Run(pathExe, commandLine.ToString(), (pOptions->quiet ? &trash : nullptr), &exitCode, subDir.Empty() ? nullptr : subDir.GetData());
      if (exitCode != 0)
      {
        MIKTEX_FATAL_ERROR(T_("BibTeX failed for some reason."));
      }
    }
  }
#endif  // SF464378__CHAPTERBIB

  if (!(File::Exists(auxName)
    && Contains(auxName, &pOptions->regex_bibdata)
    && Contains(auxName, &pOptions->regex_bibstyle)
    && File::Exists(logName)
    && (Contains(logName, &pOptions->regex_citation_undefined)
      || Contains(logName, &pOptions->regex_no_file_bbl))))
  {
    return;
  }

  CommandLineBuilder commandLine;

  commandLine.AppendArgument(jobName);

  pApplication->Verbose(T_("running %s %s..."), Q_(pOptions->bibtexProgram), commandLine.ToString().c_str());

  ProcessOutputTrash trash;

  Process::Run(pathExe, commandLine.ToString(), (pOptions->quiet ? &trash : nullptr), &exitCode, nullptr);

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

void Driver::RunIndexGenerator(const vector<string> & idxFiles)
{
  const char * lpszExeName =
    (macroLanguage == MacroLanguage::Texinfo
      ? pOptions->texindexProgram.c_str()
      : pOptions->makeindexProgram.c_str());

  PathName pathExe;

  if (!pSession->FindFile(lpszExeName, FileType::EXE, pathExe))
  {
    FatalUtilityError(lpszExeName);
  }

  CommandLineBuilder commandLine;

  commandLine.AppendArguments(pOptions->makeindexOptions);
  commandLine.AppendArguments(idxFiles);

  ProcessOutputTrash trash;

  int exitCode = 0;

  pApplication->Verbose(T_("running %s %s..."), Q_(lpszExeName), commandLine.ToString().c_str());

  Process::Run(pathExe, commandLine.ToString(), (pOptions->quiet ? &trash : nullptr), &exitCode, nullptr);

  if (exitCode != 0)
  {
    MIKTEX_FATAL_ERROR(T_("MakeIndex failed for some reason."));
  }
}

void Driver::InstallProgram(const char * lpszProgram)
{
  ALWAYS_UNUSED(lpszProgram);
  PathName pathExe;
  if (!pSession->FindFile("initexmf", FileType::EXE, pathExe))
  {
    FatalUtilityError("initexmf");
  }
  ProcessOutputTrash trash;
  Process::Run(pathExe, "--mklinks", (pOptions->quiet ? &trash : nullptr));
}

PathName Driver::GetTeXEnginePath(string & exeName)
{
  if (macroLanguage == MacroLanguage::Texinfo)
  {
    if (pOptions->outputType == OutputType::PDF)
    {
      if (pOptions->engine == Engine::XeTeX)
      {
        exeName = pOptions->xetexProgram;
      }
      else if (pOptions->engine == Engine::LuaTeX)
      {
        exeName = pOptions->luatexProgram;
      }
      else
      {
        exeName = pOptions->pdftexProgram;
      }
    }
    else
    {
      if (pOptions->engine == Engine::pdfTeX)
      {
        exeName = pOptions->pdftexProgram;
      }
      else
      {
        exeName = pOptions->texProgram;

      }
    }
  }
  else
  {
    if (pOptions->outputType == OutputType::PDF)
    {
      if (pOptions->engine == Engine::XeTeX)
      {
        exeName = pOptions->xelatexProgram;
      }
      else if (pOptions->engine == Engine::LuaTeX)
      {
        exeName = pOptions->lualatexProgram;
      }
      else
      {
        exeName = pOptions->pdflatexProgram;
      }
    }
    else
    {
      exeName = pOptions->latexProgram;
    }
  }
  PathName pathExe;
  if (!pSession->FindFile(exeName.c_str(), FileType::EXE, pathExe))
  {
    InstallProgram(exeName.c_str());
    if (!pSession->FindFile(exeName.c_str(), FileType::EXE, pathExe))
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

  CommandLineBuilder commandLine;

  if (!pOptions->jobName.empty())
  {
    commandLine.AppendOption("--job-name=", jobName);
  }

#if defined(SUPPORT_OPT_SRC_SPECIALS)
  if (pOptions->sourceSpecials)
  {
    if (!pOptions->sourceSpecialsWhere.empty())
    {
      commandLine.AppendOption("--src-specials=", pOptions->sourceSpecialsWhere);
    }
    else
    {
      commandLine.AppendOption("--src-specials");
    }
  }
#endif

  if (pOptions->synctex != SyncTeXOption::Disabled)
  {
    commandLine.AppendOption("--synctex=", pOptions->synctex == SyncTeXOption::Compressed ? "1" : "-1");
  }

  if (pOptions->quiet)
  {
    commandLine.AppendOption("--quiet");
  }
  if (pOptions->batch && !pOptions->quiet)
  {
    commandLine.AppendOption("--interaction=", "scrollmode");
  }
  commandLine.AppendArguments(pOptions->texOptions);
#if 0
  if (pOptions->traceStreams.length() > 0)
  {
    commandLine.AppendOption("--trace=", pOptions->traceStreams);
  }
#endif
  commandLine.AppendArgument(pathInputFile);

  pApplication->Verbose(T_("running %s %s..."), Q_(exeName), commandLine.ToString().c_str());

  int exitCode = 0;
  Process::Run(pathExe, commandLine.ToString(), nullptr, &exitCode, nullptr);
  if (exitCode != 0)
  {
    PathName logName(PathName(), jobName, ".log");
    if (pOptions->clean)
    {
      try
      {
        File::Copy(logName, PathName(pOptions->startDirectory, logName));
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
  PathName logName(PathName(), jobName, ".log");

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
  for (const string & aux : auxFiles)
  {
    PathName auxFile(auxDirectory, aux);
    pApplication->Verbose(T_("comparing xref file %s..."), Q_(aux));
    // We only need to keep comparing until we find one that
    // differs, because we'll have to run texindex & tex again no
    // matter how many more there might be.
    if (!File::Equals(aux, auxFile))
    {
      pApplication->Verbose(T_("xref file %s differed..."), Q_(aux));
      return false;
    }
  }

  return true;
}

void Driver::InstallOutputFile()
{
  const char * lpszExt = pOptions->outputType == OutputType::PDF ? ".pdf" : ".dvi";
  pApplication->Verbose(T_("copying %s file from %s to %s..."), lpszExt, Q_(workingDirectory), Q_(pOptions->startDirectory));
  PathName pathSource(workingDirectory, jobName, lpszExt);
  PathName pathDest(pOptions->startDirectory, jobName, lpszExt);
  File::Copy(pathSource, pathDest);
  if (pOptions->synctex != SyncTeXOption::Disabled)
  {
    const char * lpszSyncTeXExt = pOptions->synctex == SyncTeXOption::Compressed ? ".synctex.gz" : ".synctex";
    PathName pathSyncTeXSource(workingDirectory, jobName, lpszSyncTeXExt);
    PathName pathSyncTeXDest(pOptions->startDirectory, jobName, lpszSyncTeXExt);
    File::Copy(pathSyncTeXSource, pathSyncTeXDest);
  }
}

void Driver::GetAuxFiles(const PathName & baseName, const char * lpszExtension, vector<string> & vec)
{
  PathName pattern(PathName(), baseName, lpszExtension);

  PathName curDir;
  curDir.SetToCurrentDirectory();

  pApplication->Trace(T_("collecting %s in %s..."), Q_(pattern), Q_(curDir));

  unique_ptr<DirectoryLister> pLister = DirectoryLister::Open(curDir, pattern.Get());

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
    FileStream stream(File::Open(entry.name, FileMode::Open, FileAccess::Read));
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

void Driver::GetAuxFiles(vector<string> & auxFiles, vector<string> * pIdxFiles)
{
  auxFiles.clear();

  if (pIdxFiles != nullptr)
  {
    pIdxFiles->clear();
  }

  GetAuxFiles(jobName, ".?o?", auxFiles);
  GetAuxFiles(jobName, ".aux", auxFiles);

  vector<string> files;

  GetAuxFiles(jobName, ".??", files);

  auxFiles.insert(auxFiles.end(), files.begin(), files.end());

  if (pIdxFiles != nullptr)
  {
    pIdxFiles->insert(pIdxFiles->end(), files.begin(), files.end());
  }

  files.clear();

  GetAuxFiles(jobName, ".idx", files);

  auxFiles.insert(auxFiles.end(), files.begin(), files.end());

  if (pIdxFiles != nullptr)
  {
    pIdxFiles->insert(pIdxFiles->end(), files.begin(), files.end());
  }

  sort(auxFiles.begin(), auxFiles.end());
}

void Driver::RunViewer()
{
  const char * lpszExt = pOptions->outputType == OutputType::PDF ? ".pdf" : ".dvi";

  PathName pathFileName(PathName(), jobName, lpszExt);

  PathName pathDest(pOptions->startDirectory, pathFileName);

  if (pOptions->viewerOptions.GetArgc() == 0)
  {
    pApplication->Verbose(T_("opening %s..."), Q_(pathDest));
#if defined(MIKTEX_WINDOWS)
    if (ShellExecuteW(nullptr, L"open", pathDest.ToWideCharString().c_str(), 0, pOptions->startDirectory.ToWideCharString().c_str(), SW_SHOW) <= reinterpret_cast<HINSTANCE>(32))
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
    if (FindExecutableW(pathDest.ToWideCharString().c_str(), pOptions->startDirectory.ToWideCharString().c_str(), szExecutable) <= reinterpret_cast<HINSTANCE>(32))
    {
      MIKTEX_FATAL_ERROR(T_("The viewer could not be located."));
    }
#else
    UNIMPLEMENTED();
#endif
    CommandLineBuilder commandLine;
    commandLine.AppendArguments(pOptions->viewerOptions);
    commandLine.AppendArgument(pathDest);
    pApplication->Verbose(T_("running %s %s..."), Q_(szExecutable), commandLine.ToString().c_str());
    Process::Start(szExecutable, commandLine.ToString(), nullptr, nullptr, nullptr, nullptr, pOptions->startDirectory.Get());
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
  if (pOptions->clean)
  {
    pApplication->Verbose("cd %s", Q_(workingDirectory));
    Directory::SetCurrent(workingDirectory);
  }

  for (int i = 0; i < pOptions->maxIterations; ++i)
  {
    Application::CheckCancel();
    vector<string> idxFiles;
    GetAuxFiles(previousAuxFiles, &idxFiles);
    if (!previousAuxFiles.empty())
    {
      pApplication->Verbose(T_("backing up xref files: %s"), FlattenStringVector(previousAuxFiles, ' ').c_str());
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
  if (pOptions->clean)
  {
    InstallOutputFile();
  }

  if (pOptions->runViewer)
  {
    RunViewer();
  }
}

enum CommandLineOptions {
  OPT_AAA = 1000,
  OPT_AT,
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
  OPT_TEXINFO,
  OPT_TEX_OPTION,
  OPT_TRACE,
  OPT_VERBOSE,
  OPT_VERSION,
  OPT_VIEWER_OPTION,
};

const struct poptOption optionTable[] = {
  {
    "texiat", '@',
    POPT_ARG_NONE, nullptr,
    OPT_AT,
    T_("Use @input instead of \\input; for preloaded Texinfo."),
    nullptr,
  },

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
    OPT_LANGUAGE,
    T_("Specify the LANG of FILE: LaTeX or Texinfo."),
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

  {
    "texinfo", 't',
    POPT_ARG_STRING, nullptr,
    OPT_TEXINFO,
    T_("Insert CMD after @setfilename in copy of input file."),
    "CMD",
  },

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

void McdApp::Run(int argc, const char ** argv)
{
  Session::InitInfo initInfo(argv[0]);

  PoptWrapper popt(argc, argv, optionTable);

  int option;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_AT:
      break;
    case OPT_BATCH:
      options.batch = true;
      break;
    case OPT_CLEAN:
      options.clean = true;
      break;
    case OPT_DEBUG:
      options.traceStreams = DEFAULT_TRACE_STREAMS;
      break;
    case OPT_ENGINE:
      options.SetEngine(optArg);
      break;
    case OPT_EXPAND:
      options.expand = true;
      break;
    case OPT_INCLUDE:
    {
      PathName path;
      if (Utils::IsAbsolutePath(optArg.c_str()))
      {
        path = optArg;
      }
      else
      {
        path.Set(options.startDirectory, optArg);
      }
      path.ToUnix();
      options.includeDirectories.push_back(path.Get());
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
      else if (StringCompare(optArg.c_str(), "texinfo", true) == 0)
      {
        options.macroLanguage = MacroLanguage::Texinfo;
      }
      else
      {
        FatalError(T_("%s: unknown language"), optArg.c_str());
      }
      break;
    case OPT_PDF:
      options.outputType = OutputType::PDF;
      break;
    case OPT_QUIET:
      options.quiet = true;
      options.batch = true;
      break;
    case OPT_TEXINFO:
      options.texinfoCommands.push_back(optArg);
      break;
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
      options.makeindexOptions.Append(optArg.c_str());
      break;
    case OPT_TEX_OPTION:
      for (const string & o : forbiddenTexOptions)
      {
        if (optArg.find(o) != string::npos)
        {
          FatalError(T_("TeX option \"--%s\" is not supported."), o.c_str());
        }
      }
      options.texOptions.Append(optArg.c_str());
      break;
    case OPT_VIEWER_OPTION:
      options.viewerOptions.Append(optArg.c_str());
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
        options.traceStreams = DEFAULT_TRACE_STREAMS;
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
    FatalError("%s", msg.c_str());
  }

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    FatalError(T_("Missing file argument."));
  }

  if (options.traceStreams.length() > 0)
  {
    initInfo.SetTraceFlags(options.traceStreams.c_str());
  }

  for (const string & fileName : leftovers)
  {
    Init(initInfo);

    Verbose(T_("processing %s..."), Q_(fileName));

    // See if the file exists.  If it doesn't we're in trouble since, // even though the user may be able to reenter a valid filename at
    // the tex prompt (assuming they're attending the terminal), this
    // script won't be able to find the right xref files and so forth.
    if (!File::Exists(fileName))
    {
      FatalError(T_("The input file could not be found."));
    }

    Driver driver;
    driver.Initialize(this, &options, fileName.c_str());
    driver.Run();

    Finalize();
  }
}

extern "C" MIKTEXDLLEXPORT int MIKTEXCEECALL mcdmain2(int argc, const char ** argv)
{
  try
  {
    McdApp app;
    app.Run(argc, argv);
    return 0;
  }
  catch (const MiKTeXException & ex)
  {
    Application::Sorry(argv[0], ex);
    return 1;
  }
  catch (const exception & ex)
  {
    Application::Sorry(argv[0], ex);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
