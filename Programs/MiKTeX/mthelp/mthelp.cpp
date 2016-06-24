/* mthelp.cpp:

   Copyright (C) 2004-2016 Christian Schenk

   This file is a part of MTHelp.

   MTHelp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   MTHelp is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MTHelp; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#if defined(MIKTEX_WINDOWS)
#  include <Windows.h>
#endif

#include "mthelp-version.h"

#include <miktex/App/Application>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Registry>
#include <miktex/Core/Session>
#include <miktex/Core/StreamWriter>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

#include <cstdarg>

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#include "template.html.h"

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

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

const char * const TheNameOfTheGame = T_("MiKTeX Help Utility");
const char * const PROGNAME = "mthelp";

#if defined(MIKTEX_WINDOWS)
const char PATH_DELIMITER = ';';
#define PATH_DELIMITER_STRING ";"
#else
const char PATH_DELIMITER = ':';
#define PATH_DELIMITER_STRING ":"
#endif

#if defined(MIKTEX_WINDOWS)
const char * const DEFAULT_DOC_EXTENSIONS = ".chm;.html;.dvi;.pdf;.ps;.txt";
#else
const char * const DEFAULT_DOC_EXTENSIONS = ".html:.pdf:.dvi:.ps:.txt";
#endif

class MiKTeXHelp :
  public Application
{
public:
  void MIKTEXTHISCALL Init(const Session::InitInfo & initInfo) override;

public:
  void Run(int argc, const char ** argv);

private:
  void ShowVersion();

private:
  void Warning(const char * lpszFormat, ...);

private:
  bool SkipPrefix(const string & str, const char * lpszPrefix, string & result);

private:
  bool SkipTeXMFPrefix(const string & str, string & result);

private:
  void FindDocFilesByPackage(const string & packageName, map<string, vector<string> > & filesByExtension);

private:
  void FindDocFilesByPackage(const string & packageName, vector<string> & files);

private:
  void FindDocFilesByName(const string & name, vector<string> & files);

private:
  void PrintFiles(const vector<string> & files);

private:
  void ViewFile(const PathName & fileName);

private:
  void CreateHtmlAndView(const char * lpszPackageName, const vector<string> & files);

private:
  void WriteText(StreamWriter & writer, const string & text);

private:
  bool printOnly = false;

private:
  bool quiet = false;

private:
  shared_ptr<PackageManager> pManager;

private:
  static const struct poptOption aoption[];
};

enum Option
{
  OPT_AAA = 256,
  OPT_LIST_ONLY,
  OPT_PRINT_ONLY,
  OPT_QUIET,
  OPT_VERSION,
  OPT_VIEW,
};

const struct poptOption MiKTeXHelp::aoption[] = {
  {
    "list-only", 'l', POPT_ARG_NONE, nullptr, OPT_LIST_ONLY,
    T_("List documentation files but do not start a viewer."),
    nullptr
  },
  {
    "print-only", 'n', POPT_ARG_NONE, nullptr, OPT_PRINT_ONLY,
    T_("Print the commands that would be executed, but do not execute them."),
    nullptr
  },
  {
    "quiet", 0, POPT_ARG_NONE, nullptr, OPT_QUIET,
    T_("Suppress all output (except errors)."),
    nullptr
  },
  {
    "version", 0, POPT_ARG_NONE, nullptr, OPT_VERSION,
    T_("Show version information and exit."),
    nullptr
  },
  {
    "view", 0, POPT_ARG_NONE, nullptr, OPT_VIEW,
    T_("View the main documentation file of the package."),
    nullptr
  },
  POPT_AUTOHELP
  POPT_TABLEEND
};

void MiKTeXHelp::Init(const Session::InitInfo & initInfo)
{
  Application::Init(initInfo);
  pManager = PackageManager::Create();
}

void MiKTeXHelp::ShowVersion()
{
  cout << Utils::MakeProgramVersionString(TheNameOfTheGame, MIKTEX_COMPONENT_VERSION_STR) << endl
    << "Copyright (C) 2004-2016 Christian Schenk" << endl
    << "This is free software; see the source for copying conditions.  There is NO" << endl
    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

void MiKTeXHelp::Warning(const char * lpszFormat, ...)
{
  if (quiet)
  {
    return;
  }
  va_list arglist;
  VA_START(arglist, lpszFormat);
  cerr << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
}

bool MiKTeXHelp::SkipPrefix(const string & str, const char * lpszPrefix, string & result)
{
  size_t n = StrLen(lpszPrefix);
  if (str.compare(0, n, lpszPrefix) != 0)
  {
    return false;
  }
  result = str.c_str() + n;
  return true;
}

bool MiKTeXHelp::SkipTeXMFPrefix(const string & str, string & result)
{
  return SkipPrefix(str, "texmf/", result)
    || SkipPrefix(str, "texmf\\", result)
    || SkipPrefix(str, "./texmf/", result)
    || SkipPrefix(str, ".\\texmf\\", result);
}

void MiKTeXHelp::FindDocFilesByPackage(const string & packageName, map<string, vector<string> > & filesByExtension)
{
  PackageInfo pi;
  if (!pManager->TryGetPackageInfo(packageName, pi))
  {
    return;
  }
  for (const string & fileName : pi.docFiles)
  {
    char szExt[BufferSizes::MaxPath];
    PathName::Split(fileName.c_str(), nullptr, 0, nullptr, 0, szExt, BufferSizes::MaxPath);
    string file;
    PathName path;
    if (SkipTeXMFPrefix(fileName, file) && session->FindFile(file.c_str(), MIKTEX_PATH_TEXMF_PLACEHOLDER_NO_MPM, path))
    {
      vector<string> & files = filesByExtension[szExt];
      files.push_back(path.ToString());
    }
  }
}

void MiKTeXHelp::FindDocFilesByPackage(const string & packageName, vector<string> & files)
{
  map<string, vector<string> > filesByExtension;
  FindDocFilesByPackage(packageName, filesByExtension);
  string extensions = session->GetConfigValue(0, MIKTEX_REGVAL_DOC_EXTENSIONS, DEFAULT_DOC_EXTENSIONS);
  for (Tokenizer ext(extensions.c_str(), PATH_DELIMITER_STRING); ext.GetCurrent() != nullptr; ++ext)
  {
    vector<string> & vec = filesByExtension[ext.GetCurrent()];
    vector<string>::iterator it = vec.begin();
    while (it != vec.end())
    {
      char szName[BufferSizes::MaxPath];
      PathName::Split(it->c_str(), nullptr, 0, szName, BufferSizes::MaxPath, nullptr, 0);
      if (PathName::Compare(szName, packageName.c_str()) == 0)
      {
        files.push_back(*it);
        it = vec.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
  for (Tokenizer ext(extensions.c_str(), PATH_DELIMITER_STRING); ext.GetCurrent() != nullptr; ++ext)
  {
    vector<string> & vec = filesByExtension[ext.GetCurrent()];
    files.insert(files.end(), vec.begin(), vec.end());
  }
}

void MiKTeXHelp::FindDocFilesByName(const string & name, vector<string> & files)
{
  string extensions = session->GetConfigValue(0, MIKTEX_REGVAL_DOC_EXTENSIONS, DEFAULT_DOC_EXTENSIONS);
  string searchSpec = MIKTEX_PATH_TEXMF_PLACEHOLDER_NO_MPM;
  searchSpec += MIKTEX_PATH_DIRECTORY_DELIMITER_STRING;
  searchSpec += MIKTEX_PATH_DOC_DIR;
  searchSpec += MIKTEX_PATH_RECURSION_INDICATOR;
  for (Tokenizer ext(extensions.c_str(), PATH_DELIMITER_STRING); ext.GetCurrent() != nullptr; ++ext)
  {
    PathName fileName(PathName(), name, ext.GetCurrent());
    PathName path;
    if (session->FindFile(fileName.Get(), searchSpec.c_str(), path))
    {
      files.push_back(path.Get());
    }
  }
}

void MiKTeXHelp::PrintFiles(const vector<string> & files)
{
  for (const string & fileName : files)
  {
    cout << fileName << endl;
  }
}

void MiKTeXHelp::ViewFile(const PathName & fileName)
{
  string viewer;

  const char * lpszExt = fileName.GetExtension();

  if (lpszExt != nullptr)
  {
    string env = "MIKTEX_VIEW_";
    env += (lpszExt + 1);
    if (!Utils::GetEnvironmentString(env.c_str(), viewer))
    {
      viewer = "";
    }
  }

#if defined(MIKTEX_WINDOWS)
  if (viewer.empty())
  {
    wchar_t szExecutable[BufferSizes::MaxPath];
    PathName directory(fileName);
    HINSTANCE hInst = FindExecutableW(fileName.ToWideCharString().c_str(), L"C:\\", szExecutable);
    if (hInst >= reinterpret_cast<HINSTANCE>(32))
    {
      viewer = StringUtil::WideCharToUTF8(szExecutable);
      if (printOnly)
      {
        cout << Q_(szExecutable) << ' ' << Q_(fileName) << endl;
      }
      else
      {
        Process::Start(szExecutable, Q_(fileName));
      }
      return;
    }
    else if (hInst != reinterpret_cast<HINSTANCE>(SE_ERR_NOASSOC))
    {
      FatalError(T_("The viewer could not be started."));
    }
  }
#else
  if (viewer.empty())
  {
    string pager;
    if (!Utils::GetEnvironmentString("PAGER", pager))
    {
      pager = "more";
    }
    if (lpszExt == 0)
    {
      viewer = pager + " %f";
    }
    else if (PathName::Compare(lpszExt, ".dvi") == 0)
    {
      viewer = "xdvi %f &";
    }
    else if (PathName::Compare(lpszExt, ".pdf") == 0)
    {
      viewer = "xpdf %f &";
    }
    else if (PathName::Compare(lpszExt, ".ps") == 0)
    {
      viewer = "gv %f &";
    }
    else if (PathName::Compare(lpszExt, ".html") == 0)
    {
      viewer = "firefox %f &";
    }
    else if (PathName::Compare(lpszExt, ".txt") == 0)
    {
      viewer = pager + " %f";
    }
  }
#endif

  if (viewer.empty())
  {
    FatalError(T_("No viewer associated with file type."));
  }

  string commandLine = viewer;

  string::size_type pos = 0;

  while ((pos = commandLine.find("%f", pos)) != string::npos)
  {
    commandLine.replace(pos, 2, fileName.Get());
    pos += fileName.GetLength();
  }

  if (printOnly)
  {
    cout << commandLine << endl;
  }
  else
  {
    bool runDetached = true;
    if (runDetached)
    {
      Process::StartSystemCommand(commandLine);
    }
    else
    {
      Process::ExecuteSystemCommand(commandLine);
    }
  }
}

void MiKTeXHelp::CreateHtmlAndView(const char * lpszPackageName, const vector<string> & files)
{
  PackageInfo pi = pManager->GetPackageInfo(lpszPackageName);
  PathName fileName = session->GetSpecialPath(SpecialPath::DataRoot);
  fileName /= MIKTEX_PATH_MIKTEX_MTHELP_DIR;
  Directory::Create(fileName);
  fileName /= lpszPackageName;
  fileName.SetExtension(".html");
  StreamWriter writer(fileName);
  int idx = 0;
  for (const char * lpsz = reinterpret_cast<const char *>(templateHtml); idx < sizeof(templateHtml); ++lpsz, ++idx)
  {
    if (*lpsz == '%')
    {
      string tag;
      ++lpsz;
      ++idx;
      while (idx < sizeof(templateHtml) && *lpsz != '%')
      {
        tag += *lpsz;
        ++lpsz;
        ++idx;
      }
      if (tag == "NAME")
      {
        WriteText(writer, pi.deploymentName);
      }
      else if (tag == "TITLE")
      {
        WriteText(writer, pi.title);
      }
      else if (tag == "DESCRIPTION")
      {
        WriteText(writer, pi.description);
      }
      else if (tag == "TRURL")
      {
        for (const string & fileName : files)
        {
          writer.WriteFormattedLine("<tr><td><a href=\"file://%s\">%s</td></tr>", fileName.c_str(), fileName.c_str());
        }
      }
    }
    else
    {
      writer.Write(*lpsz);
    }
  }
  writer.Close();
  ViewFile(fileName);
}

void MiKTeXHelp::WriteText(StreamWriter & writer, const string & text)
{
  for (string::const_iterator it = text.begin(); it != text.end(); ++it)
  {
    switch (*it)
    {
    case '<':
      writer.Write("&lt;");
      break;
    case '>':
      writer.Write("&gt;");
      break;
    case '&':
      writer.Write(T_("&amp;"));
      break;
    default:
      writer.Write(*it);
      break;
    }
  }
}

void MiKTeXHelp::Run(int argc, const char ** argv)
{
  bool optListOnly = false;
  bool optView = false;

  PoptWrapper popt(argc, argv, aoption);

  popt.SetOtherOptionHelp(T_("[OPTION...] NAME..."));

  int option;
  while ((option = popt.GetNextOpt()) >= 0)
  {
    switch (option)
    {
    case OPT_LIST_ONLY:
      optListOnly = true;
      break;
    case OPT_PRINT_ONLY:
      printOnly = true;
      break;
    case OPT_QUIET:
      quiet = true;
      break;
    case OPT_VERSION:
      ShowVersion();
      throw (0);
    case OPT_VIEW:
      optView = true;
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
    FatalError(T_("Missing NAME argument."));
  }

  for (const string & name : leftovers)
  {
    vector<string> filesByPackage;
    FindDocFilesByPackage(name.c_str(), filesByPackage);
    vector<string> filesByName;
    FindDocFilesByName(name.c_str(), filesByName);
    if (filesByPackage.size() + filesByName.size() == 0)
    {
      Warning(T_("Documentation for %s could not be found."), Q_(name));
      continue;
    }
    if (optListOnly)
    {
      PrintFiles(filesByPackage);
      PrintFiles(filesByName);
    }
    else if (optView || filesByPackage.size() == 0)
    {
      if (filesByPackage.size() > 0)
      {
        ViewFile(filesByPackage[0]);
      }
      else
      {
        ViewFile(filesByName[0]);
      }
    }
    else
    {
      if (filesByPackage.size() == 0)
      {
        Warning(T_("Documentation for package %s could not be found."), Q_(name));
        continue;
      }
      CreateHtmlAndView(name.c_str(), filesByPackage);
    }
  }
}

extern "C" MIKTEXDLLEXPORT int MIKTEXCEECALL mthelp(int argc, const char ** argv)
{
  try
  {
    MiKTeXHelp app;
    app.Init(Session::InitInfo(argv[0]));
    app.EnableInstaller(TriState::False);
    app.Run(argc, argv);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    Application::Sorry(TheNameOfTheGame, e);
    return 1;
  }
  catch (const exception & e)
  {
    Application::Sorry(TheNameOfTheGame, e);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
