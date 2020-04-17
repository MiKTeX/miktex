/* webapp.cpp:

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

#include <set>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/FileStream>

#include "internal.h"
#include "texmf-version.h"

class WebApp::impl
{
public:
  impl() = default;
public:
  impl(const impl& other) = delete;
public:
  impl& operator=(const impl& other) = delete;
public:
  impl(impl&& other) = delete;
public:
  impl& operator=(impl&& other) = delete;
public:
  ~impl() noexcept
  {
    try
    {
      for (char* cstr : cstrings)
      {
        delete[] cstr;
      }
    }
    catch (const exception&)
    {
    }
  }
public:
  vector<char*> cstrings;
public:
  char* AddString(const string& s)
  {
    size_t l = s.length();
    char* cstr = new char[l + 1];
    memcpy(cstr, s.c_str(), l + 1);
    cstrings.push_back(cstr);
    return cstr;
  }
public:
  OptionSet<Feature> features;
public:
  PoptWrapper popt;
public:
  string copyright;
public:
  PathName packageListFileName;
public:
  string programName;
public:
  PathName tcxFileName;
public:
  bool enable8BitChars;
public:
  string trademarks;
public:
  string version;
public:
  vector<poptOption> options;
public:
  int optBase;
public:
  unordered_map<string, vector<string>> optionShortcuts;
public:
  ICharacterConverter* characterConverter = nullptr;
public:
  IInitFinalize* initFinalize = nullptr;
public:
  bool isTeXProgram;
public:
  bool isMETAFONTProgram;
public:
  bool verbose;
};

WebApp::WebApp() :
  pimpl(make_unique<impl>())
{
}

WebApp::~WebApp() noexcept
{
}

void WebApp::Init(vector<char*>& args)
{
  Session::InitInfo initInfo(args[0]);
  initInfo.SetTheNameOfTheGame(TheNameOfTheGame());
  Application::Init(initInfo, args);
  pimpl->enable8BitChars = false;
  pimpl->isTeXProgram = TheNameOfTheGame() == "TeX";
  pimpl->isMETAFONTProgram = TheNameOfTheGame() == "METAFONT";
}

void WebApp::Finalize()
{
  shared_ptr<Session> session = GetSession();
  if (!pimpl->packageListFileName.Empty())
  {
    ofstream stream = File::CreateOutputStream(pimpl->packageListFileName);
    vector<FileInfoRecord> fileInfoRecords = session->GetFileInfoRecords();
    set<string> packages;
    for (const FileInfoRecord& fir : fileInfoRecords)
    {
      if (!fir.packageName.empty())
      {
	packages.insert(fir.packageName);
      }
    }
    for (const string& pkg : packages)
    {
      stream << pkg << "\n";
    }
    stream.close();
  }
  pimpl->features.Reset();
  pimpl->copyright = "";
  pimpl->packageListFileName = "";
  pimpl->programName = "";
  pimpl->tcxFileName = "";
  pimpl->trademarks = "";
  pimpl->version = "";
  pimpl->options.clear();
  pimpl->optionShortcuts.clear();
  Application::Finalize();
}

void WebApp::ShowHelp(bool usageOnly) const
{
  if (pimpl->options.empty() || usageOnly || pimpl->popt == nullptr)
  {
    return;
  }
  if (usageOnly)
  {
    pimpl->popt.PrintUsage();
  }
  else
  {
    pimpl->popt.PrintHelp();
  }
}

void WebApp::BadUsage() const
{
  cerr << T_("Invalid command-line. Try this:\n") << Utils::GetExeName() << " -help" << endl;
  throw 1;
}

void WebApp::AddOption(const string& name, const string& help, int val, int argInfo, const string& argDescription, void* arg, char shortName)
{
  poptOption opt{};
  opt.longName = pimpl->AddString(name);
  opt.shortName = shortName;
  opt.argInfo = argInfo | POPT_ARGFLAG_ONEDASH;
  if (val == OPT_UNSUPPORTED || val == OPT_NOOP)
  {
    opt.argInfo |= POPT_ARGFLAG_DOC_HIDDEN;
  }
  opt.arg = arg;
  opt.val = val;
  if (!help.empty()
    && val != OPT_UNSUPPORTED
    && val != OPT_NOOP
    && !(argInfo & POPT_ARGFLAG_DOC_HIDDEN))
  {
    opt.descrip = pimpl->AddString(help);
  }
  else
  {
    opt.descrip = nullptr;
  }
  opt.argDescrip = argDescription.empty() ? nullptr : pimpl->AddString(argDescription);
  pimpl->options.push_back(opt);
}

void WebApp::AddOption(const string& aliasName, const string& name)
{
  AddOptionShortcut(aliasName, { "--" + name });
}

void WebApp::AddOptionShortcut(const std::string& longName, const std::vector<std::string>& args)
{
  pimpl->optionShortcuts[longName] = args;
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
  pimpl->options.reserve(50);
  pimpl->optBase = (int)GetOptions().size();
  AddOption(T_("alias\0Pretend to be APP.  This affects both the format used and the search path."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ALIAS, POPT_ARG_STRING, T_("APP"));
  AddOption(T_("disable-installer\0Disable the package installer.  Missing files will not be installed."), FIRST_OPTION_VAL + pimpl->optBase + OPT_DISABLE_INSTALLER);
  AddOption(T_("enable-installer\0Enable the package installer.  Missing files will be installed."), FIRST_OPTION_VAL + pimpl->optBase + OPT_ENABLE_INSTALLER);
  AddOption(T_("help\0Show this help screen and exit."), FIRST_OPTION_VAL + pimpl->optBase + OPT_HELP);
  AddOption(T_("include-directory\0Prefix DIR to the input search path."), FIRST_OPTION_VAL + pimpl->optBase + OPT_INCLUDE_DIRECTORY, POPT_ARG_STRING, T_("DIR"));
  AddOption(T_("kpathsea-debug\0"), OPT_UNSUPPORTED, POPT_ARG_STRING);
  AddOption(T_("record-package-usages\0Enable the package usage recorder.  Output is written to FILE."), FIRST_OPTION_VAL + pimpl->optBase + OPT_RECORD_PACKAGE_USAGES, POPT_ARG_STRING, T_("FILE"));
  AddOption(T_("trace\0Turn tracing on.  OPTIONS must be a comma-separated list of trace options.   See the manual, for more information."), FIRST_OPTION_VAL + pimpl->optBase + OPT_TRACE, POPT_ARG_STRING, T_("OPTIONS"));
  AddOption(T_("verbose\0Turn on verbose mode."), FIRST_OPTION_VAL + pimpl->optBase + OPT_VERBOSE);
  AddOption(T_("version\0Print version information and exit."), FIRST_OPTION_VAL + pimpl->optBase + OPT_VERSION);
#if defined(MIKTEX_WINDOWS)
  if (GetHelpId() > 0)
  {
    AddOption(T_("hhelp\0Show the manual page in an HTMLHelp window and exit when the window is closed."), FIRST_OPTION_VAL + pimpl->optBase + OPT_HHELP);
  }
#endif
}

bool WebApp::ProcessOption(int opt, const string& optArg)
{
  shared_ptr<Session> session = GetSession();
  if (opt == OPT_UNSUPPORTED)
  {
    MIKTEX_UNEXPECTED();
  }
  else if (opt == OPT_NOOP)
  {
    return true;
  }
  bool done = true;
  switch (opt - FIRST_OPTION_VAL - pimpl->optBase)
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
      session->AddInputDirectory(path, true);
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
    pimpl->packageListFileName = optArg;
    break;
  case OPT_TRACE:
    MiKTeX::Trace::TraceStream::SetOptions(optArg);
    break;
  case OPT_VERBOSE:
    pimpl->verbose = true;
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

inline bool operator< (const poptOption& opt1, const poptOption& opt2)
{
  MIKTEX_ASSERT(opt1.longName != nullptr);
  MIKTEX_ASSERT(opt2.longName != nullptr);
  return StringCompare(opt1.longName, opt2.longName, false) < 0;
}

void WebApp::ProcessCommandLineOptions()
{
  int argc = C4P::GetArgC();
  const char** argv = C4P::GetArgV();

  if (pimpl->options.empty())
  {
    AddOptions();
    sort(pimpl->options.begin(), pimpl->options.end());
    pimpl->options.push_back(poptOption{});
  }

  pimpl->popt.Construct(argc, argv, &pimpl->options[0]);
  for (auto shortcut : pimpl->optionShortcuts)
  {
    Argv argv(shortcut.second);
    pimpl->popt.AddAlias(shortcut.first.c_str(), 0, argv.GetArgc(), (const char**)argv.CloneFreeable());
  }
  pimpl->popt.SetOtherOptionHelp(GetUsage());

  int opt;

  while ((opt = pimpl->popt.GetNextOpt()) >= 0)
  {
    if (!ProcessOption(opt, pimpl->popt.GetOptArg()))
    {
      MIKTEX_UNEXPECTED();
    }
  }

  if (opt != -1)
  {
    MIKTEX_FATAL_ERROR_2(T_("The command line options could not be processed."), "optionError", pimpl->popt.Strerror(opt));
  }

  C4P::MakeCommandLine(pimpl->popt.GetLeftovers());
}

string WebApp::TheNameOfTheGame() const
{
  return pimpl->programName;
}

void WebApp::ShowProgramVersion() const
{
  cout << "MiKTeX" << '-' << TheNameOfTheGame() << ' ' << pimpl->version << " (" << Utils::GetMiKTeXBannerString() << ')' << endl
    << pimpl->copyright << endl;
  if (!pimpl->trademarks.empty())
  {
    cout << pimpl->trademarks << endl;
  }
  cout << flush;
  ShowLibraryVersions();
}

void WebApp::SetProgramInfo(const string& programName, const string& version, const string& copyright, const string& trademarks)
{
  pimpl->programName = programName;
  pimpl->version = version;
  pimpl->copyright = copyright;
  pimpl->trademarks = trademarks;
  LogInfo(fmt::format(T_("this is MiKTeX-{0} {1} ({2})"), programName, version, Utils::GetMiKTeXBannerString()));
}

bool WebApp::IsFeatureEnabled(Feature f) const
{
  return pimpl->features[f];
}

string WebApp::GetProgramName() const
{
  return pimpl->programName;
}

bool WebApp::AmI(const std::string& name) const
{
  return StringUtil::Contains(GetProgramName().c_str(), name.c_str());
}

bool WebApp::Enable8BitCharsP() const
{
  return pimpl->enable8BitChars;
}

PathName WebApp::GetTcxFileName() const
{
  return pimpl->tcxFileName;
}

void WebApp::EnableFeature(Feature f)
{
  pimpl->features += f;
}

void WebApp::Enable8BitChars(bool enable8BitChars)
{
  pimpl->enable8BitChars = enable8BitChars;
}

void WebApp::SetTcxFileName(const PathName& tcxFileName)
{
  pimpl->tcxFileName = tcxFileName;
}

void WebApp::InitializeCharTables() const
{
  unsigned long flags = 0;
  PathName tcxFileName = GetTcxFileName();
  if (!tcxFileName.Empty())
  {
    flags |= ICT_TCX;
  }
  if (Enable8BitCharsP())
  {
    flags |= ICT_8BIT;
  }
  if (pimpl->characterConverter == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  MiKTeX::TeXAndFriends::InitializeCharTables(flags, tcxFileName,
    pimpl->characterConverter->xchr(),
    pimpl->characterConverter->xord(),
    pimpl->isMETAFONTProgram || pimpl->isTeXProgram ? pimpl->characterConverter->xprn() : nullptr);
}

void WebApp::SetCharacterConverter(ICharacterConverter* characterConverter)
{
  pimpl->characterConverter = characterConverter;
}

ICharacterConverter* WebApp::GetCharacterConverter() const
{
  return pimpl->characterConverter;
}

void WebApp::SetInitFinalize(IInitFinalize* initFinalize)
{
  pimpl->initFinalize = initFinalize;
}

IInitFinalize* WebApp::GetInitFinalize() const
{
  return pimpl->initFinalize;
}

vector<poptOption> WebApp::GetOptions() const
{
  return pimpl->options;
}

void WebApp::SetTeX()
{
  pimpl->isTeXProgram = true;
}

bool WebApp::AmITeX() const
{
  return pimpl->isTeXProgram;
}

bool WebApp::AmIMETAFONT() const
{
  return pimpl->isMETAFONTProgram;
}

bool WebApp::GetVerboseFlag() const
{
  return pimpl->verbose;
}