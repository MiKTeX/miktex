/* initexmf.cpp: MiKTeX configuration utility

   Copyright (C) 1996-2022 Christian Schenk

   This file is part of IniTeXMF.

   IniTeXMF is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   IniTeXMF is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with IniTeXMF; if not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#if defined(HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "initexmf-version.h"

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/Directory>
#include <miktex/Core/Exceptions>
#include <miktex/Core/File>
#include <miktex/Core/FileType>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Process>
#include <miktex/Core/Quoter>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Setup/SetupService>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/DateUtil>
#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

#if defined(MIKTEX_WINDOWS)
#include <miktex/Core/win/ConsoleCodePageSwitcher>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>
#if defined(LOG4CXX_INFO_FMT)
#  define MIKTEX_LOG4CXX_12 1
#endif

using namespace std;
using namespace std::string_literals;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

const char* const TheNameOfTheGame = T_("MiKTeX Configuration Utility");

#define PROGNAME "initexmf"

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(PROGNAME));
static bool isLog4cxxConfigured = false;

static struct
{
  const char* lpszShortcut;
  const char* lpszFile;
}
configShortcuts[] = {
  {"pdftex", MIKTEX_PATH_PDFTEX_CFG},
  {"dvips", MIKTEX_PATH_CONFIG_PS},
  {"dvipdfmx", MIKTEX_PATH_DVIPDFMX_CONFIG},
  {"updmap", MIKTEX_PATH_UPDMAP_CFG},
};

string Timestamp()
{
  auto now = time(nullptr);
  stringstream s;
  s << std::put_time(localtime(&now), "%Y-%m-%d-%H%M%S");
  return s.str();
}

class IniTeXMFApp :
  public IFindFileCallback,
  public ICreateFndbCallback,
  public PackageInstallerCallback,
  public TraceCallback
{
public:
  IniTeXMFApp();

public:
  ~IniTeXMFApp();

public:
  PathName GetLogDir()
  {
    return session->GetSpecialPath(SpecialPath::LogDirectory);
  }

public:
  string GetLogName()
  {
    string logName = "initexmf";
    if (session->IsAdminMode() && session->RunningAsAdministrator())
    {
      logName += MIKTEX_ADMIN_SUFFIX;
    }
    return logName;
  }

public:
  void Init(int argc, const char* argv[]);

public:
  void Finalize(bool keepSession);

private:
  void Verbose(const string& s);

private:
  void PrintOnly(const string& s);

private:
  void Warning(const string& s);

private:
  void SecurityRisk(const string& s);

private:
  MIKTEXNORETURN void FatalError(const string& s);

private:
  void ListMetafontModes();

private:
  void Clean();

private:
  void SetTeXMFRootDirectories(RegisterRootDirectoriesOptionSet options);

private:
  void RunProcess(const PathName& fileName, const vector<string>& arguments)
  {
    ProcessOutput<4096> output;
    int exitCode;
    MiKTeXException miktexException;
    if (!Process::Run(fileName, arguments, &output, &exitCode, &miktexException, nullptr) || exitCode != 0)
    {
      auto outputBytes = output.GetStandardOutput();
      PathName outfile = GetLogDir() / fileName.GetFileNameWithoutExtension();
      outfile += "_";
      outfile += Timestamp().c_str();
      outfile.SetExtension(".out");
      File::WriteBytes(outfile, outputBytes);
      MIKTEX_ASSERT(isLog4cxxConfigured);
      LOG4CXX_ERROR(logger, "sub-process error output has been saved to '" << outfile.ToDisplayString() << "'");
      throw miktexException;
    }
  }

private:
  void RunOneMiKTeXUtility(const vector<string>& arguments);

private:
  void MakeFormatFiles(const vector<string>& formats);

private:
  void MakeFormatFilesByName(const vector<string>& formatsByName, const string& engine);

private:
  void MakeMaps(bool force);

private:
  void CreateConfigFile(const string& relPath, bool edit);

private:
  void SetConfigValue(const string& valueSpec);

private:
  void ShowConfigValue(const string& valueSpec);

private:
  void ManageLinks(bool remove, bool force);

private:
  void MakeLanguageDat(bool force);

private:
  void RegisterRoots(const vector<PathName>& roots, bool other, bool reg);

private:
  void ModifyPath();

private:
  void WriteReport();

private:
  void Bootstrap();

private:
  struct OtherTeX
  {
    string name;
    string version;
    StartupConfig startupConfig;
  };

private:
  vector<OtherTeX> FindOtherTeX();

private:
  void RegisterOtherRoots();

private:
  void CreatePortableSetup(const PathName& portableRoot);

public:
  void Run(int argc, const char* argv[]);

private:
  bool InstallPackage(const string& packageId, const PathName& trigger, PathName& installRoot) override;

private:
  bool TryCreateFile(const MiKTeX::Util::PathName& fileName, MiKTeX::Core::FileType fileType) override;

private:
  bool ReadDirectory(const PathName& path, vector<string>& subDirNames, vector<string>& fileNames, vector<string>& fileNameInfos) override;

private:
  bool OnProgress(unsigned level, const PathName& directory) override;

public:
  void ReportLine(const string& str) override;
  
public:
  bool OnRetryableError(const string& message) override;
  
public:
  bool OnProgress(MiKTeX::Packages::Notification nf) override;

private:
  vector<TraceCallback::TraceMessage> pendingTraceMessages;

private:
  void PushTraceMessage(const TraceCallback::TraceMessage& traceMessage)
  {
    if (pendingTraceMessages.size() > 100)
    {
      pendingTraceMessages.clear();
    }
    pendingTraceMessages.push_back(traceMessage);
  }

private:
  void PushTraceMessage(const string& message)
  {
    PushTraceMessage(TraceCallback::TraceMessage("initexmf", "initexmf", TraceLevel::Trace, message));
  }
  
public:
  bool Trace(const TraceCallback::TraceMessage& traceMessage) override
  {
    if (!isLog4cxxConfigured)
    {
#if 0
      fprintf(stderr, "%s\n", traceMessage.message.c_str());
#endif
      PushTraceMessage(traceMessage);
      return true;
    }
    FlushPendingTraceMessages();
    LogTraceMessage(traceMessage);
    return true;
  }

private:
  void FlushPendingTraceMessages()
  {
    for (const TraceCallback::TraceMessage& msg : pendingTraceMessages)
    {
      if (isLog4cxxConfigured)
      {
        LogTraceMessage(msg);
      }
      else
      {
        cerr << msg.message << endl;
      }
    }
    pendingTraceMessages.clear();
  }

private:
  void LogTraceMessage(const TraceCallback::TraceMessage& traceMessage)
  {
    MIKTEX_ASSERT(isLog4cxxConfigured);
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.initexmf.") + traceMessage.facility);
    switch (traceMessage.level)
    {
    case TraceLevel::Fatal:
      LOG4CXX_FATAL(logger, traceMessage.message);
      break;
    case TraceLevel::Error:
      LOG4CXX_ERROR(logger, traceMessage.message);
      break;
    case TraceLevel::Warning:
      LOG4CXX_WARN(logger, traceMessage.message);
      break;
    case TraceLevel::Info:
      LOG4CXX_INFO(logger, traceMessage.message);
      break;
    case TraceLevel::Trace:
      LOG4CXX_TRACE(logger, traceMessage.message);
      break;
    case TraceLevel::Debug:
    default:
      LOG4CXX_DEBUG(logger, traceMessage.message);
      break;
    }
  }

private:
  void EnsureInstaller()
  {
    if (packageInstaller == nullptr)
    {
      packageInstaller = packageManager->CreateInstaller({ this, true, false });
    }
  }

private:
  string principal;

private:
  bool recursive = false;

private:
  bool verbose = false;

private:
  bool quiet = false;

private:
  bool printOnly = false;

private:
  StartupConfig startupConfig;

private:
  vector<string> formatsMade;

private:
  ofstream logStream;

private:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

private:
  std::shared_ptr<MiKTeX::Packages::PackageInstaller> packageInstaller;

private:
  TriState enableInstaller = TriState::Undetermined;

private:
  std::shared_ptr<MiKTeX::Core::Session> session;

private:
  static const struct poptOption options[];
};

enum Option
{
  OPT_AAA = 256,

  OPT_ADMIN,
  OPT_DISABLE_INSTALLER,
  OPT_DUMP,
  OPT_DUMP_BY_NAME,
  OPT_EDIT_CONFIG_FILE,
  OPT_ENABLE_INSTALLER,
  OPT_ENGINE,
  OPT_FORCE,
  OPT_LIST_MODES,
  OPT_MKLANGS,
  OPT_MKLINKS,
  OPT_MKMAPS,
  OPT_PRINCIPAL,
  OPT_PRINT_ONLY,
  OPT_REGISTER_ROOT,
  OPT_REMOVE_LINKS,
  OPT_QUIET,
  OPT_UNREGISTER_ROOT,
  OPT_REPORT,
  OPT_UPDATE_FNDB,
  OPT_USER_ROOTS,
  OPT_VERBOSE,
  OPT_VERSION,

  OPT_ADD_FILE,                 // <experimental/>
  OPT_CLEAN,                    // <experimental/>
  OPT_CREATE_CONFIG_FILE,       // <experimental/>
  OPT_FIND_OTHER_TEX,           // <experimental/>
  OPT_LIST_FORMATS,             // <experimental/>
  OPT_MODIFY_PATH,              // <experimental/>
  OPT_RECURSIVE,                // <experimental/>
  OPT_REGISTER_OTHER_ROOTS,     // <experimental/>
  OPT_REMOVE_FILE,              // <experimental/>
  OPT_SET_CONFIG_VALUE,         // <experimental/>
  OPT_SHOW_CONFIG_VALUE,                // <experimental/>

  OPT_COMMON_CONFIG,            // <internal/>
  OPT_COMMON_DATA,              // <internal/>
  OPT_COMMON_INSTALL,           // <internal/>
  OPT_COMMON_ROOTS,             // <internal/>
  OPT_LOG_FILE,                 // <internal/>
  OPT_DEFAULT_PAPER_SIZE,       // <internal/>
#if defined(MIKTEX_WINDOWS)
  OPT_NO_REGISTRY,              // <internal/>
#endif
  OPT_PORTABLE,                 // <internal/>
  OPT_USER_CONFIG,              // <internal/>
  OPT_USER_DATA,                // <internal/>
  OPT_USER_INSTALL,             // <internal/>
};

#include "options.h"

IniTeXMFApp::IniTeXMFApp()
{
}

IniTeXMFApp::~IniTeXMFApp()
{
  try
  {
    Finalize(false);
  }
  catch (const exception &)
  {
  }
}

void IniTeXMFApp::Init(int argc, const char* argv[])
{
  bool adminMode = false;
  bool forceAdminMode = false;
  Session::InitOptionSet options;
  for (const char** opt = &argv[1]; *opt != nullptr; ++opt)
  {
    if ("--admin"s == *opt || "-admin"s == *opt)
    {
      adminMode = true;
    }
    else if ("--principal=setup"s == *opt || "-principal=setup"s == *opt)
    {
      options += Session::InitOption::SettingUp;
      forceAdminMode = true;
    }
  }
  Session::InitInfo initInfo(argv[0]);
  initInfo.SetOptions(options);
  initInfo.SetTraceCallback(this);
  session = Session::Create(initInfo);
  packageManager = PackageManager::Create(PackageManager::InitInfo(this));
  if (adminMode)
  {
    if (!forceAdminMode && !session->IsSharedSetup())
    {
      FatalError(T_("Option --admin only makes sense for a shared MiKTeX setup."));
    }
    if (!session->RunningAsAdministrator())
    {
      Warning(T_("Option --admin may require administrator privileges"));
    }
    session->SetAdminMode(true, forceAdminMode);
  }
  if (session->RunningAsAdministrator() && !session->IsAdminMode())
  {
    SecurityRisk(T_("running with elevated privileges"));
  }
  Bootstrap();
  enableInstaller = session->GetConfigValue(MIKTEX_CONFIG_SECTION_MPM, MIKTEX_CONFIG_VALUE_AUTOINSTALL).GetTriState();
  PathName xmlFileName;
  if (session->FindFile("initexmf." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
    || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
  {
    Utils::SetEnvironmentString("MIKTEX_LOG_DIR", GetLogDir().ToString());
    Utils::SetEnvironmentString("MIKTEX_LOG_NAME", GetLogName());
    log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
  }
  else
  {
    log4cxx::BasicConfigurator::configure();
  }
  isLog4cxxConfigured = true;
  auto thisProcess = Process::GetCurrentProcess();
  auto parentProcess = thisProcess->get_Parent();
  string invokerName;
  if (parentProcess != nullptr)
  {
    invokerName = parentProcess->get_ProcessName();
  }
  if (invokerName.empty())
  {
    invokerName = "unknown process";
  }
  LOG4CXX_INFO(logger, "this is " << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)));
  PathName cwd;
  cwd.SetToCurrentDirectory();
  LOG4CXX_INFO(logger, fmt::format("this process ({0}) started by {1} in directory {2} with command line: {3}", thisProcess->GetSystemId(), Q_(invokerName), cwd.ToDisplayString(), CommandLineBuilder(argc, argv)));
  FlushPendingTraceMessages();
  if (session->IsAdminMode())
  {
    Verbose(T_("Operating on the shared (system-wide) MiKTeX setup"));
  }
  else
  {
    Verbose(T_("Operating on the private (per-user) MiKTeX setup"));
  }
  PathName myName = PathName(argv[0]).GetFileNameWithoutExtension();
  session->SetFindFileCallback(this);
}

void IniTeXMFApp::Finalize(bool keepSession)
{
  if (logStream.is_open())
  {
    logStream.close();
  }
  FlushPendingTraceMessages();
  packageInstaller = nullptr;
  packageManager = nullptr;
  if (session != nullptr && !keepSession)
  {
    session->Close();
    session = nullptr;
  }
}

void IniTeXMFApp::Verbose(const string& s)
{
  if (!printOnly && isLog4cxxConfigured)
  {
    LOG4CXX_INFO(logger, s);
  }
  if (verbose && !printOnly)
  {
    cout << s << endl;
  }
}

void IniTeXMFApp::PrintOnly(const string& s)
{
  if (!printOnly)
  {
    return;
  }
  cout << s << endl;
}

void IniTeXMFApp::Warning(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_WARN(logger, s);
  }
  if (!quiet)
  {
    cerr << PROGNAME << ": " << T_("warning") << ": " << s << endl;
  }
}

void IniTeXMFApp::SecurityRisk(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_WARN(logger, T_("security risk") << ": " << s);
  }
  if (!quiet)
  {
    cerr << PROGNAME << ": " << T_("security risk") << ": " << s << endl;
  }
}

static void Sorry(const string& description, const string& remedy, const string& url)
{
  if (cerr.fail())
  {
    return;
  }
  cerr << endl;
  if (description.empty())
  {
    cerr << fmt::format(T_("Sorry, but {0} did not succeed."), Q_(TheNameOfTheGame)) << endl;
  }
  else
  {
    cerr
      << fmt::format(T_("Sorry, but {0} did not succeed for the following reason:"), Q_(TheNameOfTheGame)) << "\n"
      << "\n"
      << "  " << description << endl;
    if (!remedy.empty())
    {
      cerr
        << "\n"
        << T_("Remedy:") << "\n"
        << "\n"
        << "  " << remedy << endl;
    }
  }
  if (isLog4cxxConfigured)
  {
#if defined(MIKTEX_LOG4CXX_12)
    log4cxx::AppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
    log4cxx::FileAppenderPtr fileAppender = log4cxx::cast<log4cxx::FileAppender>(appender);
#else
    log4cxx::FileAppenderPtr fileAppender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
#endif
    if (fileAppender != nullptr)
    {
      cerr
        << "\n"
        << T_("The log file hopefully contains the information to get MiKTeX going again:") << "\n"
        << "\n"
        << "  " << PathName(fileAppender->getFile()) << endl;
    }
  }
  if (!url.empty())
  {
    cerr
      << "\n"
      << T_("For more information, visit:") << " " << url << endl;
  }
}

static void Sorry()
{
  Sorry("", "", "");
}

MIKTEXNORETURN void IniTeXMFApp::FatalError(const string& s)
{
  if (isLog4cxxConfigured)
  {
    LOG4CXX_FATAL(logger, s);
  }
  else
  {
    cerr << s << endl;
  }
  Sorry(s, "", "");
  throw 1;
}

bool IniTeXMFApp::InstallPackage(const string& packageId, const PathName& trigger, PathName& installRoot)
{
  if (enableInstaller != TriState::True)
  {
    return false;
  }
  LOG4CXX_INFO(logger, "installing package " << packageId << " triggered by " << trigger.ToString());
  Verbose(fmt::format(T_("Installing package {0}..."), packageId));
  EnsureInstaller();
  packageInstaller->SetFileLists({ packageId }, {});
  packageInstaller->InstallRemove(PackageInstaller::Role::Application);
  installRoot = session->GetSpecialPath(SpecialPath::InstallRoot);
  return true;
}

bool IniTeXMFApp::TryCreateFile(const MiKTeX::Util::PathName& fileName, MiKTeX::Core::FileType fileType)
{
  return false;
}

bool IniTeXMFApp::ReadDirectory(const PathName& path, vector<string>& subDirNames, vector<string>& fileNames, vector<string>& fileNameInfos)
{
  return false;
}

bool IniTeXMFApp::OnProgress(unsigned level, const PathName& directory)
{
#if 0
  if (verbose && level == 1)
  {
    Verbose(fmt::format(T_("Scanning {0}"), Q_(directory)));
  }
  else if (level == 1)
  {
    Message(".");
  }
#endif
  return true;
}

void IniTeXMFApp::ListMetafontModes()
{
  MIKTEXMFMODE mode;
  for (unsigned i = 0; session->GetMETAFONTMode(i, mode); ++i)
  {
    cout << fmt::format("{:<8}  {:>5}x{:<5}  {}", mode.mnemonic, mode.horizontalResolution, mode.verticalResolution, mode.description) << endl;
  }
}

void IniTeXMFApp::Clean()
{
  RunOneMiKTeXUtility({"links", "uninstall"});
  session->UnloadFilenameDatabase();
  Finalize(true);
  isLog4cxxConfigured = false;
  logger = nullptr;
  PathName dataRoot = session->GetSpecialPath(SpecialPath::DataRoot);
  if (Directory::Exists(dataRoot))
  {
    Directory::Delete(dataRoot, true);
  }
}

void IniTeXMFApp::SetTeXMFRootDirectories(RegisterRootDirectoriesOptionSet options)
{
  Verbose(T_("Registering root directories..."));
  PrintOnly(fmt::format("regroots ur={} ud={} uc={} ui={} cr={} cd={} cc={} ci={}",
    Q_(startupConfig.userRoots), Q_(startupConfig.userDataRoot), Q_(startupConfig.userConfigRoot), Q_(startupConfig.userInstallRoot),
    Q_(startupConfig.commonRoots), Q_(startupConfig.commonDataRoot), Q_(startupConfig.commonConfigRoot), Q_(startupConfig.commonInstallRoot)));
  if (!printOnly)
  {
    options += RegisterRootDirectoriesOption::Review;
    session->RegisterRootDirectories(startupConfig, options);
  }
}

void IniTeXMFApp::MakeFormatFiles(const vector<string>& formats)
{
  if (formats.empty())
  {
    RunOneMiKTeXUtility({ "formats", "build" });
    return;
  }
  for (const string& fmt : formats)
  {
    RunOneMiKTeXUtility({ "formats", "build", fmt });
  }
}

void IniTeXMFApp::MakeFormatFilesByName(const vector<string>& formatsByName, const string& engine)
{
  // ASSUME: format key and name are the same
  for (const string& name : formatsByName)
  {
    vector<string> args{ "formats", "build", name };
    if (!engine.empty())
    {
      args.insert(args.end(), { "--engine", engine });
    }
    RunOneMiKTeXUtility(args);
  }
}

void IniTeXMFApp::RegisterRoots(const vector<PathName>& roots, bool other, bool reg)
{
  for (const auto& root : roots)
  {
    if (reg)
    {
      session->RegisterRootDirectory(root, other);
    }
    else
    {
      session->UnregisterRootDirectory(root, other);
    }
  }
  if (reg)
  {
    RunOneMiKTeXUtility({"fndb", "refresh"});
  }
}

void IniTeXMFApp::ModifyPath()
{
#if defined(MIKTEX_WINDOWS)
  Utils::CheckPath(true);
#else
  // TODO: check path
  UNIMPLEMENTED();
#endif
}

void IniTeXMFApp::ManageLinks(bool remove, bool force)
{
  vector<string> args{"links"};
  if (remove)
  {
    args.push_back("uninstall");
  }
  else
  {
    args.push_back("install");
    if (force)
    {
      args.push_back("--force");
    }
  }
  RunOneMiKTeXUtility(args);
}

void IniTeXMFApp::RunOneMiKTeXUtility(const vector<string>& arguments)
{
  PathName oneMiKTeXUtility;
  if (!session->FindFile("miktex", FileType::EXE, oneMiKTeXUtility))
  {
    FatalError(T_("The miktex executable could not be found."));
  }
  vector<string> allArguments{ "miktex" };
  if (verbose)
  {
    allArguments.push_back("--verbose");
  }
  if (session->IsAdminMode())
  {
    allArguments.push_back("--admin");
  }
  switch (enableInstaller)
  {
  case TriState::True:
    allArguments.push_back("--enable-installer");
    break;
  case TriState::False:
    allArguments.push_back("--disable-installer");
    break;
  default:
    break;
  }
#if 0
  // TODO: remove
  allArguments.push_back("--miktex-disable-maintenance");
  allArguments.push_back("--miktex-disable-diagnose");
#endif
  allArguments.insert(allArguments.end(), arguments.begin(), arguments.end());
  if (printOnly)
  {
    PrintOnly(CommandLineBuilder(allArguments).ToString());
  }
  else
  {
    LOG4CXX_INFO(logger, "running: " << CommandLineBuilder(allArguments));
    RunProcess(oneMiKTeXUtility, allArguments);
  }
}

void IniTeXMFApp::MakeLanguageDat(bool force)
{
  vector<string> arguments{"languages", "configure"};
  if (force)
  {
    arguments.push_back("--force");
  }
  RunOneMiKTeXUtility(arguments);
}

void IniTeXMFApp::MakeMaps(bool force)
{
  vector<string> arguments{"fontmaps", "configure"};
  if (force)
  {
    arguments.push_back("--force");
  }
  RunOneMiKTeXUtility(arguments);
}

void IniTeXMFApp::CreateConfigFile(const string& relPath, bool edit)
{
  PathName configFile(session->GetSpecialPath(SpecialPath::ConfigRoot));
  bool haveConfigFile = false;
  for (const auto& shortCut : configShortcuts)
  {
    if (PathName::Compare(relPath, shortCut.lpszShortcut) == 0)
    {
      configFile /= shortCut.lpszFile;
      haveConfigFile = true;
      break;
    }
  }
  if (!haveConfigFile)
  {
    PathName fileName(relPath);
    fileName.RemoveDirectorySpec();
    if (fileName == PathName(relPath))
    {
      configFile /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
    }
    configFile /= relPath;
    configFile.SetExtension(".ini", false);
    haveConfigFile = true;
  }
  if (!File::Exists(configFile))
  {
    Verbose(fmt::format(T_("Creating config file: {0}..."), configFile));
    if (!session->TryCreateFromTemplate(configFile))
    {
      File::WriteBytes(configFile, {});
      Fndb::Add({ {configFile} });
    }
  }
  if (edit)
  {
    string editor;
    const char* lpszEditor = getenv("EDITOR");
    if (lpszEditor != nullptr)
    {
      editor = lpszEditor;
    }
    else
    {
#if defined(MIKTEX_WINDOWS)
      editor = "notepad.exe";
#else
      FatalError(T_("Environment variable EDITOR is not defined."));
#endif
    }
    Process::Start(PathName(editor), vector<string>{ editor, configFile.ToString() });
  }
}

void IniTeXMFApp::SetConfigValue(const string& valueSpec)
{
  const char* lpsz = valueSpec.c_str();
  string section;
  bool haveSection = (*lpsz == '[');
  if (haveSection)
  {
    ++lpsz;
    for (; *lpsz != 0 && *lpsz != ']'; ++lpsz)
    {
      section += *lpsz;
    }
    if (*lpsz == 0)
    {
      LOG4CXX_FATAL(logger, T_("Invalid value: ") << Q_(valueSpec));
      FatalError(fmt::format(T_("The configuration value '{0}' could not be set."), Q_(valueSpec)));
    }
    ++lpsz;
  }
  string valueName;
  for (; *lpsz != 0 && *lpsz != '='; ++lpsz)
  {
    valueName += *lpsz;
  }
  if (*lpsz == 0)
  {
    LOG4CXX_FATAL(logger, T_("Invalid value: ") << Q_(valueSpec));
    FatalError(fmt::format(T_("The configuration value '{0}' could not be set."), Q_(valueSpec)));
  }
  ++lpsz;
  string value = lpsz;
  Verbose(fmt::format(T_("Setting config value: [{0}]{1}={2}"), section, valueName, value));
  session->SetConfigValue(section, valueName, ConfigValue(value));
}

void IniTeXMFApp::ShowConfigValue(const string& valueSpec)
{
  const char* lpsz = valueSpec.c_str();
  string section;
  bool haveSection = (*lpsz == '[');
  if (haveSection)
  {
    ++lpsz;
    for (; *lpsz != 0 && *lpsz != ']'; ++lpsz)
    {
      section += *lpsz;
    }
    if (*lpsz == 0)
    {
      FatalError(fmt::format(T_("Invalid value: {0}."), Q_(valueSpec)));
    }
    ++lpsz;
  }
  string valueName = lpsz;
  string value;
  if (session->TryGetConfigValue(section, valueName, value))
  {
    cout << value << endl;
  }
}

void IniTeXMFApp::ReportLine(const string& str)
{
  Verbose(str);
}

bool IniTeXMFApp::OnRetryableError(const string& message)
{
  return false;
}

bool IniTeXMFApp::OnProgress(MiKTeX::Packages::Notification nf)
{
  return true;
}

void IniTeXMFApp::Bootstrap()
{
#if defined(WITH_BOOTSTRAPPING)
  vector<string> neededPackages;
  for (const string& package : StringUtil::Split(MIKTEX_BOOTSTRAPPING_PACKAGES, ';'))
  {
    PackageInfo packageInfo;
    if (!packageManager->TryGetPackageInfo(package, packageInfo))
    {
      neededPackages.push_back(package);
    }
  }
  if (!neededPackages.empty())
  {
    PathName bootstrappingDir = session->GetSpecialPath(SpecialPath::DistRoot) / PathName(MIKTEX_PATH_MIKTEX_BOOTSTRAPPING_DIR);
    if (Directory::Exists(bootstrappingDir))
    {
      PushTraceMessage("running MIKTEX_HOOK_BOOTSTRAPPING");
      EnsureInstaller();
      packageInstaller->SetRepository(bootstrappingDir.ToString());
      packageInstaller->UpdateDb({});
      packageInstaller->SetFileList(neededPackages);
      packageInstaller->InstallRemove(PackageInstaller::Role::Application);
      packageInstaller = nullptr;
    }
  }
#endif
}

string kpsewhich_expand_path(const string& varname)
{
  string cmd = "kpsewhich --expand-path=";
#if defined(MIKTEX_UNIX)
  cmd += "\\";
#endif
  ProcessOutput<1024> output;
  int exitCode;
  if (!Process::ExecuteSystemCommand(cmd + "$" + varname, &exitCode, &output, nullptr) || exitCode != 0)
  {
    return string();
  }
  string result = output.StdoutToString();
  if (!result.empty() && result[result.length() - 1] == '\n')
  {
    result.erase(result.length() - 1);
  }
  return result;
}

string Concat(const initializer_list<string>& searchPaths, char separator = PathNameUtil::PathNameDelimiter)
{
  string result;
  for (const string& s : searchPaths)
  {
    if (s.empty())
    {
      continue;
    }
    if (!result.empty())
    {
      result += separator;
    }
    result += s;
  }
  return result;
}

vector<IniTeXMFApp::OtherTeX> IniTeXMFApp::FindOtherTeX()
{
  vector<OtherTeX> result;
  ProcessOutput<1024> version;
  int exitCode;
  if (Process::ExecuteSystemCommand("kpsewhich --version", &exitCode, &version, nullptr) && version.StdoutToString().find("MiKTeX") == string::npos)
  {
    OtherTeX otherTeX;
    otherTeX.name = "kpathsea";
    string versionString = version.StdoutToString();
    otherTeX.version = versionString.substr(0, versionString.find_first_of("\r\n"));
    StartupConfig otherConfig;
    otherConfig.userRoots = Concat({
      kpsewhich_expand_path("TEXMFHOME")
    });
    otherConfig.commonRoots = Concat({
      kpsewhich_expand_path("TEXMFLOCAL"),
      kpsewhich_expand_path("TEXMFDEBIAN"),
      kpsewhich_expand_path("TEXMFDIST")
    });
    otherTeX.startupConfig = otherConfig;
    result.push_back(otherTeX);
  }
  return result;
}

void IniTeXMFApp::RegisterOtherRoots()
{
  vector<OtherTeX> otherTeXDists = FindOtherTeX();
  vector<PathName> otherRoots;
  for (const OtherTeX& other : otherTeXDists)
  {
    const string& roots = (session->IsAdminMode() ? other.startupConfig.commonRoots : other.startupConfig.userRoots);
    for (const string& r : StringUtil::Split(roots, PathNameUtil::PathNameDelimiter))
    {
      otherRoots.push_back(PathName(r));
    }
  }
  if (otherRoots.empty())
  {
  }
  else
  {
    RegisterRoots(otherRoots, true, true);
  }
}

void IniTeXMFApp::CreatePortableSetup(const PathName& portableRoot)
{
  Verbose(T_("Creating portable setup..."));
  unique_ptr<Cfg> config(Cfg::Create());
  config->PutValue(MIKTEX_CONFIG_SECTION_AUTO, MIKTEX_CONFIG_VALUE_CONFIG, "Portable");
  config->PutValue(MIKTEX_CONFIG_SECTION_SETUP, MIKTEX_CONFIG_VALUE_VERSION, VersionNumber(MIKTEX_MAJOR_VERSION, MIKTEX_MINOR_VERSION, MIKTEX_PATCH_VERSION, 0).ToString());
  PathName configDir(portableRoot);
  configDir /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
  Directory::Create(configDir);
  PathName startupFile(portableRoot);
  startupFile /= MIKTEX_PATH_STARTUP_CONFIG_FILE;
  config->Write(startupFile, T_("MiKTeX startup configuration"));
  PathName tempDir(portableRoot);
  tempDir /= MIKTEX_PATH_MIKTEX_TEMP_DIR;
  if (!Directory::Exists(tempDir))
  {
    Directory::Create(tempDir);
  }
  session->Reset();
}

void IniTeXMFApp::WriteReport()
{
  auto setupService = SetupService::Create();
  setupService->WriteReport(cout, { ReportOption::General, ReportOption::RootDirectories, ReportOption::CurrentUser });
}

void IniTeXMFApp::Run(int argc, const char* argv[])
{
  vector<string> addFiles;
  vector<string> showConfigValue;
  vector<string> setConfigValues;
  vector<string> createConfigFiles;
  vector<string> editConfigFiles;
  vector<string> formats;
  vector<string> formatsByName;
  vector<string> removeFiles;
  vector<PathName> registerRoots;
  vector<PathName> unregisterRoots;
  string defaultPaperSize;
  string engine;
  string logFile;
  string portableRoot;

  bool optClean = false;
  bool optDump = false;
  bool optDumpByName = false;
  bool optFindOtherTeX = false;
  bool optForce = false;
  bool optMakeLanguageDat = false;
  bool optMakeMaps = false;
  bool optListFormats = false;
  bool optListModes = false;
  bool optMakeLinks = false;
#if defined(MIKTEX_WINDOWS)
  bool optNoRegistry = false;
#endif
  bool optPortable = false;
  bool optRegisterOtherRoots = false;
  bool optRemoveLinks = false;
  bool optModifyPath = false;
  bool optReport = false;
  bool optUpdateFilenameDatabase = false;
  bool optVersion = false;

  const struct poptOption* aoptions;

  aoptions = options;

  PoptWrapper popt(argc, argv, aoptions);

  int option;

  while ((option = popt.GetNextOpt()) >= 0)
  {
    string optArg = popt.GetOptArg();
    switch (option)
    {

    case OPT_ADD_FILE:

      addFiles.push_back(optArg);
      break;

    case OPT_CLEAN:
      optClean = true;
      break;

    case OPT_CREATE_CONFIG_FILE:

      createConfigFiles.push_back(optArg);
      break;


    case OPT_DEFAULT_PAPER_SIZE:

      defaultPaperSize = optArg;
      break;

    case OPT_DISABLE_INSTALLER:
      enableInstaller = TriState::False;
      break;

    case OPT_DUMP:

      if (!optArg.empty())
      {
        formats.push_back(optArg);
      }
      optDump = true;
      break;

    case OPT_DUMP_BY_NAME:

      formatsByName.push_back(optArg);
      optDumpByName = true;
      break;

    case OPT_EDIT_CONFIG_FILE:

      editConfigFiles.push_back(optArg);
      break;

    case OPT_ENABLE_INSTALLER:
      enableInstaller = TriState::True;
      break;

    case OPT_ENGINE:
      engine = optArg;
      break;

    case OPT_FIND_OTHER_TEX:
      optFindOtherTeX = true;
      break;

    case OPT_FORCE:

      optForce = true;
      break;

    case OPT_COMMON_INSTALL:

      startupConfig.commonInstallRoot = optArg;
      break;

    case OPT_USER_INSTALL:

      startupConfig.userInstallRoot = optArg;
      break;

    case OPT_LIST_FORMATS:

      optListFormats = true;
      break;

    case OPT_LIST_MODES:

      optListModes = true;
      break;

    case OPT_COMMON_DATA:

      startupConfig.commonDataRoot = optArg;
      break;

    case OPT_COMMON_CONFIG:

      startupConfig.commonConfigRoot = optArg;
      break;

    case OPT_USER_DATA:

      startupConfig.userDataRoot = optArg;
      break;

    case OPT_USER_CONFIG:

      startupConfig.userConfigRoot = optArg;
      break;

    case OPT_LOG_FILE:

      logFile = optArg;
      break;

    case OPT_MKLANGS:

      optMakeLanguageDat = true;
      break;

    case OPT_MKLINKS:

      optMakeLinks = true;
      break;

    case OPT_MKMAPS:

      optMakeMaps = true;
      break;

    case OPT_MODIFY_PATH:

      optModifyPath = true;
      break;

#if defined(MIKTEX_WINDOWS)
    case OPT_NO_REGISTRY:

      optNoRegistry = true;
      break;
#endif

    case OPT_PORTABLE:

      portableRoot = optArg;
      optPortable = true;
      break;

    case OPT_PRINCIPAL:
      principal = optArg;
      break;

    case OPT_PRINT_ONLY:

      printOnly = true;
      break;

    case OPT_QUIET:

      quiet = true;
      break;

    case OPT_RECURSIVE:

      recursive = true;
      break;

    case OPT_REGISTER_OTHER_ROOTS:

      optRegisterOtherRoots = true;
      break;

    case OPT_REGISTER_ROOT:

      registerRoots.push_back(PathName(optArg));
      break;

    case OPT_REMOVE_FILE:

      removeFiles.push_back(optArg);
      break;

    case OPT_REMOVE_LINKS:

      optRemoveLinks = true;
      break;
      
    case OPT_REPORT:

      optReport = true;
      break;

    case OPT_SET_CONFIG_VALUE:

      setConfigValues.push_back(optArg);
      break;

    case OPT_SHOW_CONFIG_VALUE:

      showConfigValue.push_back(optArg);
      break;

    case OPT_COMMON_ROOTS:

      startupConfig.commonRoots = optArg;
      break;

    case OPT_USER_ROOTS:

      startupConfig.userRoots = optArg;
      break;

    case OPT_ADMIN:

      if (!session->IsAdminMode())
      {
        MIKTEX_UNEXPECTED();
      }
      break;

    case OPT_UNREGISTER_ROOT:

      unregisterRoots.push_back(PathName(optArg));
      break;

    case OPT_UPDATE_FNDB:

      optUpdateFilenameDatabase = true;
      break;

    case OPT_VERBOSE:

      verbose = true;
      break;

    case OPT_VERSION:

      optVersion = true;
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

  if (!popt.GetLeftovers().empty())
  {
    FatalError(T_("This utility does not accept non-option arguments."));
  }

  if (optVersion)
  {
    cout
      << Utils::MakeProgramVersionString(TheNameOfTheGame, VersionNumber(MIKTEX_COMPONENT_VERSION_STR)) << endl
      << endl
      << MIKTEX_COMP_COPYRIGHT_STR << endl
      << endl
      << "This is free software; see the source for copying conditions.  There is NO" << endl
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
    return;
  }

  if (!logFile.empty())
  {
    auto mode = File::Exists(PathName(logFile)) ? ios_base::app : ios_base::out;
    logStream = File::CreateOutputStream(PathName(logFile), mode);
  }

  if (optPortable)
  {
    CreatePortableSetup(PathName(portableRoot));
  }

  if ((principal == "setup" && !optPortable)
    || !startupConfig.userRoots.empty()
    || !startupConfig.userDataRoot.Empty()
    || !startupConfig.userConfigRoot.Empty()
    || !startupConfig.userInstallRoot.Empty()
    || !startupConfig.commonRoots.empty()
    || !startupConfig.commonDataRoot.Empty()
    || !startupConfig.commonConfigRoot.Empty()
    || !startupConfig.commonInstallRoot.Empty())
  {
    RegisterRootDirectoriesOptionSet options;
#if defined(MIKTEX_WINDOWS)
    if (optNoRegistry)
    {
      options += RegisterRootDirectoriesOption::NoRegistry;
    }
#endif
    SetTeXMFRootDirectories(options);
  }

  if (!defaultPaperSize.empty())
  {
    session->SetDefaultPaperSize(defaultPaperSize);
  }

  if (optDump)
  {
    MakeFormatFiles(formats);
  }

  if (optDumpByName)
  {
    MakeFormatFilesByName(formatsByName, engine);
  }

  if (optModifyPath)
  {
    ModifyPath();
  }

  if (optMakeLanguageDat)
  {
    MakeLanguageDat(optForce);
  }

  if (optMakeLinks || optRemoveLinks)
  {
    ManageLinks(optRemoveLinks, optForce);
  }

  if (optMakeMaps)
  {
    MakeMaps(optForce);
  }

  vector<Fndb::Record> records;
  for (const string& fileName : addFiles)
  {
    Verbose(fmt::format(T_("Adding {0} to the file name database..."), Q_(fileName)));
    PrintOnly(fmt::format("fndbadd {}", Q_(fileName)));
    if (!printOnly)
    {
      if (!Fndb::FileExists(PathName(fileName)))
      {
        records.push_back({ PathName(fileName) });
      }
      else
      {
        Warning(fmt::format(T_("{0} is already recorded in the file name database"), Q_(fileName)));
      }
    }
  }
  if (!records.empty())
  {
    Fndb::Add(records);
  }

  vector<PathName> paths;
  for (const string& fileName : removeFiles)
  {
    Verbose(fmt::format(T_("Removing {0} from the file name database..."), Q_(fileName)));
    PrintOnly(fmt::format("fndbremove {}", Q_(fileName)));
    if (!printOnly)
    {
      if (Fndb::FileExists(PathName(fileName)))
      {
        paths.push_back(PathName(fileName));
      }
      else
      {
        Warning(fmt::format(T_("{0} is not recorded in the file name database"), Q_(fileName)));
      }
    }
  }
  if (!paths.empty())
  {
    Fndb::Remove(paths);
  }

  if (!unregisterRoots.empty())
  {
    RegisterRoots(unregisterRoots, false, false);
  }

  if (!registerRoots.empty())
  {
    RegisterRoots(registerRoots, false, true);
  }

  if (optUpdateFilenameDatabase)
  {
    RunOneMiKTeXUtility({"fndb", "refresh"});
  }

  for (const string& fileName : createConfigFiles)
  {
    CreateConfigFile(fileName, false);
  }

  for (const string& v : setConfigValues)
  {
    SetConfigValue(v);
  }

  for (const string& v : showConfigValue)
  {
    ShowConfigValue(v);
  }

  for (const string& fileName : editConfigFiles)
  {
    CreateConfigFile(fileName, true);
  }

  if (optListFormats)
  {
    RunOneMiKTeXUtility({ "formats", "list" });
  }

  if (optListModes)
  {
    ListMetafontModes();
  }

  if (optReport)
  {
    WriteReport();
  }

  if (optFindOtherTeX)
  {
    vector<OtherTeX> otherTeXs = FindOtherTeX();
    for (const OtherTeX& otherTeX : otherTeXs)
    {
      cout << "Found OtherTeX: " << otherTeX.name << "\n";
      cout << "  Version: " << otherTeX.version << "\n";
      cout << "  UserRoots: " << otherTeX.startupConfig.userRoots << "\n";
      cout << "  CommonRoots: " << otherTeX.startupConfig.commonRoots << "\n";
    }
  }

  if (optRegisterOtherRoots)
  {
    RegisterOtherRoots();
  }

  if (optClean)
  {
    Clean();
  }
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
#if defined(MIKTEX_WINDOWS)
  ConsoleCodePageSwitcher cpSwitcher;
#endif
  int retCode = 0;
  try
  {
    vector<string> utf8args;
    utf8args.reserve(argc);
    vector<const char*> newargv;
    newargv.reserve(static_cast<size_t>(argc) + 1);
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
    IniTeXMFApp app;
    app.Init(argc, &newargv[0]);
    app.Run(argc, &newargv[0]);
    app.Finalize(false);
  }
  catch (const MiKTeXException& e)
  {
    if (logger != nullptr && isLog4cxxConfigured)
    {
      LOG4CXX_FATAL(logger, e.GetErrorMessage());
      LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
      LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
      LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
    }
    else
    {
      cerr << e.GetErrorMessage() << endl
           << "Info: " << e.GetInfo() << endl
           << "Source: " << e.GetSourceFile() << endl
           << "Line: " << e.GetSourceLine() << endl;
    }
    Sorry(e.GetDescription(), e.GetRemedy(), e.GetUrl());
    e.Save();
    retCode = 1;
  }
  catch (const exception& e)
  {
    if (logger != nullptr && isLog4cxxConfigured)
    {
      LOG4CXX_FATAL(logger, e.what());
    }
    else
    {
      cerr <<  e.what() << endl;
    }
    Sorry();
    retCode = 1;
  }
  catch (int exitCode)
  {
    logger = nullptr;
    retCode = 1;
  }
  if (logger != nullptr)
  {
    LOG4CXX_INFO(logger, "this process (" << Process::GetCurrentProcess()->GetSystemId() << ") finishes with exit code " << retCode);
    logger = nullptr;
  }
  return retCode;
}
