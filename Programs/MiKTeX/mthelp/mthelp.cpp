/* mthelp.cpp:

   Copyright (C) 2004-2020 Christian Schenk

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
#include <miktex/Core/ConfigNames>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Core/StreamWriter>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Wrappers/PoptWrapper>

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#include "template.html.h"

#define T_(x) MIKTEXTEXT(x)
#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

const char* const TheNameOfTheGame = T_("MiKTeX Help Utility");

class MiKTeXHelp :
  public Application
{
public:
  void MIKTEXTHISCALL Init(const Session::InitInfo& initInfo, vector<char*>&args) override;

public:
  void Run(int argc, const char** argv);

private:
  void ShowVersion();

private:
  void Warning(const string& msg);

private:
  bool SkipPrefix(const string& str, const char* lpszPrefix, string& result);

private:
  bool SkipTeXMFPrefix(const string& str, string& result);

private:
  void FindDocFilesByPackage(const string& packageName, map<string, vector<string>>& filesByExtension);

private:
  void FindDocFilesByPackage(const string& packageName, vector<string>& files);

private:
  void FindDocFilesByName(const string& name, vector<string>& files);

private:
  void PrintFiles(const vector<string>& files);

private:
  void ViewFile(const PathName& fileName);

private:
  void CreateHtmlAndView(const char* lpszPackageName, const vector<string>& files);

private:
  void WriteText(StreamWriter& writer, const string& text);

private:
  bool printOnly = false;

private:
  bool quiet = false;

private:
  shared_ptr<Session> session;

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

void MiKTeXHelp::Init(const Session::InitInfo& initInfo, vector<char*>& args)
{
  Application::Init(initInfo, args);
  session = GetSession();
  pManager = PackageManager::Create();
}

void MiKTeXHelp::ShowVersion()
{
  cout << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << endl
       << endl
       << MIKTEX_COMP_COPYRIGHT_STR << endl
       << endl
       << "This is free software; see the source for copying conditions.  There is NO" << endl
       << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

void MiKTeXHelp::Warning(const string& msg)
{
  if (quiet)
  {
    return;
  }
  cerr << msg << endl;
}

bool MiKTeXHelp::SkipPrefix(const string& str, const char* lpszPrefix, string& result)
{
  size_t n = StrLen(lpszPrefix);
  if (str.compare(0, n, lpszPrefix) != 0)
  {
    return false;
  }
  result = str.c_str() + n;
  return true;
}

bool MiKTeXHelp::SkipTeXMFPrefix(const string& str, string& result)
{
  return SkipPrefix(str, "texmf/", result)
    || SkipPrefix(str, "texmf\\", result)
    || SkipPrefix(str, "./texmf/", result)
    || SkipPrefix(str, ".\\texmf\\", result);
}

void MiKTeXHelp::FindDocFilesByPackage(const string& packageName, map<string, vector<string>>& filesByExtension)
{
  PackageInfo pi;
  if (!pManager->TryGetPackageInfo(packageName, pi))
  {
    return;
  }
  for (const string& fileName : pi.docFiles)
  {
    string extension = PathName(fileName).GetExtension();
    string file;
    PathName path;
    if (SkipTeXMFPrefix(fileName, file) && session->FindFile(file, MIKTEX_PATH_TEXMF_PLACEHOLDER_NO_MPM, path))
    {
      vector<string>& files = filesByExtension[extension];
      files.push_back(path.ToString());
    }
  }
}

void MiKTeXHelp::FindDocFilesByPackage(const string& packageName, vector<string>& files)
{
  map<string, vector<string>> filesByExtension;
  FindDocFilesByPackage(packageName, filesByExtension);
  vector<string> extensions = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE_FILETYPES + ".TeX system documentation"s, MIKTEX_CONFIG_VALUE_EXTENSIONS).GetStringArray();
  for (const string& ext : extensions)
  {
    vector<string>& vec = filesByExtension[ext];
    vector<string>::iterator it = vec.begin();
    while (it != vec.end())
    {
      PathName name = PathName(*it).GetFileNameWithoutExtension();
      if (PathName::Compare(name, PathName(packageName)) == 0)
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
  for (const string& ext : extensions)
  {
    vector<string>& vec = filesByExtension[ext];
    files.insert(files.end(), vec.begin(), vec.end());
  }
}

void MiKTeXHelp::FindDocFilesByName(const string& name, vector<string>& files)
{
  vector<string> extensions = session->GetConfigValue(MIKTEX_CONFIG_SECTION_CORE_FILETYPES + ".TeX system documentation"s, MIKTEX_CONFIG_VALUE_EXTENSIONS).GetStringArray();
  string searchSpec = MIKTEX_PATH_TEXMF_PLACEHOLDER_NO_MPM;
  searchSpec += MIKTEX_PATH_DIRECTORY_DELIMITER_STRING;
  searchSpec += MIKTEX_PATH_DOC_DIR;
  searchSpec += MIKTEX_PATH_RECURSION_INDICATOR;
  for (const string& ext : extensions)
  {
    PathName fileName(name);
    fileName.AppendExtension(ext);
    PathName path;
    if (session->FindFile(fileName.GetData(), searchSpec, path))
    {
      files.push_back(path.GetData());
    }
  }
}

void MiKTeXHelp::PrintFiles(const vector<string>& files)
{
  for (const string& fileName : files)
  {
    cout << fileName << endl;
  }
}

void MiKTeXHelp::ViewFile(const PathName& fileName)
{
  string viewer;

  string ext = fileName.GetExtension();

  if (!ext.empty())
  {
    string env = "MIKTEX_VIEW_";
    env += ext.substr(1);
    if (!Utils::GetEnvironmentString(env, viewer))
    {
      viewer = "";
    }
  }

#if defined(MIKTEX_WINDOWS)
  if (viewer.empty())
  {
    wchar_t szExecutable[BufferSizes::MaxPath];
    HINSTANCE hInst = FindExecutableW(fileName.ToWideCharString().c_str(), L"C:\\", szExecutable);
    if (hInst >= reinterpret_cast<HINSTANCE>(32))
    {
      viewer = StringUtil::WideCharToUTF8(szExecutable);
      if (printOnly)
      {
        cout << Q_(PathName(szExecutable)) << ' ' << Q_(fileName) << endl;
      }
      else
      {
        Process::Start(PathName(szExecutable), { PathName(szExecutable).GetFileNameWithoutExtension().ToString(), fileName.ToString() });
      }
      return;
    }
    else if (hInst != reinterpret_cast<HINSTANCE>(SE_ERR_NOASSOC))
    {
      FatalError(T_("The viewer could not be started."));
    }
  }
#elif defined(MIKTEX_MACOS_BUNDLE)
  if (viewer.empty())
  {
    if (printOnly)
    {
      cout << "open " << Q_(fileName) << endl;
    }
    else
    {
      Process::ExecuteSystemCommand(fmt::format("open {0}", Q_(fileName)));
    }
    return;
  }
#else
  if (viewer.empty())
  {
    string pager;
    if (!Utils::GetEnvironmentString("PAGER", pager))
    {
      pager = "more";
    }
    if (ext.empty())
    {
      viewer = pager + " %f";
    }
    else if (PathName::Compare(ext, ".dvi") == 0)
    {
      viewer = "xdvi %f &";
    }
    else if (PathName::Compare(ext, ".pdf") == 0)
    {
      viewer = "xpdf %f &";
    }
    else if (PathName::Compare(ext, ".ps") == 0)
    {
      viewer = "gv %f &";
    }
    else if (PathName::Compare(ext, ".html") == 0)
    {
      viewer = "firefox %f &";
    }
    else if (PathName::Compare(ext, ".txt") == 0)
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
    commandLine.replace(pos, 2, fileName.GetData());
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

void MiKTeXHelp::CreateHtmlAndView(const char* lpszPackageName, const vector<string>& files)
{
  PackageInfo pi = pManager->GetPackageInfo(lpszPackageName);
  PathName fileName = session->GetSpecialPath(SpecialPath::DataRoot);
  fileName /= MIKTEX_PATH_MIKTEX_MTHELP_DIR;
  Directory::Create(fileName);
  fileName /= lpszPackageName;
  fileName.AppendExtension(".html");
  StreamWriter writer(fileName);
  int idx = 0;
  for (const char* lpsz = reinterpret_cast<const char*>(templateHtml); idx < sizeof(templateHtml); ++lpsz, ++idx)
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
        WriteText(writer, pi.id);
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
        for (const string& fileName : files)
        {
          writer.WriteLine(fmt::format("<tr><td><a href=\"file://{0}\">{0}</td></tr>", fileName));
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

void MiKTeXHelp::WriteText(StreamWriter& writer, const string& text)
{
  for (const char& ch :text)
  {
    switch (ch)
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
      writer.Write(ch);
      break;
    }
  }
}

void MiKTeXHelp::Run(int argc, const char** argv)
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
    FatalError(msg);
  }

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    FatalError(T_("Missing NAME argument."));
  }

  for (const string& name : leftovers)
  {
    vector<string> filesByPackage;
    FindDocFilesByPackage(name.c_str(), filesByPackage);
    vector<string> filesByName;
    FindDocFilesByName(name.c_str(), filesByName);
    if (filesByPackage.size() + filesByName.size() == 0)
    {
      Warning(fmt::format(T_("Documentation for {0} could not be found."), Q_(name)));
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
        ViewFile(PathName(filesByPackage[0]));
      }
      else
      {
        ViewFile(PathName(filesByName[0]));
      }
    }
    else
    {
      if (filesByPackage.size() == 0)
      {
        Warning(fmt::format(T_("Documentation for package {0} could not be found."), Q_(name)));
        continue;
      }
      CreateHtmlAndView(name.c_str(), filesByPackage);
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
  MiKTeXHelp app;
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
    app.Init(Session::InitInfo(newargv[0]), newargv);
    app.EnableInstaller(TriState::False);
    app.Run(newargv.size() - 1, const_cast<const char**>(&newargv[0]));
    app.Finalize2(0);
    return 0;
  }
  catch (const MiKTeXException& ex)
  {
    Application::Sorry(TheNameOfTheGame, ex);
    ex.Save();
    return 1;
  }
  catch (const exception& ex)
  {
    Application::Sorry(TheNameOfTheGame, ex);
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
