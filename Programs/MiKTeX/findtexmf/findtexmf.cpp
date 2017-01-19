/* findtexmf.cpp: finding TeXMF related files

   Copyright (C) 2001-2017 Christian Schenk

   This file is part of FindTeXMF.

   FindTeXMF is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   FindTeXMF is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FindTeXMF; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(MIKTEX_WINDOWS)
#  include <Windows.h>
#endif

#include <map>
#include <string>
#include <vector>

#include <iomanip>
#include <iostream>

#include "findtexmf-version.h"

#include <miktex/App/Application>
#include <miktex/Core/CsvList>
#include <miktex/Core/Exceptions>
#include <miktex/Core/FileType>
#include <miktex/Core/Paths>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/Wrappers/PoptWrapper>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Wrappers;
using namespace MiKTeX::Util;
using namespace std;

#define WU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

const char * const TheNameOfTheGame = T_("MiKTeX Find Utility");
const char * const PROGNAME = "findtexmf";

class FindTeXMF :
  public Application
{
private:
  void ShowVersion();

private:
  void ListFileTypes();

private:
  void PrintSearchPath(const char * lpszSearchPath);

public:
  void Run(int argc, const char ** argv);

private:
  bool mustExist = false;

private:
  bool start = false;

private:
  FileType fileType = FileType::None;

private:
  shared_ptr<Session> session;

private:
  static const struct poptOption aoption[];
};

enum Option
{
  OPT_AAA = 256,
  OPT_ALIAS,
  OPT_EXPAND_PATH,
  OPT_EXPAND_VAR,
  OPT_FILE_TYPE,
  OPT_LIST_FILE_TYPES,
  OPT_MUST_EXIST,
  OPT_SHOW_PATH,
  OPT_START,
  OPT_THE_NAME_OF_THE_GAME,
  OPT_VERSION,
};

const struct poptOption FindTeXMF::aoption[] =
{
  {
    "alias", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_ALIAS,
    T_("Pretend to be APP, i.e., use APP's configuration settings when searching for files."),
    T_("APP")
  },

  {
    "engine", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_THE_NAME_OF_THE_GAME,
    T_("Set the name of the engine. Relevant when searching for format files."),
    T_("NAME")
  },

  {
    "file-type", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_FILE_TYPE,
    T_("The type of the file to search for."),
    T_("FILETYPE"),
  },

  {
    "list-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_LIST_FILE_TYPES,
    T_("List known file types."),
    nullptr
  },

  {
    "must-exist", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_MUST_EXIST,
    T_("Run the package installer, if necessary."),
    nullptr
  },

  {
    "show-path", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_SHOW_PATH,
    T_("Show the search path for a certain file type."),
    T_("FILETYPE")
  },

  {
    "start", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_START,
    T_("Start the program which is associated with the file name extension."),
    nullptr
  },

  {
    "the-name-of-the-game", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_THE_NAME_OF_THE_GAME,
    T_("Set the name of the program. Relevant when searching for format files."),
    T_("NAME")
  },

  {
    "version", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

void FindTeXMF::ShowVersion()
{
  cout
    << Utils::MakeProgramVersionString(TheNameOfTheGame, MIKTEX_COMPONENT_VERSION_STR) << endl
    << "Copyright (C) 2001-2016 Christian Schenk" << endl
    << "This is free software; see the source for copying conditions.  There is NO" << endl
    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

void FindTeXMF::ListFileTypes()
{
  FileTypeInfo fti;
  for (const FileTypeInfo & fti : session->GetFileTypes())
  {
    if (fti.fileNameExtensions.empty())
    {
      continue;
    }
    cout << "  " << fti.fileTypeString << " (" << fti.fileNameExtensions << ")" << endl;
  }
}

void FindTeXMF::PrintSearchPath(const char * lpszSearchPath)
{
  bool first = true;
  PathName mpmRootPath = session->GetMpmRootPath();
  size_t mpmRootPathLen = mpmRootPath.GetLength();
  for (CsvList path(lpszSearchPath, ';'); path; ++path)
  {
    const char * lpszPath = (*path).c_str();
    if ((PathName::Compare(lpszPath, mpmRootPath, mpmRootPathLen) == 0)
      && (lpszPath[mpmRootPathLen] == 0 || IsDirectoryDelimiter(lpszPath[mpmRootPathLen])))
    {
      continue;
    }
    if (first)
    {
      first = false;
    }
    else
    {
      cout << ";";
    }
    cout << session->Expand(lpszPath, { ExpandOption::Braces }, nullptr);
  }
  cout << endl;
}

void FindTeXMF::Run(int argc, const char ** argv)
{
  session = GetSession();

  bool needArg = true;

  PoptWrapper popt(argc, argv, aoption);

  int option;
  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {

    case OPT_ALIAS:

      session->PushAppName(optArg);
      break;

    case OPT_EXPAND_VAR:

      cout << session->Expand(optArg, { ExpandOption::Values }, nullptr) << endl;
      needArg = false;
      break;

    case OPT_EXPAND_PATH:

      cout << session->Expand(optArg, { ExpandOption::Values, ExpandOption::Braces, ExpandOption::PathPatterns }, nullptr) << endl;
      needArg = false;
      break;

    case OPT_FILE_TYPE:

      fileType = session->DeriveFileType(optArg);
      if (fileType == FileType::None)
      {
        FatalError(T_("Unknown file type: %s."), optArg.c_str());
      }
      break;

    case OPT_LIST_FILE_TYPES:

      ListFileTypes();
      needArg = false;
      break;

    case OPT_MUST_EXIST:

      mustExist = true;
      break;

    case OPT_SHOW_PATH:

    {
      FileType filetype = session->DeriveFileType(optArg);
      if (filetype == FileType::None)
      {
        FatalError(T_("Unknown file type: %s."), optArg.c_str());
      }
      string searchPath = session->GetExpandedSearchPath(filetype);
      if (!searchPath.empty())
      {
        PrintSearchPath(searchPath.c_str());
      }
      needArg = false;
      break;
    }

    case OPT_START:

      start = true;
      break;

    case OPT_THE_NAME_OF_THE_GAME:

      session->SetTheNameOfTheGame(optArg);
      break;

    case OPT_VERSION:

      ShowVersion();
      throw 0;
    }
  }

  if (option != -1)
  {
    string msg = popt.BadOption(POPT_BADOPTION_NOALIAS);
    msg += ": ";
    msg += popt.Strerror(option);
    FatalError("%s", msg.c_str());
  }

  EnableInstaller(mustExist ? TriState::True : TriState::False);

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    if (!needArg)
    {
      return;
    }
    else
    {
      FatalError(T_("Missing argument. Try 'findtexmf --help'."));
    }
  }

  for (const string & fileName : leftovers)
  {
    PathName path;
    FileType filetype = fileType;
    if (filetype == FileType::None)
    {
      filetype = session->DeriveFileType(fileName);
      if (filetype == FileType::None)
      {
        filetype = FileType::TEX;
      }
    }
    bool found = session->FindFile(fileName, filetype, path);
    if (found)
    {
      cout << path << endl;
      if (start)
      {
#if defined(MIKTEX_WINDOWS)
        PathName pathDir(path);
        pathDir.RemoveFileSpec();
        if (ShellExecuteW(nullptr, UW_("open"), path.ToWideCharString().c_str(), nullptr, pathDir.ToWideCharString().c_str(), SW_SHOW) <= reinterpret_cast<HINSTANCE>(32))
        {
          FatalError(T_("%s could not be started."), Q_(fileName));
        }
#else
        // TODO: start program
        UNIMPLEMENTED();
#endif
      }
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
    FindTeXMF app;
    app.Init(newargv[0]);
    app.Run(argc, &newargv[0]);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException& ex)
  {
    Application::Sorry(TheNameOfTheGame, ex);
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
