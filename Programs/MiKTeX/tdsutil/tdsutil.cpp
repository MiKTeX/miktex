/* tdsutil.cpp: MiKTeX TDS Utility

   Copyright (C) 2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.
   
   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

#include "StdAfx.h"

#include "internal.h"

#include "Recipe.h"

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

const char * const TheNameOfTheGame = T_("MiKTeX TDS Utility");

class TdsUtility :
  public Application
{
public:
  void Run(int argc, const char ** argv);

private:
  MIKTEXNORETURN void Error(const char * lpszFormat, ...);

private:
  bool verbose = false;

private:
  bool printOnly = false;

private:
  static struct poptOption aoption[];
};

enum Option
{
  OPT_AAA = 1000,
  OPT_DEST_DIR,
  OPT_PRINT_ONLY,
  OPT_RECIPE,
  OPT_SOURCE,
  OPT_VERBOSE,
  OPT_VERSION
};

struct poptOption TdsUtility::aoption[] = {

  {
    "dest-dir", 0,
    POPT_ARG_STRING, nullptr,
    OPT_DEST_DIR,
    T_("The destination directory."),
    nullptr
  },

  {
    "print-only", 'n',
    POPT_ARG_NONE, nullptr,
    OPT_PRINT_ONLY,
    T_("Print what would be done."),
    nullptr
  },

  {
    "recipe", 0,
    POPT_ARG_STRING, nullptr,
    OPT_RECIPE,
    T_("The recipe for package installation."),
    nullptr
  },

  {
    "source", 0,
    POPT_ARG_STRING, nullptr,
    OPT_SOURCE,
    T_("The source directory or file."),
    nullptr
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

MIKTEXNORETURN void TdsUtility::Error(const char * lpszFormat, ...)
{
  va_list arglist;
  VA_START(arglist, lpszFormat);
  cerr << "tdsutil: " << StringUtil::FormatString(lpszFormat, arglist) << endl;
  VA_END(arglist);
  throw 1;
}

void TdsUtility::Run(int argc, const char ** argv)
{
  PoptWrapper popt(argc, argv, aoption);
  popt.SetOtherOptionHelp("install <package>");

  int option;
  vector<PathName> optionRecipeFiles;
  PathName source;
  PathName destDir;

  Session::InitInfo initInfo(argv[0]);

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {
    case OPT_DEST_DIR:
      destDir = optArg;
      break;
    case OPT_PRINT_ONLY:
      printOnly = true;
      break;
    case OPT_RECIPE:
      optionRecipeFiles.push_back(optArg);
      break;
    case OPT_SOURCE:
      source = optArg;
      break;
    case OPT_VERBOSE:
      verbose = true;
      break;
    case OPT_VERSION:
      cout
        << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_COMP_J2000_VERSION, 0)) << endl
        << "Copyright (C) 2016 Christian Schenk" << endl
        << "This is free software; see the source for copying conditions.  There is NO" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
      return;
    }
  }
  
  Init(initInfo);

  vector<string> leftovers = popt.GetLeftovers();

  if (leftovers.empty())
  {
    Error("Nothing to do?\nTry '%s --help' for more information.", argv[0]);
  }

  if (leftovers[0] == "install")
  {
    if (leftovers.size() != 2)
    {
      Error("Usage: %s install <package>", argv[0]);
    }
    string package = leftovers[1];
    if (source.Empty())
    {
      source.SetToCurrentDirectory();
    }
    if (destDir.Empty())
    {
      // TODO: home texmf
      destDir = session->GetSpecialPath(SpecialPath::UserDataRoot);
    }
    Recipe recipe(package, source, destDir, verbose);
    vector<PathName> recipeFiles;
    if (optionRecipeFiles.empty())
    {
      string packageRecipeFile = package + MIKTEX_TDSUTIL_RECIPE_FILE_SUFFIX;
      session->FindFile(MIKTEX_PATH_TDSUTIL_DEFAULT_RECIPE, MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, recipeFiles);
      session->FindFile(packageRecipeFile.c_str(), MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_TDSUTIL_RECIPES_DIR, { Session::FindFileOption::All }, recipeFiles);
    }
    recipeFiles.insert(recipeFiles.end(), optionRecipeFiles.begin(), optionRecipeFiles.end());
    for (const PathName & recipeFile : recipeFiles)
    {
      recipe.Read(recipeFile);
    }
    recipe.Execute(printOnly);
  }
  else
  {
    Error("Unknown command: %s", leftovers[0].c_str());
  }

  Finalize();
}

#if defined(_UNICODE)
#  define MAIN wmain
#  define MAINCHAR wchar_t
#else
#  define MAIN main
#  define MAINCHAR char
#endif

int MAIN(int argc, MAINCHAR * argv[])
{
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char *> newargv;
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
    TdsUtility app;
    app.Run(argc, &newargv[0]);
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
