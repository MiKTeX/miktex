/* initexmf.cpp: MiKTeX configuration utility

   Copyright (C) 1996-2020 Christian Schenk

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

#include <miktex/Core/BufferSizes>
#include <miktex/Core/Cfg>
#include <miktex/Core/CommandLineBuilder>
#include <miktex/Core/ConfigNames>
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
#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>
#include <miktex/Wrappers/PoptWrapper>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logger.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>

using namespace std;
using namespace std::string_literals;

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

bool EndsWith(const string& s, const string& suffix)
{
  return s.length() >= suffix.length() &&
    s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

template<class VALTYPE> class AutoRestore
{
public:
  AutoRestore(VALTYPE& val) :
    oldVal(val),
    pVal(&val)
  {
  }

public:
  ~AutoRestore()
  {
    *pVal = oldVal;
  }

private:
  VALTYPE oldVal;

private:
  VALTYPE* pVal;
};

enum class LinkType
{
  Hard,
  Symbolic,
  Copy
};

struct FileLink
{
  FileLink(const string& target, const vector<string>& linkNames) :
    target(target),
    linkNames(linkNames)
  {
  }
  FileLink(const string& target, const vector<string>& linkNames, LinkType linkType) :
    target(target),
    linkNames(linkNames),
    linkType(linkType)
  {
  }
  string target;
  vector<string> linkNames;
#if defined(MIKTEX_WINDOWS)
  LinkType linkType = LinkType::Hard;
#else
  LinkType linkType = LinkType::Symbolic;
#endif
};

enum class LinkCategory
{
  Formats,
  MiKTeX,
  Scripts
};

typedef OptionSet<LinkCategory> LinkCategoryOptions;

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
  void UpdateFilenameDatabase(const PathName& root);

private:
  void UpdateFilenameDatabase(unsigned root);

private:
  void ListFormats();

private:
  void ListMetafontModes();

private:
  void Clean();

private:
  void RemoveFndb();

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
  void RunMakeTeX(const string& makeProg, const vector<string>& arguments);

private:
  void MakeFormatFile(const string& formatKey);

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
  vector<FileLink> CollectLinks(LinkCategoryOptions linkCategories);

private:
  void ManageLinks(LinkCategoryOptions linkCategories, bool remove, bool force);

#if defined(MIKTEX_UNIX)
private:
  void MakeFilesExecutable();
#endif

private:
  void MakeLanguageDat(bool force);

private:
  void RegisterRoots(const vector<PathName>& roots, bool other, bool reg);

#if defined(MIKTEX_WINDOWS)
private:
  void RegisterShellFileTypes(bool reg);
#endif

private:
  void ModifyPath();

private:
  void ManageLink(const FileLink& fileLink, bool supportsHardLinks, bool remove, bool overwrite);

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
  bool TryCreateFile(const MiKTeX::Core::PathName& fileName, MiKTeX::Core::FileType fileType) override;

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
  bool removeFndb = false;

private:
  StartupConfig startupConfig;

private:
  vector<string> formatsMade;

private:
  ofstream logStream;

private:
  bool isMktexlsrMode = false;

private:
  bool isTexlinksMode = false;
  
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

private:
  static const struct poptOption options_mktexlsr[];

private:
  static const struct poptOption options_texlinks[];
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
  OPT_REGISTER_SHELL_FILE_TYPES,        // <experimental/>
  OPT_REMOVE_FILE,              // <experimental/>
  OPT_SET_CONFIG_VALUE,         // <experimental/>
  OPT_SHOW_CONFIG_VALUE,                // <experimental/>
  OPT_UNREGISTER_SHELL_FILE_TYPES,      // <experimental/>

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
  OPT_RMFNDB,                   // <internal/>
  OPT_USER_CONFIG,              // <internal/>
  OPT_USER_DATA,                // <internal/>
  OPT_USER_INSTALL,             // <internal/>
};

#include "options.h"

const struct poptOption IniTeXMFApp::options_mktexlsr[] = {
  {
    "dry-run", 0,
    POPT_ARG_NONE, nullptr,
    OPT_PRINT_ONLY,
    T_("Print what would be done."),
    nullptr
  },

  {
    "quiet", 0,
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Suppress screen output."),
    nullptr
  },

  {
    "silent", 0,
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Same as --quiet."),
    nullptr
  },

  {
    "verbose", 0,
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Print information on what is being done."),
    nullptr
  },

  {
    "version", 0,
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

const struct poptOption IniTeXMFApp::options_texlinks[] = {
  {
    "quiet", 'q',
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Suppress screen output."),
    nullptr
  },

  {
    "silent", 's',
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Same as --quiet."),
    nullptr
  },

  {
    "verbose", 'v',
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Print information on what is being done."),
    nullptr
  },

  {
    "version", 0,
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

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
  for (const char** opt = &argv[1]; *opt != nullptr; ++opt)
  {
    if ("--admin"s == *opt || "-admin"s == *opt)
    {
      adminMode = true;
    }
    else if ("--principal=setup"s == *opt || "-principal=setup"s == *opt)
    {
      forceAdminMode = true;
    }
  }
  Session::InitInfo initInfo(argv[0]);
#if defined(MIKTEX_WINDOWS)
  initInfo.SetOptions({ Session::InitOption::InitializeCOM });
#endif
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
  LOG4CXX_INFO(logger, "this process (" << thisProcess->GetSystemId() << ") started by '" << invokerName << "' with command line: " << CommandLineBuilder(argc, argv));
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
  isMktexlsrMode = myName == PathName("mktexlsr") || myName == PathName("texhash");
  isTexlinksMode = myName == PathName("texlinks");
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
  if (!keepSession)
  {
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
    log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
    if (appender != nullptr)
    {
      cerr
        << "\n"
        << T_("The log file hopefully contains the information to get MiKTeX going again:") << "\n"
        << "\n"
        << "  " << PathName(appender->getFile()) << endl;
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

bool IniTeXMFApp::TryCreateFile(const MiKTeX::Core::PathName& fileName, MiKTeX::Core::FileType fileType)
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

void IniTeXMFApp::UpdateFilenameDatabase(const PathName& root)
{
  // unload the file name database
  if (!printOnly && !session->UnloadFilenameDatabase())
  {
    FatalError(T_("The file name database could not be unloaded."));
  }

  unsigned rootIdx = session->DeriveTEXMFRoot(root);

  // create the FNDB file
  PathName fndbPath = session->GetFilenameDatabasePathName(rootIdx);
  if (session->IsCommonRootDirectory(rootIdx))
  {
    Verbose(fmt::format(T_("Creating fndb for common root directory ({0})..."), Q_(root)));
  }
  else
  {
    Verbose(fmt::format(T_("Creating fndb for user root directory ({0})..."), Q_(root)));
  }
  PrintOnly(fmt::format("fndbcreate {} {}", Q_(fndbPath), Q_(root)));
  if (!printOnly)
  {
    Fndb::Create(fndbPath, root, this);
  }
}

void IniTeXMFApp::UpdateFilenameDatabase(unsigned root)
{
  UpdateFilenameDatabase(session->GetRootDirectoryPath(root));
}

void IniTeXMFApp::ListFormats()
{
  for (const FormatInfo& formatInfo : session->GetFormats())
  {
    cout << fmt::format("{} ({})", formatInfo.key, formatInfo.description) << endl;
  }
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
  LinkCategoryOptions linkCategories;
  linkCategories.Set();
  ManageLinks(linkCategories, true, false);
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

void IniTeXMFApp::RemoveFndb()
{
  session->UnloadFilenameDatabase();
  size_t nRoots = session->GetNumberOfTEXMFRoots();
  for (unsigned r = 0; r < nRoots; ++r)
  {
    PathName path = session->GetFilenameDatabasePathName(r);
    PrintOnly(fmt::format("rm {}", Q_(path)));
    if (!printOnly && File::Exists(path))
    {
      Verbose(fmt::format(T_("Removing fndb ({0})..."), Q_(path)));
      File::Delete(path, { FileDeleteOption::TryHard });
    }
    PathName changeFile = path;
    changeFile.SetExtension(MIKTEX_FNDB_CHANGE_FILE_SUFFIX);
    PrintOnly(fmt::format("rm {}", Q_(changeFile)));
    if (!printOnly && File::Exists(changeFile))
    {
      Verbose(fmt::format(T_("Removing fndb change file ({0})..."), Q_(changeFile)));
      File::Delete(changeFile, { FileDeleteOption::TryHard });
    }
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

void IniTeXMFApp::RunMakeTeX(const string& makeProg, const vector<string>& arguments)
{
  PathName exe;

  if (!session->FindFile(makeProg, FileType::EXE, exe))
  {
    FatalError(fmt::format(T_("The {0} executable could not be found."), Q_(makeProg)));
  }

  vector<string> xArguments{ makeProg };

  xArguments.insert(xArguments.end(), arguments.begin(), arguments.end());

  if (printOnly)
  {
    xArguments.push_back("--print-only");
  }

  if (verbose)
  {
    xArguments.push_back("--verbose");
  }

  if (quiet)
  {
    xArguments.push_back("--quiet");
  }

  if (session->IsAdminMode())
  {
    xArguments.push_back("--admin");
  }

  switch (enableInstaller)
  {
  case TriState::True:
    xArguments.push_back("--enable-installer");
    break;
  case TriState::False:
    xArguments.push_back("--disable-installer");
    break;
  default:
    break;
  }

  xArguments.push_back("--miktex-disable-maintenance");
  xArguments.push_back("--miktex-disable-diagnose");

  LOG4CXX_INFO(logger, "running: " << CommandLineBuilder(xArguments));
  RunProcess(exe, xArguments);
}

void IniTeXMFApp::MakeFormatFile(const string& formatKey)
{
  if (find(formatsMade.begin(), formatsMade.end(), formatKey) != formatsMade.end())
  {
    return;
  }

  FormatInfo formatInfo;
  if (!session->TryGetFormatInfo(formatKey, formatInfo))
  {
    FatalError(fmt::format(T_("Unknown format: {0}"), Q_(formatKey)));
  }

  string maker;

  vector<string> arguments;

  if (formatInfo.compiler == "mf")
  {
    maker = MIKTEX_MAKEBASE_EXE;
  }
  else
  {
    maker = MIKTEX_MAKEFMT_EXE;
    arguments.push_back("--engine="s + formatInfo.compiler);
  }

  arguments.push_back("--dest-name="s + formatInfo.name);

  if (!formatInfo.preloaded.empty())
  {
    if (PathName::Compare(formatInfo.preloaded, formatKey) == 0)
    {
      LOG4CXX_FATAL(logger, T_("Rule recursion detected for: ") << formatKey);
      FatalError(fmt::format(T_("Format '{0}' cannot be built."), formatKey));
    }
    // RECURSION
    MakeFormatFile(formatInfo.preloaded);
    arguments.push_back("--preload="s + formatInfo.preloaded);
  }

  if (PathName(formatInfo.inputFile).HasExtension(".ini"))
  {
    arguments.push_back("--no-dump");
  }

  arguments.push_back(formatInfo.inputFile);

  if (!formatInfo.arguments.empty())
  {
    arguments.push_back("--engine-option="s + formatInfo.arguments);
  }

  RunMakeTeX(maker, arguments);

  formatsMade.push_back(formatKey);
}

void IniTeXMFApp::MakeFormatFiles(const vector<string>& formats)
{
  if (formats.empty())
  {
    for (const FormatInfo& formatInfo : session->GetFormats())
    {
      if (!formatInfo.exclude)
      {
        MakeFormatFile(formatInfo.key);
      }
    }
  }
  else
  {
    for (const string& fmt : formats)
    {
      MakeFormatFile(fmt);
    }
  }
}

void IniTeXMFApp::MakeFormatFilesByName(const vector<string>& formatsByName, const string& engine)
{
  for (const string& name : formatsByName)
  {
    bool done = false;
    for (const FormatInfo& formatInfo : session->GetFormats())
    {
      if (PathName::Compare(formatInfo.name, name) == 0 && (engine.empty()
        || (Utils::EqualsIgnoreCase(formatInfo.compiler, engine))))
      {
        MakeFormatFile(formatInfo.key);
        done = true;
      }
    }
    if (!done)
    {
      if (engine.empty())
      {
        FatalError(fmt::format(T_("Unknown format name: {0}"), Q_(name)));
      }
      else
      {
        FatalError(fmt::format(T_("Unknown format name/engine: {0}/{1}"), Q_(name), engine));
      }
    }
  }
}

void IniTeXMFApp::ManageLink(const FileLink& fileLink, bool supportsHardLinks, bool isRemoveRequested, bool allowOverwrite)
{
  LinkType linkType = fileLink.linkType;
  if (linkType == LinkType::Hard && !supportsHardLinks)
  {
    linkType = LinkType::Copy;
  }
  for (const string& linkName : fileLink.linkNames)
  {
    FileExistsOptionSet fileExistsOptions;
#if defined(MIKTEX_UNIX)
    fileExistsOptions += FileExistsOption::SymbolicLink;
#endif
    if (File::Exists(PathName(linkName), fileExistsOptions))
    {
      if (!isRemoveRequested && (!allowOverwrite || (linkType == LinkType::Copy && File::Equals(PathName(fileLink.target), PathName(linkName)))))
      {
        continue;
      }
#if defined(MIKTEX_UNIX)
      if (File::IsSymbolicLink(PathName(linkName)))
      {
        PathName linkTarget = File::ReadSymbolicLink(PathName(linkName));
	string linkTargetFileName = linkTarget.GetFileName().ToString();
        bool isMiKTeXSymlinked = linkTargetFileName.find(MIKTEX_PREFIX) == 0 || PathName(linkTargetFileName) == PathName(fileLink.target).GetFileName();
        if (!isMiKTeXSymlinked)
        {
          if (File::Exists(PathName(linkTarget)))
          {
            LOG4CXX_WARN(logger, Q_(linkName) << " already symlinked to " << Q_(linkTarget));
            continue;
          }
          else
          {
            LOG4CXX_TRACE(logger, Q_(linkName) << " is symlinked to non-existing " << Q_(linkTarget));
          }
        }
      }
#endif
      PrintOnly(fmt::format("rm {}", Q_(linkName)));
      if (!printOnly)
      {
        File::Delete(PathName(linkName), { FileDeleteOption::TryHard, FileDeleteOption::UpdateFndb });
      }
    }
    if (isRemoveRequested)
    {
      continue;
    }
    PathName sourceDirectory(linkName);
    sourceDirectory.RemoveFileSpec();
    if (!Directory::Exists(sourceDirectory) && !printOnly)
    {
      Directory::Create(sourceDirectory);
    }
    switch (linkType)
    {
    case LinkType::Symbolic:
      {
        const char* target = Utils::GetRelativizedPath(fileLink.target.c_str(), sourceDirectory.GetData());
        if (target == nullptr)
        {
          target = fileLink.target.c_str();
        }
        PrintOnly(fmt::format("ln -s {} {}", Q_(linkName), Q_(target)));
        if (!printOnly)
        {
          File::CreateLink(PathName(target), PathName(linkName), { CreateLinkOption::UpdateFndb, CreateLinkOption::Symbolic });
        }
        break;
      }
    case LinkType::Hard:
      PrintOnly(fmt::format("ln {} {}", Q_(fileLink.target), Q_(linkName)));
      if (!printOnly)
      {
        File::CreateLink(PathName(fileLink.target), PathName(linkName), { CreateLinkOption::UpdateFndb });
      }
      break;
    case LinkType::Copy:
      PrintOnly(fmt::format("cp {} {}", Q_(fileLink.target), Q_(linkName)));
      if (!printOnly)
      {
        File::Copy(PathName(fileLink.target), PathName(linkName), { FileCopyOption::UpdateFndb });
      }
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
    if (logStream.is_open())
    {
      logStream << linkName << "\n";
    }
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
    for (const PathName& r : roots)
    {
      UpdateFilenameDatabase(r);
    }
  }
}

#if defined(MIKTEX_WINDOWS)
struct ShellFileType {
  const char* lpszComponent;
  const char* lpszExtension;
  const char* lpszUserFriendlyName;
  const char* lpszExecutable;
  int iconIndex;
  bool takeOwnership;
  const char* lpszVerb;
  const char* lpszCommandArgs;
  const char* lpszDdeArgs;
} const shellFileTypes[] = {
  "asy", ".asy", "Asymptote File", MIKTEX_ASY_EXE, -2, false, "open", "-cd \"%w\" \"%1\"", nullptr,
  "bib", ".bib", "BibTeX Database", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
  "cls", ".cls", "LaTeX Class", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
  "dtx", ".dtx", "LaTeX Macros", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
  "dvi", ".dvi", "DVI File", MIKTEX_YAP_EXE, 1, false, "open", "/dde", "[open(\"%1\")]",
  "dvi", nullptr, nullptr, MIKTEX_YAP_EXE, INT_MAX, false, "print", "/dde", "[print(\"%1\")]",
  "dvi", nullptr, nullptr, MIKTEX_YAP_EXE, INT_MAX, false, "printto", "/dde", "[printto(\"%1\",\"%2\",\"%3\",\"%4\")]",
  "ltx", ".ltx", "LaTeX Document", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
  "pdf", ".pdf", "PDF File", MIKTEX_TEXWORKS_EXE, INT_MAX, false, "open", "\"%1\"", nullptr,
  "sty", ".sty", "LaTeX Style", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
  "tex", ".tex", "TeX Document", MIKTEX_TEXWORKS_EXE, -2, false, "open", "\"%1\"", nullptr,
};

void IniTeXMFApp::RegisterShellFileTypes(bool reg)
{
  for (const ShellFileType& sft : shellFileTypes)
  {
    string progId = Utils::MakeProgId(sft.lpszComponent);
    if (reg)
    {
      PathName exe;
      if (sft.lpszExecutable != nullptr && !session->FindFile(sft.lpszExecutable, FileType::EXE, exe))
      {
        FatalError(fmt::format(T_("Could not find {0}."), sft.lpszExecutable));
      }
      string command;
      if (sft.lpszExecutable != nullptr && sft.lpszCommandArgs != nullptr)
      {
        command = '\"';
        command += exe.ToDos().ToString();
        command += "\" ";
        command += sft.lpszCommandArgs;
      }
      string iconPath;
      if (sft.lpszExecutable != nullptr && sft.iconIndex != INT_MAX)
      {
        iconPath += exe.ToDos().ToString();
        iconPath += ",";
        iconPath += std::to_string(sft.iconIndex);
      }
      if (sft.lpszUserFriendlyName != nullptr || !iconPath.empty())
      {
        Utils::RegisterShellFileType(progId, sft.lpszUserFriendlyName == nullptr ? "" : sft.lpszUserFriendlyName, iconPath);
      }
      if (sft.lpszVerb != nullptr && (!command.empty() || sft.lpszDdeArgs != nullptr))
      {
          Utils::RegisterShellVerb(progId, sft.lpszVerb, command, sft.lpszDdeArgs == nullptr ? "" : sft.lpszDdeArgs);
      }
      if (sft.lpszExtension != nullptr)
      {
        LOG4CXX_INFO(logger, "registering file extension: " << sft.lpszExtension);
        Utils::RegisterShellFileAssoc(sft.lpszExtension, progId, sft.takeOwnership);
      }
    }
    else
    {
      Utils::UnregisterShellFileType(progId);
      if (sft.lpszExtension != nullptr)
      {
        LOG4CXX_INFO(logger, "unregistering file extension: " << sft.lpszExtension);
        Utils::UnregisterShellFileAssoc(sft.lpszExtension, progId);
      }
    }
  }
}
#endif

void IniTeXMFApp::ModifyPath()
{
#if defined(MIKTEX_WINDOWS)
  Utils::CheckPath(true);
#else
  // TODO: check path
  UNIMPLEMENTED();
#endif
}

// TODO: extra source file;
// TODO: better: configuration file (miktex-links.ini)
vector<FileLink> miktexFileLinks =
{
#if defined(MIKTEX_WINDOWS)
  { "arctrl" MIKTEX_EXE_FILE_SUFFIX, { "pdfclose", "pdfdde", "pdfopen" } },
#endif
  { "cjklatex" MIKTEX_EXE_FILE_SUFFIX, { "bg5pluslatex", "bg5pluspdflatex", "bg5latex", "bg5pdflatex", "bg5platex", "bg5ppdflatex", "cef5latex", "cef5pdflatex", "ceflatex", "cefpdflatex", "cefslatex", "cefspdflatex", "gbklatex", "gbkpdflatex", "sjislatex", "sjispdflatex" } },

  { MIKTEX_AFM2TFM_EXE, { "afm2tfm" } },
#if defined(MIKTEX_WINDOWS)
  { MIKTEX_ASY_EXE, { "asy" } },
#endif
  { MIKTEX_AUTOSP_EXE, { "autosp" } },
  { MIKTEX_AXOHELP_EXE,{ "axohelp" } },
  { MIKTEX_BG5CONV_EXE, { "bg5conv" } },
  { MIKTEX_BIBSORT_EXE, { "bibsort" } },
  { MIKTEX_BIBTEX8_EXE, { "bibtex8" } },
  { MIKTEX_BIBTEXU_EXE, { "bibtexu" } },
  { MIKTEX_BIBTEX_EXE, { "bibtex" } },
  { MIKTEX_CEF5CONV_EXE, { "cef5conv" } },
  { MIKTEX_CEFCONV_EXE, { "cefconv" } },
  { MIKTEX_CEFSCONV_EXE, { "cefsconv" } },
  { MIKTEX_CHKTEX_EXE, { "chktex" } },
  { MIKTEX_CTANGLE_EXE,{ "ctangle" } },
  { MIKTEX_CWEAVE_EXE,{ "cweave" } },
  { MIKTEX_DEVNAG_EXE, { "devnag" } },
  { MIKTEX_DVICOPY_EXE, { "dvicopy" } },
  { MIKTEX_DVIPDFMX_EXE, { "dvipdfm", "dvipdfmx", "ebb", "extractbb", "xbb", "xdvipdfmx", MIKTEX_XDVIPDFMX_EXE } },
  { MIKTEX_DVIPDFT_EXE,{ "dvipdft" } },
  { MIKTEX_DVIPNG_EXE, { "dvipng" } },
  { MIKTEX_DVIPS_EXE, { "dvips" } },
  { MIKTEX_DVISVGM_EXE, { "dvisvgm" } },
  { MIKTEX_DVITYPE_EXE, { "dvitype" } },
  { MIKTEX_EPSFFIT_EXE, { "epsffit" } },
  { MIKTEX_EPSTOPDF_EXE,{ "epstopdf" } },
  { MIKTEX_EXTCONV_EXE, { "extconv" } },
  { MIKTEX_FRIBIDIXETEX_EXE, { "fribidixetex" } },
  { MIKTEX_GFTODVI_EXE, { "gftodvi" } },
  { MIKTEX_GFTOPK_EXE, { "gftopk" } },
  { MIKTEX_GFTYPE_EXE, { "gftype" } },
  { MIKTEX_GREGORIO_EXE, { "gregorio" } },
  { MIKTEX_HBF2GF_EXE, { "hbf2gf" } },
  { MIKTEX_LACHECK_EXE, { "lacheck" } },
  { MIKTEX_MAKEBASE_EXE, { "makebase" } },
  { MIKTEX_MAKEFMT_EXE, { "makefmt" } },
  { MIKTEX_MAKEINDEX_EXE, { "makeindex" } },
  { MIKTEX_MAKEPK_EXE, { "makepk" } },
  { MIKTEX_MAKETFM_EXE, { "maketfm" } },
  { MIKTEX_MFT_EXE, { "mft" } },
  { MIKTEX_MF_EXE, { "mf", "inimf", "virmf" } },
  { MIKTEX_MKOCP_EXE, { "mkocp" } },
  { MIKTEX_MPOST_EXE, { "dvitomp", "mpost" } },
  { MIKTEX_ODVICOPY_EXE, { "odvicopy" } },
  { MIKTEX_OFM2OPL_EXE, { "ofm2opl" } },
  { MIKTEX_OPL2OFM_EXE, { "opl2ofm" } },
  { MIKTEX_OTP2OCP_EXE, { "otp2ocp" } },
  { MIKTEX_OUTOCP_EXE, { "outocp" } },
  { MIKTEX_OVF2OVP_EXE, { "ovf2ovp" } },
  { MIKTEX_OVP2OVF_EXE, { "ovp2ovf" } },
  { MIKTEX_PREFIX "patgen" MIKTEX_EXE_FILE_SUFFIX, { "patgen"} },
  { MIKTEX_PDFTEX_EXE, { "pdftex", MIKTEX_LATEX_EXE, MIKTEX_PDFLATEX_EXE } },
  { MIKTEX_PDFTOSRC_EXE, { "pdftosrc" } },
  { MIKTEX_PK2BM_EXE, { "pk2bm" } },
  { MIKTEX_PLTOTF_EXE, { "pltotf" } },
  { MIKTEX_PMXAB_EXE, { "pmxab" } },
  { MIKTEX_POOLTYPE_EXE, { "pooltype" } },
  { MIKTEX_PREPMX_EXE, { "prepmx" } },
  { MIKTEX_PS2PK_EXE, { "ps2pk" } },
  { MIKTEX_PSBOOK_EXE, { "psbook" } },
  { MIKTEX_PSNUP_EXE, { "psnup" } },
  { MIKTEX_PSRESIZE_EXE, { "psresize" } },
  { MIKTEX_PSSELECT_EXE, { "psselect" } },
  { MIKTEX_PSTOPS_EXE, { "pstops" } },
  { MIKTEX_REBAR_EXE, { "rebar" } },
  { MIKTEX_SCOR2PRT_EXE, { "scor2prt" } },
  { MIKTEX_SJISCONV_EXE, { "sjisconv" } },
  { MIKTEX_SYNCTEX_EXE, { "synctex" } },
  { MIKTEX_T4HT_EXE, { "t4ht" } },
  { MIKTEX_TANGLE_EXE, { "tangle" } },
  { MIKTEX_PREFIX "tex2aspc" MIKTEX_EXE_FILE_SUFFIX, { "tex2aspc" } },
  { MIKTEX_TEX4HT_EXE, { "tex4ht" } },
#if defined(MIKTEX_QT)
  { MIKTEX_TEXWORKS_EXE, { "texworks" } },
#endif
  { MIKTEX_TEX_EXE, { "tex", "initex", "virtex" } },
  { MIKTEX_TFTOPL_EXE, { "tftopl" } },
  { MIKTEX_TTF2AFM_EXE, { "ttf2afm" } },
  { MIKTEX_TTF2PK_EXE, { "ttf2pk" } },
  { MIKTEX_TTF2TFM_EXE, { "ttf2tfm" } },
  { MIKTEX_PREFIX "upmendex" MIKTEX_EXE_FILE_SUFFIX, { "upmendex"} },
  { MIKTEX_VFTOVP_EXE, { "vftovp" } },
  { MIKTEX_VPTOVF_EXE, { "vptovf" } },
  { MIKTEX_WEAVE_EXE, { "weave" } },
  { MIKTEX_XETEX_EXE, { "xetex", MIKTEX_XELATEX_EXE } },
#if defined(WITH_RUNGS)
  { MIKTEX_RUNGS_EXE, { "rungs" } },
#endif
#if defined(WITH_KPSEWHICH)
  { MIKTEX_KPSEWHICH_EXE, { "kpsewhich" } },
#endif
#if defined(MIKTEX_MACOS_BUNDLE)
  { MIKTEX_INITEXMF_EXE, { MIKTEX_INITEXMF_EXE }},
  { MIKTEX_MPM_EXE, { MIKTEX_MPM_EXE } },
  { MIKTEX_TEXIFY_EXE, { MIKTEX_TEXIFY_EXE } },
  { "mthelp" MIKTEX_EXE_FILE_SUFFIX, { "mthelp" MIKTEX_EXE_FILE_SUFFIX } },
#endif
#if defined(WITH_MKTEXLSR)
  { MIKTEX_INITEXMF_EXE, { "mktexlsr" }, LinkType::Copy },
#endif
#if defined(WITH_TEXHASH)
  { MIKTEX_INITEXMF_EXE, { "texhash" }, LinkType::Copy },
#endif
#if defined(WITH_TEXLINKS)
  { MIKTEX_INITEXMF_EXE, { "texlinks" }, LinkType::Copy },
#endif
#if defined(WITH_UPDMAP)
  { "mkfntmap" MIKTEX_EXE_FILE_SUFFIX, { "updmap" } },
#endif
#if defined(WITH_TEXDOC)
  { "mthelp" MIKTEX_EXE_FILE_SUFFIX, { "texdoc" } },
#endif
#if defined(WITH_POPPLER_UTILS)
  { MIKTEX_PDFDETACH_EXE, { "pdfdetach" } },
  { MIKTEX_PDFFONTS_EXE, { "pdffonts" } },
  { MIKTEX_PDFIMAGES_EXE, { "pdfimages" } },
  { MIKTEX_PDFINFO_EXE, { "pdfinfo" } },
  { MIKTEX_PDFSEPARATE_EXE, { "pdfseparate" } },
#if 0
  { MIKTEX_PDFSIG_EXE, { "pdfsig" } },
#endif
  { MIKTEX_PDFTOCAIRO_EXE,{ "pdftocairo" } },
  { MIKTEX_PDFTOHTML_EXE, { "pdftohtml" } },
  { MIKTEX_PDFTOPPM_EXE, { "pdftoppm" } },
  { MIKTEX_PDFTOPS_EXE, { "pdftops" } },
  { MIKTEX_PDFTOTEXT_EXE, { "pdftotext" } },
  { MIKTEX_PDFUNITE_EXE, { "pdfunite" } },
#endif
#if defined(WITH_FONTCONFIG_UTILS)
  { MIKTEX_FC_CACHE_EXE, { "fc-cache" } },
  { MIKTEX_PREFIX "fc-cat" MIKTEX_EXE_FILE_SUFFIX, { "fc-cat" } },
  { MIKTEX_PREFIX "fc-conflist" MIKTEX_EXE_FILE_SUFFIX, { "fc-conflist" } },
  { MIKTEX_PREFIX "fc-list" MIKTEX_EXE_FILE_SUFFIX, { "fc-list" } },
  { MIKTEX_PREFIX "fc-match" MIKTEX_EXE_FILE_SUFFIX, { "fc-match" } },
  { MIKTEX_PREFIX "fc-pattern" MIKTEX_EXE_FILE_SUFFIX, { "fc-pattern" } },
  { MIKTEX_PREFIX "fc-query" MIKTEX_EXE_FILE_SUFFIX, { "fc-query" } },
  { MIKTEX_PREFIX "fc-scan" MIKTEX_EXE_FILE_SUFFIX, { "fc-scan" } },
  { MIKTEX_PREFIX "fc-validate" MIKTEX_EXE_FILE_SUFFIX, { "fc-validate" } },
#endif
#if defined(WITH_LCDF_TYPETOOLS)
  { MIKTEX_PREFIX "cfftot1" MIKTEX_EXE_FILE_SUFFIX, { "cfftot1" } },
  { MIKTEX_PREFIX "mmafm" MIKTEX_EXE_FILE_SUFFIX, { "mmafm" } },
  { MIKTEX_PREFIX "mmpfb" MIKTEX_EXE_FILE_SUFFIX, { "mmpfb" } },
  { MIKTEX_PREFIX "otfinfo" MIKTEX_EXE_FILE_SUFFIX, { "otfinfo" } },
  { MIKTEX_PREFIX "otftotfm" MIKTEX_EXE_FILE_SUFFIX, { "otftotfm" } },
  { MIKTEX_PREFIX "t1dotlessj" MIKTEX_EXE_FILE_SUFFIX, { "t1dotlessj" } },
  { MIKTEX_PREFIX "t1lint" MIKTEX_EXE_FILE_SUFFIX, { "t1lint" } },
  { MIKTEX_PREFIX "t1rawafm" MIKTEX_EXE_FILE_SUFFIX, { "t1rawafm" } },
  { MIKTEX_PREFIX "t1reencode" MIKTEX_EXE_FILE_SUFFIX, { "t1reencode" } },
  { MIKTEX_PREFIX "t1testpage" MIKTEX_EXE_FILE_SUFFIX, { "t1testpage" } },
  { MIKTEX_PREFIX "ttftotype42" MIKTEX_EXE_FILE_SUFFIX, { "ttftotype42" } },
#endif
#if defined(MIKTEX_WINDOWS)
  { MIKTEX_PREFIX "zip" MIKTEX_EXE_FILE_SUFFIX, {"zip"} },
#endif
#if defined(MIKTEX_WINDOWS)
  { MIKTEX_CONSOLE_EXE, { MIKTEX_TASKBAR_ICON_EXE, MIKTEX_UPDATE_EXE } },
  { MIKTEX_CONSOLE_ADMIN_EXE,{ MIKTEX_UPDATE_ADMIN_EXE } },
#endif
};

vector<FileLink> lua52texLinks =
{
  { MIKTEX_LUAHBTEX_EXE, { "luahbtex", MIKTEX_LUALATEX_EXE } },
  { MIKTEX_LUATEX_EXE, { MIKTEX_PREFIX "texlua", MIKTEX_PREFIX "texluac", "luatex", "texlua", "texluac" } },
};

vector<FileLink> IniTeXMFApp::CollectLinks(LinkCategoryOptions linkCategories)
{
  vector<FileLink> result;
  PathName linkTargetDirectory = session->GetSpecialPath(SpecialPath::LinkTargetDirectory);
  PathName pathBinDir = session->GetSpecialPath(SpecialPath::BinDirectory);

  Verbose(fmt::format(T_("Creating links in target directory {0}..."), linkTargetDirectory));

  if (linkCategories[LinkCategory::MiKTeX])
  {
    vector<FileLink> links = miktexFileLinks;
    links.insert(links.end(), lua52texLinks.begin(), lua52texLinks.end());
#if defined(MIKTEX_MACOS_BUNDLE)
    PathName console(session->GetSpecialPath(SpecialPath::MacOsDirectory) / PathName(MIKTEX_MACOS_BUNDLE_NAME));
    links.push_back(FileLink(console.ToString(), { MIKTEX_CONSOLE_EXE }, LinkType::Symbolic));
#endif
    for (const FileLink& fileLink : links)
    {
      PathName targetPath;
      if (PathNameUtil::IsAbsolutePath(fileLink.target))
      {
        targetPath = fileLink.target;
      }
      else
      {
        targetPath = pathBinDir / PathName(fileLink.target);
      }
      string extension = targetPath.GetExtension();
      if (File::Exists(targetPath))
      {
        vector<string> linkNames;
        for (const string& linkName : fileLink.linkNames)
        {
          PathName linkPath = linkTargetDirectory / PathName(linkName);
          if (linkPath == targetPath)
          {
            continue;
          }
          if (!extension.empty())
          {
            linkPath.AppendExtension(extension);
          }
          linkNames.push_back(linkPath.ToString());
        }
        result.push_back(FileLink(targetPath.ToString(), linkNames, fileLink.linkType));
      }
      else
      {
        Warning(fmt::format(T_("The link target {0} does not exist."), Q_(targetPath)));
      }
    }
  }

  if (linkCategories[LinkCategory::Formats])
  {
    for (const FormatInfo& formatInfo : session->GetFormats())
    {
      if (formatInfo.noExecutable)
      {
        continue;
      }
      string engine = formatInfo.compiler;
      PathName enginePath;
      if (!session->FindFile(string(MIKTEX_PREFIX) + engine, FileType::EXE, enginePath))
      {
        Warning(fmt::format(T_("The {0} executable could not be found."), engine));
        continue;
      }
      PathName exePath(linkTargetDirectory, PathName(formatInfo.name));
      if (strlen(MIKTEX_EXE_FILE_SUFFIX) > 0)
      {
        exePath.AppendExtension(MIKTEX_EXE_FILE_SUFFIX);
      }
      if (!(enginePath == exePath))
      {
        result.push_back(FileLink(enginePath.ToString(), { exePath.ToString() }));
      }
    }
  }

  if (linkCategories[LinkCategory::Scripts])
  {

    PathName scriptsIni;
    if (!session->FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
    {
      FatalError(T_("Script configuration file not found."));
    }
    unique_ptr<Cfg> config(Cfg::Create());
    config->Read(scriptsIni, true);
    for (const shared_ptr<Cfg::Key>& key : *config)
    {
      PathName wrapper = session->GetSpecialPath(SpecialPath::InternalBinDirectory);
      wrapper.AppendDirectoryDelimiter();
      wrapper.Append("run", false);
      wrapper.Append(key->GetName(), false);
      wrapper.Append(MIKTEX_EXE_FILE_SUFFIX, false);
      if (!File::Exists(wrapper))
      {
        continue;
      }
      for (const shared_ptr<Cfg::Value>& v : *key)
      {
        string name = v->GetName();
        if (EndsWith(name, "[]"))
        {
          continue;
        }
        PathName pathExe(linkTargetDirectory, PathName(name));
        if (strlen(MIKTEX_EXE_FILE_SUFFIX) > 0)
        {
          pathExe.AppendExtension(MIKTEX_EXE_FILE_SUFFIX);
        }
        result.push_back(FileLink(wrapper.ToString(), { pathExe.ToString() }));
      }
    }
  }

  return result;
}

void IniTeXMFApp::ManageLinks(LinkCategoryOptions linkCategories, bool remove, bool force)
{
  PathName pathBinDir = session->GetSpecialPath(SpecialPath::BinDirectory);
  PathName internalBinDir = session->GetSpecialPath(SpecialPath::InternalBinDirectory);

  // TODO: MIKTEX_ASSERT(pathBinDir.GetMountPoint() == internalBinDir.GetMountPoint());

  bool supportsHardLinks = Utils::SupportsHardLinks(pathBinDir);

  if (!remove && !Directory::Exists(pathBinDir))
  {
    Directory::Create(pathBinDir);
  }

  if (!remove && logStream.is_open())
  {
    logStream << "[files]" << "\n";
  }

  for (const FileLink& fileLink : CollectLinks(linkCategories))
  {
    ManageLink(fileLink, supportsHardLinks, remove, force);
  }
}

#if defined(MIKTEX_UNIX)
void IniTeXMFApp::MakeFilesExecutable()
{
  PathName scriptsIni;
  if (!session->FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
  {
    FatalError(T_("Script configuration file not found."));
  }
  unique_ptr<Cfg> config(Cfg::Create());
  config->Read(scriptsIni, true);
  AutoRestore<TriState> x(enableInstaller);
  enableInstaller = TriState::False;
  for (const shared_ptr<Cfg::Key>& key : *config)
  {
    if (key->GetName() != "sh" && key->GetName() != "exe")
    {
      continue;
    }
    for (const shared_ptr<Cfg::Value>& val : *key)
    {
      if (EndsWith(val->GetName(), "[]"))
      {
        continue;
      }
      PathName scriptPath;
      if (!session->FindFile(session->Expand(val->AsString()), MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptPath))
      {
        continue;
      }
      if (session->GetRootDirectories()[session->DeriveTEXMFRoot(scriptPath)].IsCommon() && !session->IsAdminMode())
      {
        continue;
      }
      File::SetAttributes(scriptPath, File::GetAttributes(scriptPath) + FileAttribute::Executable);
    }
  }
}
#endif

void IniTeXMFApp::MakeLanguageDat(bool force)
{
  Verbose(T_("Creating language.dat, language.dat.lua and language.def..."));

  if (printOnly)
  {
    return;
  }

  PathName languageDatPath = session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_LANGUAGE_DAT);
  ofstream languageDat = File::CreateOutputStream(languageDatPath);

  PathName languageDatLuaPath = session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_LANGUAGE_DAT_LUA);
  ofstream languageDatLua = File::CreateOutputStream(languageDatLuaPath);

  PathName languageDefPath = session->GetSpecialPath(SpecialPath::ConfigRoot) / PathName(MIKTEX_PATH_LANGUAGE_DEF);
  ofstream languageDef = File::CreateOutputStream(languageDefPath);

  languageDatLua << "return {" << "\n";
  languageDef << "%% e-TeX V2.2" << "\n";

  for (const LanguageInfo& languageInfo : session->GetLanguages())
  {
    if (languageInfo.exclude)
    {
      continue;
    }

    PathName loaderPath;
    if (!session->FindFile(languageInfo.loader, "%r/tex//", loaderPath))
    {
      continue;
    }

    // language.dat
    languageDat << languageInfo.key << " " << languageInfo.loader << "\n";
    for (const string& synonym : StringUtil::Split(languageInfo.synonyms, ','))
    {
      languageDat << "=" << synonym << "\n";
    }

    // language.def
    languageDef << "\\addlanguage{" << languageInfo.key << "}{" << languageInfo.loader << "}{}{" << languageInfo.lefthyphenmin << "}{" << languageInfo.righthyphenmin << "}" << "\n";

    // language.dat.lua
    languageDatLua << "\t['" << languageInfo.key << "'] = {" << "\n";
    languageDatLua << "\t\tloader='" << languageInfo.loader << "'," << "\n";
    languageDatLua << "\t\tlefthyphenmin=" << languageInfo.lefthyphenmin << "," << "\n";
    languageDatLua << "\t\trighthyphenmin=" << languageInfo.righthyphenmin << "," << "\n";
    languageDatLua << "\t\tsynonyms={ ";
    int nSyn = 0;
    for (const string& synonym : StringUtil::Split(languageInfo.synonyms, ','))
    {
      languageDatLua << (nSyn > 0 ? "," : "") << "'" << synonym << "'";
      nSyn++;
    }
    languageDatLua << " }," << "\n";
    languageDatLua << "\t\tpatterns='" << languageInfo.patterns << "'," << "\n";
    languageDatLua << "\t\thyphenation='" << languageInfo.hyphenation << "'," << "\n";
    if (!languageInfo.luaspecial.empty())
    {
      languageDatLua << "\t\tspecial='" << languageInfo.luaspecial << "'," << "\n";
    }
    languageDatLua << "\t}," << "\n";
  }

  languageDatLua << "}" << "\n";

  languageDatLua.close();
  Fndb::Add({ {languageDatLuaPath} });

  languageDef.close();
  Fndb::Add({ {languageDefPath} });

  languageDat.close();
  Fndb::Add({ {languageDatPath} });
}

void IniTeXMFApp::MakeMaps(bool force)
{
  PathName pathMkfontmap;
  if (!session->FindFile("mkfntmap", FileType::EXE, pathMkfontmap))
  {
    FatalError(T_("The mkfntmap executable could not be found."));
  }
  vector<string> arguments{ "mkfntmap" };
  if (verbose)
  {
    arguments.push_back("--verbose");
  }
  if (session->IsAdminMode())
  {
    arguments.push_back("--admin");
  }
  if (force)
  {
    arguments.push_back("--force");
  }
  switch (enableInstaller)
  {
  case TriState::True:
    arguments.push_back("--enable-installer");
    break;
  case TriState::False:
    arguments.push_back("--disable-installer");
    break;
  default:
    break;
  }
  arguments.push_back("--miktex-disable-maintenance");
  arguments.push_back("--miktex-disable-diagnose");
  if (printOnly)
  {
    PrintOnly(CommandLineBuilder(arguments).ToString());
  }
  else
  {
    LOG4CXX_INFO(logger, "running: " << CommandLineBuilder(arguments));
    RunProcess(pathMkfontmap, arguments);
  }
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
  SetupService::WriteReport(cout, { ReportOption::General, ReportOption::RootDirectories, ReportOption::CurrentUser });
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
  vector<string> updateRoots;
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
  bool optMakeLinks = isTexlinksMode;
  LinkCategoryOptions linkCategories;
#if defined(MIKTEX_WINDOWS)
  bool optNoRegistry = false;
#endif
  bool optPortable = false;
  bool optRegisterOtherRoots = false;
  bool optRegisterShellFileTypes = false;
  bool optRemoveLinks = false;
  bool optModifyPath = false;
  bool optReport = false;
  bool optUnRegisterShellFileTypes = false;
  bool optUpdateFilenameDatabase = isMktexlsrMode;
  bool optVersion = false;

  const struct poptOption* aoptions;

  if (isMktexlsrMode)
  {
    aoptions = options_mktexlsr;
  }
  else if (isTexlinksMode)
  {
    aoptions = options_texlinks;
  }
  else
  {
    aoptions = options;
  }

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
      if (optArg.empty())
      {
        linkCategories.Set();
      }
      else if (optArg == "formats")
      {
        linkCategories += LinkCategory::Formats;
      }
      else if (optArg == "miktex")
      {
        linkCategories += LinkCategory::MiKTeX;
      }
      else if (optArg == "scripts")
      {
        linkCategories += LinkCategory::Scripts;
      }
      else
      {
        MIKTEX_FATAL_ERROR(T_("Unknown link category."));
      }      
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

    case OPT_REGISTER_SHELL_FILE_TYPES:

      optRegisterShellFileTypes = true;
      break;

    case OPT_REGISTER_ROOT:

      registerRoots.push_back(PathName(optArg));
      break;

    case OPT_REMOVE_FILE:

      removeFiles.push_back(optArg);
      break;

    case OPT_REMOVE_LINKS:

      optRemoveLinks = true;
      linkCategories.Set();
      break;
      
    case OPT_REPORT:

      optReport = true;
      break;

    case OPT_RMFNDB:

      removeFndb = true;
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

    case OPT_UNREGISTER_SHELL_FILE_TYPES:

      optUnRegisterShellFileTypes = true;
      break;

    case OPT_UPDATE_FNDB:

      optUpdateFilenameDatabase = true;
      if (!optArg.empty())
      {
        updateRoots.push_back(optArg);
      }
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
      << "Copyright (C) 1996-2020 Christian Schenk" << endl
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

  auto setupConfig = session->GetSetupConfig();
  bool isVirgin = !IsValidTimeT(setupConfig.setupDate) && setupConfig.setupVersion == VersionNumber();

  if (isVirgin
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

#if defined(MIKTEX_WINDOWS)
  if (optRegisterShellFileTypes)
  {
    RegisterShellFileTypes(true);
  }
#endif

#if defined(MIKTEX_WINDOWS)
  if (optUnRegisterShellFileTypes)
  {
    RegisterShellFileTypes(false);
  }
#endif

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
    ManageLinks(linkCategories, optRemoveLinks, optForce);
#if defined(MIKTEX_UNIX)
    if (optMakeLinks)
    {
      MakeFilesExecutable();
    }
#endif
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

  if (removeFndb)
  {
    RemoveFndb();
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
    if (updateRoots.empty())
    {
      unsigned nRoots = session->GetNumberOfTEXMFRoots();
      for (unsigned r = 0; r < nRoots; ++r)
      {
        if (session->IsAdminMode())
        {
          if (session->IsCommonRootDirectory(r))
          {
            UpdateFilenameDatabase(r);
          }
          else
          {
            Verbose(fmt::format(T_("Skipping user root directory ({0})..."), Q_(session->GetRootDirectoryPath(r))));
          }
        }
        else
        {
          if (!session->IsCommonRootDirectory(r) || session->IsMiKTeXPortable())
          {
            UpdateFilenameDatabase(r);
          }
          else
          {
            Verbose(fmt::format(T_("Skipping common root directory ({0})..."), Q_(session->GetRootDirectoryPath(r))));
          }
        }
      }
      PackageInfo test;
      bool havePackageDatabase = packageManager->TryGetPackageInfo("miktex-tex", test);
      if (!havePackageDatabase)
      {
        if (enableInstaller == TriState::True)
        {
          EnsureInstaller();
          packageInstaller->UpdateDb({});
        }
        else
        {
          Warning(T_("The local package database does not exist."));
        }
      }
      else
      {
        Verbose(T_("Creating fndb for MPM..."));
        packageManager->CreateMpmFndb();
      }
    }
    else
    {
      for (const string& r : updateRoots)
      {
        UpdateFilenameDatabase(PathName(r));
      }
    }
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
    ListFormats();
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
