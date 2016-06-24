/* webapp.cpp:

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

#include <miktex/Version>

void WebApp::Init(const char * lpszProgramInvocationName)
{
  Application::Init(lpszProgramInvocationName, TheNameOfTheGame());
  theNameOfTheGame = Utils::GetExeName();
  enable8BitChars = false;
}

void WebApp::Finalize()
{
  if (!packageListFileName.Empty())
  {
    FileStream stream(File::Open(packageListFileName, FileMode::Create, FileAccess::Write));
    vector<FileInfoRecord> fileInfoRecords = session->GetFileInfoRecords();
    set<string> packages;
    for (vector<FileInfoRecord>::const_iterator it = fileInfoRecords.begin(); it != fileInfoRecords.end(); ++it)
    {
      if (!it->packageName.empty())
      {
	packages.insert(it->packageName);
      }
    }
    for (set<string>::const_iterator it2 = packages.begin(); it2 != packages.end(); ++it2)
    {
      fprintf(stream.Get(), "%s\n", it2->c_str());
    }
    stream.Close();
  }
  features.Reset();
  copyright = "";
  packageListFileName = "";
  programName = "";
  tcxFileName = "";
  trademarks = "";
  version = "";
  options.clear();
  theNameOfTheGame = "";
  Application::Finalize();
}

void WebApp::ShowHelp(bool usageOnly) const
{
  if (options.size() == 0 || usageOnly || popt == nullptr)
  {
    return;
  }
  if (usageOnly)
  {
    popt.PrintUsage();
  }
  else
  {
    popt.PrintHelp();
  }
}

void WebApp::BadUsage() const
{
  cerr << T_("Invalid command-line. Try this:\n") << Utils::GetExeName() << " -help" << endl;
  throw (1);
}

void WebApp::AddOption(const char * lpszNameAndHelp, int val, int argInfo, const char * lpszArgDescription, void * pArg, char shortName)
{
#if defined(MIKTEX_DEBUG)
  if (lpszNameAndHelp != nullptr)
  {
    for (vector<poptOption>::const_iterator it = options.begin(); it != options.end(); ++it)
    {
      if (Utils::Equals(lpszNameAndHelp, it->longName))
      {
	MIKTEX_UNEXPECTED();
      }
    }
  }
#endif
  poptOption opt;
  memset(&opt, 0, sizeof(opt));
  opt.longName = lpszNameAndHelp;
  opt.shortName = shortName;
  opt.argInfo = argInfo | POPT_ARGFLAG_ONEDASH;
  if (val == OPT_UNSUPPORTED || val == OPT_NOOP)
  {
    opt.argInfo |= POPT_ARGFLAG_DOC_HIDDEN;
  }
  opt.arg = pArg;
  opt.val = val;
  if (lpszNameAndHelp != nullptr
    && val != OPT_UNSUPPORTED
    && val != OPT_NOOP
    && !(argInfo & POPT_ARGFLAG_DOC_HIDDEN))
  {
    opt.descrip = lpszNameAndHelp + StrLen(lpszNameAndHelp) + 1;
  }
  else
  {
    opt.descrip = nullptr;
  }
  opt.argDescrip = lpszArgDescription;
  options.push_back(opt);
}

void WebApp::AddOption(const char * lpszAliasName, const char * lpszName)
{
  for (vector<poptOption>::const_iterator it = options.begin(); it != options.end(); ++it)
  {
    if (Utils::Equals(lpszName, it->longName))
    {
      poptOption opt = *it;
      opt.longName = lpszAliasName;
      opt.argInfo |= POPT_ARGFLAG_DOC_HIDDEN;
      options.push_back(opt);
      return;
    }
  }
  MIKTEX_UNEXPECTED();
}

enum {
  OPT_ALIAS,
  OPT_DISABLE_INSTALLER,
  OPT_ENABLE_INSTALLER,
  OPT_HELP,
  OPT_HHELP,
  OPT_INCLUDE_DIRECTORY,
  OPT_RECORD_PACKAGE_USAGES,
  OPT_TRACE,
  OPT_VERBOSE,
  OPT_VERSION,
};

void WebApp::AddOptions()
{
  options.reserve(50);
  optBase = static_cast<int>(GetOptions().size());
  AddOption(T_("alias\0Pretend to be APP.  This affects both the format used and the search path."), FIRST_OPTION_VAL + optBase + OPT_ALIAS, POPT_ARG_STRING, T_("APP"));
  AddOption(T_("disable-installer\0Disable the package installer.  Missing files will not be installed."), FIRST_OPTION_VAL + optBase + OPT_DISABLE_INSTALLER);
  AddOption(T_("enable-installer\0Enable the package installer.  Missing files will be installed."), FIRST_OPTION_VAL + optBase + OPT_ENABLE_INSTALLER);
  AddOption(T_("help\0Show this help screen and exit."), FIRST_OPTION_VAL + optBase + OPT_HELP);
  AddOption(T_("include-directory\0Prefix DIR to the input search path."), FIRST_OPTION_VAL + optBase + OPT_INCLUDE_DIRECTORY, POPT_ARG_STRING, T_("DIR"));
  AddOption(T_("kpathsea-debug\0"), OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption(T_("record-package-usages\0Enable the package usage recorder.  Output is written to FILE."), FIRST_OPTION_VAL + optBase + OPT_RECORD_PACKAGE_USAGES, POPT_ARG_STRING, T_("FILE"));
  AddOption(T_("trace\0Turn tracing on.  OPTIONS must be a comma-separated list of trace options.   See the manual, for more information."), FIRST_OPTION_VAL + optBase + OPT_TRACE, POPT_ARG_STRING, T_("OPTIONS"));
  AddOption("verbose\0", OPT_UNSUPPORTED);
  AddOption(T_("version\0Print version information and exit."), FIRST_OPTION_VAL + optBase + OPT_VERSION);
#if defined(MIKTEX_WINDOWS)
  if (GetHelpId() > 0)
  {
    AddOption(T_("hhelp\0Show the manual page in an HTMLHelp window and exit when the window is closed."), FIRST_OPTION_VAL + optBase + OPT_HHELP);
  }
#endif
}

bool WebApp::ProcessOption(int opt, const string & optArg)
{
  if (opt == OPT_UNSUPPORTED)
  {
    MIKTEX_UNEXPECTED();
  }
  else if (opt == OPT_NOOP)
  {
    return true;
  }
  bool done = true;
  switch (opt - FIRST_OPTION_VAL - optBase)
  {
  case OPT_ALIAS:
    session->PushAppName(optArg);
    break;
  case OPT_DISABLE_INSTALLER:
    EnableInstaller(TriState::False);
    break;
  case OPT_ENABLE_INSTALLER:
    EnableInstaller(TriState::True);
    break;
  case OPT_INCLUDE_DIRECTORY:
    if (Directory::Exists(optArg))
    {
      PathName path(optArg);
      path.MakeAbsolute();
      session->AddInputDirectory(path.Get(), true);
    }
    break;
  case OPT_HELP:
    ShowHelp();
    throw (0);
#if defined(MIKTEX_WINDOWS)
  case OPT_HHELP:
    MIKTEX_ASSERT(GetHelpId() > 0);
    session->ShowManualPageAndWait(0, GetHelpId());
    throw 0;
#endif
  case OPT_RECORD_PACKAGE_USAGES:
    session->StartFileInfoRecorder(true);
    packageListFileName = optArg;
    break;
  case OPT_TRACE:
    MiKTeX::Trace::TraceStream::SetTraceFlags(optArg.c_str());
    break;
  case OPT_VERSION:
    ShowProgramVersion();
    throw 0;
  default:
    done = false;
    break;
  }
  return done;
}

inline bool operator< (const poptOption & opt1, const poptOption & opt2)
{
  MIKTEX_ASSERT(opt1.longName != nullptr);
  MIKTEX_ASSERT(opt2.longName != nullptr);
  return StringCompare(opt1.longName, opt2.longName, false) < 0;
}

void WebApp::ProcessCommandLineOptions()
{
  int argc = C4P::GetArgC();
  const char ** argv = C4P::GetArgV();

  if (options.size() == 0)
  {
    AddOptions();
    sort(options.begin(), options.end());
    AddOption(nullptr, 0);
  }

  popt.Construct(argc, argv, &options[0]);
  popt.SetOtherOptionHelp(GetUsage());

  int opt;

  while ((opt = popt.GetNextOpt()) >= 0)
  {
    if (!ProcessOption(opt, popt.GetOptArg()))
    {
      MIKTEX_UNEXPECTED();
    }
  }

  if (opt != -1)
  {
    MIKTEX_FATAL_ERROR_2(T_("The command line options could not be processed."), "optionError", popt.Strerror(opt));
  }

  C4P::MakeCommandLine(popt.GetLeftovers());
}

const char * WebApp::TheNameOfTheGame() const
{
  return theNameOfTheGame.c_str();
}

void WebApp::ShowProgramVersion() const
{
  cout << "MiKTeX" << '-' << TheNameOfTheGame() << ' ' << version << " (" << Utils::GetMiKTeXBannerString() << ')' << endl
    << copyright << endl;
  if (!trademarks.empty())
  {
    cout << trademarks << endl;
  }
  cout << flush;
  ShowLibraryVersions();
}

void WebApp::SetProgramInfo(const char * lpszProgramName, const char * lpszVersion, const char * lpszCopyright, const char * lpszTrademarks)
{
  if (lpszProgramName != nullptr)
  {
    programName = lpszProgramName;
  }
  if (lpszVersion != nullptr)
  {
    version = lpszVersion;
  }
  if (lpszCopyright != nullptr)
  {
    copyright = lpszCopyright;
  }
  if (lpszTrademarks != nullptr)
  {
    trademarks = lpszTrademarks;
  }
}
