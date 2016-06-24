/* initexmf.cpp: MiKTeX configuration utility

   Copyright (C) 1996-2016 Christian Schenk

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

#include "StdAfx.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;
using namespace std;

#define UNUSED_ALWAYS(x)

#define UNIMPLEMENTED() MIKTEX_INTERNAL_ERROR()

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

const char * const TheNameOfTheGame = T_("MiKTeX Configuration Utility");

#define PROGNAME "initexmf"

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(PROGNAME));

enum class LinkType
{
  Hard,
  Symbolic,
  Copy
};

struct FileLink
{
  FileLink(const string & target, const vector<string> & linkNames) :
    target(target),
    linkNames(linkNames)
  {
  }
  FileLink(const string & target, const vector<string> & linkNames, LinkType linkType) :
    target(target),
    linkNames(linkNames),
    linkType(linkType)
  {
  }
  string target;
  vector<string> linkNames;
  LinkType linkType = LinkType::Hard;
};

class ProcessOutput :
  public IRunProcessCallback
{
public:
  void RemoveTrailingNewline()
  {
    if (!output.empty() && output[output.length() - 1] == '\n')
    {
      output.erase(output.length() - 1);
    }
  }
public:
  const string ToString()
  {
    return output;
  }
public:
  bool IsEmpty()
  {
    return output.empty();
  }
public:
  void Clear()
  {
    output = "";
  }
public:
  bool OnProcessOutput(const void * pOutput, size_t n) override
  {
    output.append(reinterpret_cast<const char *>(pOutput), n);
    return true;
  }
private:
  string output;
};

class XmlWriter
{
public:
  void StartDocument()
  {
    cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  }

public:
  void StartElement(const string & name)
  {
    if (freshElement)
    {
      cout << ">";
    }
    cout << "<" << name;
    freshElement = true;
    elements.push(name);
  }

public:
  void AddAttribute(const string & attributeName, const string & attributeValue)
  {
    cout << " " << attributeName << "=\"" << attributeValue << "\"";
  }

public:
  void EndElement()
  {
    if (elements.empty())
    {
      MIKTEX_FATAL_ERROR(T_("No elements on the stack."));
    }
    if (freshElement)
    {
      cout << "/>";
      freshElement = false;
    }
    else
    {
      cout << "</" << elements.top() << ">";
    }
    elements.pop();
  }

public:
  void EndAllElements()
  {
    while (!elements.empty())
    {
      EndElement();
    }
  }

public:
  void Text(const string & text)
  {
    if (freshElement)
    {
      cout << ">";
      freshElement = false;
    }
    for (char ch : text)
    {
      switch (ch)
      {
      case '&':
        cout <<"&amp;";
        break;
      case '<':
        cout << "&lt;";
        break;
      case '>':
        cout << "&gt;";
        break;
      default:
        cout << ch;
        break;
      }
    }
  }

private:
  stack<string> elements;

private:
  bool freshElement = false;
};

static struct
{
  const char * lpszShortcut;
  const char * lpszFile;
}
configShortcuts[] = {
  "pdftex", MIKTEX_PATH_PDFTEX_CFG,
  "dvips", MIKTEX_PATH_CONFIG_PS,
  "dvipdfm", MIKTEX_PATH_DVIPDFM_CONFIG,
  "dvipdfmx", MIKTEX_PATH_DVIPDFMX_CONFIG,
  "updmap", MIKTEX_PATH_UPDMAP_CFG,
};

class IniTeXMFApp :
  public IFindFileCallback,
  public ICreateFndbCallback,
  public IEnumerateFndbCallback,
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
    PathName logDir(session->GetSpecialPath(SpecialPath::DataRoot));
    logDir /= MIKTEX_PATH_MIKTEX_LOG_DIR;
    return logDir;
  }

public:
  void Init(const char * argv0);

public:
  void Finalize();

private:
  void Verbose(const char * lpszFormat, ...);

private:
  void PrintOnly(const char * lpszFormat, ...);

private:
  void Warning(const char * lpszFormat, ...);

private:
  MIKTEXNORETURN void FatalError(const char * lpszFormat, ...);

private:
  void UpdateFilenameDatabase(const PathName & root);

private:
  void UpdateFilenameDatabase(unsigned root);

private:
  void ListFormats();

private:
  void ListMetafontModes();

private:
  void RemoveFndb();

private:
#if defined(MIKTEX_WINDOWS)
  void SetTeXMFRootDirectories(bool noRegistry);
#else
  void SetTeXMFRootDirectories();
#endif

private:
  void RunMakeTeX(const string & makeProg, const CommandLineBuilder & arguments);

private:
  void MakeFormatFile(const string & formatKey);

private:
  void MakeFormatFiles(const vector<string> & formats);

private:
  void MakeFormatFilesByName(const vector<string> & formatsByName, const string & engine);

private:
  void MakeMaps(bool force);

private:
  void CreateConfigFile(const string & relPath, bool edit);

private:
  void SetConfigValue(const string & valueSpec);

private:
  void ShowConfigValue(const string & valueSpec);

private:
  vector<FileLink> CollectLinks(bool overwrite);

private:
  void MakeLinks(bool force);

private:
  void MakeLanguageDat(bool force);

private:
  void RegisterRoots(const vector<PathName> & roots, bool reg);

#if defined(MIKTEX_WINDOWS)
private:
  void RegisterShellFileTypes(bool reg);
#endif

private:
  void ModifyPath();

private:
  void CreateLink(const FileLink & fileLink, bool supportsHardLinks, bool overwrite);

private:
  void ReportMiKTeXVersion();

private:
  void ReportOSVersion();

private:
  void ReportRoots();

private:
  void ReportFndbFiles();

#if defined(MIKTEX_WINDOWS)
private:
  void ReportEnvironmentVariables();
#endif

private:
  void ReportBrokenPackages();

private:
  void WriteReport();

#if !defined(MIKTEX_STANDALONE)
private:
  void Configure();
#endif

private:
  void CreatePortableSetup(const PathName & portableRoot);

public:
  void Run(int argc, const char * argv[]);

private:
  void FindWizards();

private:
  bool InstallPackage(const string & deploymentName, const PathName & trigger, PathName & installRoot) override;

private:
  bool TryCreateFile(const MiKTeX::Core::PathName & fileName, MiKTeX::Core::FileType fileType) override;

private:
  bool ReadDirectory(const char * lpszPath, char * * ppSubDirNames, char * * ppFileNames, char * * ppFileNameInfos) override;

private:
  bool OnProgress(unsigned level, const char * lpszDirectory) override;

private:
  bool OnFndbItem(const char * lpszPath, const char * lpszName, const char * lpszInfo, bool isDirectory) override;

public:
  void ReportLine(const string & str) override;
  
public:
  bool OnRetryableError(const string & message) override;
  
public:
  bool OnProgress(Notification nf) override;

private:
  bool isLog4cxxConfigured = false;

private:
  vector<TraceCallback::TraceMessage> pendingTraceMessages;

public:
  void Trace(const TraceCallback::TraceMessage & traceMessage) override
  {
    if (!isLog4cxxConfigured)
    {
      if (pendingTraceMessages.size() > 100)
      {
        pendingTraceMessages.clear();
      }
      pendingTraceMessages.push_back(traceMessage);
      return;
    }
    FlushPendingTraceMessages();
    TraceInternal(traceMessage);
  }

private:
  void FlushPendingTraceMessages()
  {
    for (const TraceCallback::TraceMessage & msg : pendingTraceMessages)
    {
      TraceInternal(msg);
    }
    pendingTraceMessages.clear();
  }

private:
  void TraceInternal(const TraceCallback::TraceMessage & traceMessage)
  {
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.initexmf.") + traceMessage.facility);

    if (traceMessage.streamName == MIKTEX_TRACE_ERROR)
    {
      LOG4CXX_ERROR(logger, traceMessage.message);
    }
    else
    {
      LOG4CXX_TRACE(logger, traceMessage.message);
    }
  }

private:
  PathName enumDir;

private:
  bool csv = false;

private:
  bool xml = false;

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
  bool adminMode = false;

private:
  StartupConfig startupConfig;

private:
  vector<string> formatsMade;

private:
  StreamWriter logStream;

private:
  bool updateWizardRunning = false;

private:
  bool setupWizardRunning = false;

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
  XmlWriter xmlWriter;

private:
  static const struct poptOption aoption_user[];

private:
  static const struct poptOption aoption_setup[];

private:
  static const struct poptOption aoption_update[];

private:
  static const struct poptOption aoption_mktexlsr[];

private:
  static const struct poptOption aoption_texlinks[];
};

enum Option
{
  OPT_AAA = 256,

  OPT_CONFIGURE,                // ! MIKTEX_STANDALONE

  OPT_ADMIN,
  OPT_DISABLE_INSTALLER,
  OPT_DUMP,
  OPT_DUMP_BY_NAME,
  OPT_EDIT_CONFIG_FILE,
  OPT_ENABLE_INSTALLER,
  OPT_ENGINE,
  OPT_FORCE,
  OPT_LIST_MODES,
  OPT_MKLINKS,
  OPT_MKMAPS,
  OPT_PRINT_ONLY,
  OPT_REGISTER_ROOT,
  OPT_QUIET,
  OPT_UNREGISTER_ROOT,
  OPT_REPORT,
  OPT_UPDATE_FNDB,
  OPT_USER_ROOTS,
  OPT_VERBOSE,
  OPT_VERSION,

  OPT_ADD_FILE,                 // <experimental/>
  OPT_CREATE_CONFIG_FILE,       // <experimental/>
  OPT_CSV,                      // <experimental/>
  OPT_LIST_DIRECTORY,           // <experimental/>
  OPT_LIST_FORMATS,             // <experimental/>
  OPT_MODIFY_PATH,              // <experimental/>
  OPT_RECURSIVE,                // <experimental/>
  OPT_REGISTER_SHELL_FILE_TYPES,        // <experimental/>
  OPT_REMOVE_FILE,              // <experimental/>
  OPT_SET_CONFIG_VALUE,         // <experimental/>
  OPT_SHOW_CONFIG_VALUE,                // <experimental/>
  OPT_UNREGISTER_SHELL_FILE_TYPES,      // <experimental/>
  OPT_XML,                      // <experimental/>

  OPT_COMMON_CONFIG,            // <internal/>
  OPT_COMMON_DATA,              // <internal/>
  OPT_COMMON_INSTALL,           // <internal/>
  OPT_COMMON_ROOTS,             // <internal/>
  OPT_MKLANGS,                  // <internal/>
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

const struct poptOption IniTeXMFApp::aoption_user[] = {

  {
    "add-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ADD_FILE,
    T_("Add a file to the file name database."),
    T_("FILE")
  },

  {
    "admin", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ADMIN,
    T_("Run in administrative mode."),
    nullptr
  },

#if !defined(MIKTEX_STANDALONE)
  {
    "configure", 0,
    POPT_ARG_NONE, nullptr,
    OPT_CONFIGURE,
    T_("Configure MiKTeX."),
    nullptr
  },
#endif

  {
    "create-config-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CREATE_CONFIG_FILE,
    T_("Create the specified configuration file."),
    T_("CONFIGFILE")
  },

  {
    "csv", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CSV,
    T_("Print comma-separated values."),
    nullptr
  },

  {
    "disable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_DISABLE_INSTALLER,
    T_("Disable the package installer (do not automatically install missing files)."),
    nullptr
  },

  {
    "dump", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_DUMP,
    T_("Create memory dump files."),
    T_("KEY")
  },

  {
    "dump-by-name", 0,
    POPT_ARG_STRING, nullptr,
    OPT_DUMP_BY_NAME,
    T_("Create a memory dump file by name."),
    T_("NAME")
  },

  {
    "edit-config-file", 0,
    POPT_ARG_STRING, nullptr,
    OPT_EDIT_CONFIG_FILE,
    T_("Open the specified configuration file in an editor. See the manual, for more information."),
    T_("CONFIGFILE")
  },

  {
    "enable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ENABLE_INSTALLER,
    T_("Enable the package installer (automatically install missing files)."),
    nullptr
  },

  {
    "engine", 0,
    POPT_ARG_STRING, nullptr,
    OPT_ENGINE,
    T_("Engine to be used."),
    T_("ENGINE")
  },

  {
    "force", 0,
    POPT_ARG_NONE, nullptr,
    OPT_FORCE,
    T_("Force --mklinks to overwrite existing files."),
    nullptr
  },

  {
    "list-directory", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LIST_DIRECTORY,
    T_("List the FNDB contents of a directory."),
    T_("DIR")
  },

  {
    "list-formats", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_FORMATS,
    T_("List formats."),
    0
  },

  {
    "list-modes", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_MODES,
    T_("List METAFONT modes"),
    nullptr
  },

  {
    "mklangs", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MKLANGS,
    T_("Create language.dat, language.dat.lua and language.def."),
    nullptr
  },

  {
    "mklinks", 0,
    POPT_ARG_NONE, nullptr,
    OPT_MKLINKS,
    T_("Create executables."),
    nullptr
  },

  {
    "mkmaps", 0,
    POPT_ARG_NONE, nullptr,
    OPT_MKMAPS,
    T_("Create font map files."),
    nullptr
  },

  {
    "modify-path", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MODIFY_PATH,
    T_("Modify the environmaent variable PATH."),
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "no-registry", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NO_REGISTRY,
    T_("Do not use the Windows Registry to store configuration settings."),
    nullptr
  },
#endif

  {
    "portable", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_PORTABLE,
    T_("Create a portable setup."),
    "DIR"
  },

  {
    "print-only", 'n',
    POPT_ARG_NONE, nullptr,
    OPT_PRINT_ONLY,
    T_("Print what would be done."),
    nullptr
  },

  {
    "quiet", 'q',
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Suppress screen output."),
    nullptr
  },

  {
    "recursive", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RECURSIVE,
    T_("Operate recursively."),
    nullptr
  },

  {
    "register-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_REGISTER_ROOT,
    T_("Register a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "register-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REGISTER_SHELL_FILE_TYPES,
    T_("Register shell file types."),
    nullptr
  },
#endif

  {
    "remove-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REMOVE_FILE,
    T_("Remove a file from the file name database."),
    T_("FILE")
  },

  {
    "report", 0,
    POPT_ARG_NONE, nullptr,
    OPT_REPORT,
    T_("Create a configuration report."),
    nullptr
  },

  {
    "rmfndb", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RMFNDB,
    T_("Remove file name database files."),
    nullptr
  },

  {
    "set-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SET_CONFIG_VALUE,
    T_("Set the specified configuration value."),
    T_("[SECTION]VALUENAME=VALUE")
  },

  {
    "show-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SHOW_CONFIG_VALUE,
    T_("Show the specified configuration value."),
    T_("[SECTION]VALUENAME")
  },

  {
    "unregister-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_UNREGISTER_ROOT,
    T_("Unegister a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "unregister-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_UNREGISTER_SHELL_FILE_TYPES,
    T_("Unregister shell file types."),
    nullptr
  },
#endif

  {
    "update-fndb", 'u',
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_UPDATE_FNDB,
    T_("Update the file name database."),
    T_("ROOT")
  },

#if !defined(MIKTEX_STANDALONE)
  {
    "user-install", 0,
    POPT_ARG_STRING, nullptr,
    OPT_USER_INSTALL,
    T_("Register the user installation directory."),
    T_("DIR")
  },
#endif

  {
    "user-roots", 'r',
    POPT_ARG_STRING, nullptr,
    OPT_USER_ROOTS,
    T_("Register user root directories."),
    T_("DIRS")
  },

  {
    "verbose", 'v',
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Print information on what is being done."),
    nullptr
  },

  {
    "version", 'V',
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  {
    "xml", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_XML,
    T_("Print XML."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

const struct poptOption IniTeXMFApp::aoption_setup[] = {

  {
    "add-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ADD_FILE,
    T_("Add a file to the file name database."),
    T_("FILE")
  },

  {
    "admin", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ADMIN,
    T_("Run in administration mode."),
    nullptr
  },

  {
    "common-config", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_CONFIG,
    T_("Register the common configuration directory."),
    T_("DIR")
  },

  {
    "common-data", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_DATA,
    T_("Register the common data directory."),
    T_("DIR")
  },

  {
    "common-install", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_INSTALL,
    T_("Register the common installation directory."),
    T_("DIR")
  },

  {
    "common-roots", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_ROOTS,
    T_("Register common root directories."),
    T_("DIRS")
  },

  {
    "create-config-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CREATE_CONFIG_FILE,
    T_("Create the specified configuration file."),
    T_("CONFIGFILE")
  },

  {
    "csv", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CSV,
    T_("Print comma-separated values."),
    nullptr
  },

  {
    "default-paper-size", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_DEFAULT_PAPER_SIZE,
    T_("Set the default paper size."),
    T_("SIZE"),
  },

  {
    "disable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_DISABLE_INSTALLER,
    T_("Disable the package installer (do not automatically install missing files)."),
    nullptr
  },

  {
    "dump", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_DUMP,
    T_("Create format files."),
    T_("FORMAT")
  },

  {
    "dump-by-name", 0,
    POPT_ARG_STRING, nullptr,
    OPT_DUMP_BY_NAME,
    T_("Create a memory dump file by name."),
    T_("NAME")
  },

  {
    "edit-config-file", 0,
    POPT_ARG_STRING, nullptr,
    OPT_EDIT_CONFIG_FILE,
    T_("Open the specified configuration file in an editor. See the manual, for more information."),
    T_("CONFIGFILE")
  },

  {
    "enable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ENABLE_INSTALLER,
    T_("Enable the package installer (automatically install missing files)."),
    nullptr
  },

  {
    "engine", 0,
    POPT_ARG_STRING, nullptr,
    OPT_ENGINE,
    T_("Engine to be used."),
    T_("ENGINE")
  },

  {
    "force", 0,
    POPT_ARG_NONE, nullptr,
    OPT_FORCE,
    T_("Force --mklinks to overwrite existing files."),
    nullptr
  },

  {
    "list-directory", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LIST_DIRECTORY,
    T_("List the FNDB contents of a directory."),
    T_("DIR")
  },

  {
    "list-formats", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_FORMATS,
    T_("List formats."),
    nullptr
  },

  {
    "list-modes", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_MODES,
    T_("List METAFONT modes"),
    nullptr
  },

  {
    "log-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LOG_FILE,
    T_("Append to log file."),
    T_("FILE")
  },

  {
    "mklangs", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MKLANGS,
    T_("Create language.dat, language.dat.lua and language.def."),
    nullptr
  },

  {
    "mklinks", 0,
    POPT_ARG_NONE, nullptr,
    OPT_MKLINKS,
    T_("Create executables."),
    nullptr
  },

  {
    "mkmaps", 0,
    POPT_ARG_NONE, nullptr,
    OPT_MKMAPS,
    T_("Create font map files."),
    nullptr
  },

  {
    "modify-path", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MODIFY_PATH,
    T_("Modify the environmaent variable PATH."),
    nullptr
  },

#if defined(MIKTEX_WINDOWS)
  {
    "no-registry", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_NO_REGISTRY,
    T_("Do not use the Windows Registry to store configuration settings."),
    nullptr
  },
#endif

  {
    "portable", 0,
    POPT_ARG_STRING, nullptr,
    OPT_PORTABLE,
    T_("Create a portable setup."),
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
    "quiet", 'q',
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Suppress screen output."),
    nullptr
  },

  {
    "recursive", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RECURSIVE,
    T_("Operate recursively."),
    nullptr
  },

  {
    "register-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_REGISTER_ROOT,
    T_("Register a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "register-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REGISTER_SHELL_FILE_TYPES,
    T_("Register shell file types."),
    nullptr
  },
#endif

  {
    "remove-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REMOVE_FILE,
    T_("Remove a file from the file name database."),
    T_("FILE")
  },

  {
    "report", 0,
    POPT_ARG_NONE, nullptr,
    OPT_REPORT,
    T_("Create a configuration report."),
    nullptr
  },

  {
    "rmfndb", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RMFNDB,
    T_("Remove file name database files."),
    nullptr
  },

  {
    "set-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SET_CONFIG_VALUE,
    T_("\
Set the specified configuration value."),
    T_("[SECTION]VALUENAME=VALUE")
  },

  {
    "show-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SHOW_CONFIG_VALUE,
    T_("\
Show the specified configuration value."),
    T_("[SECTION]VALUENAME")
  },

  {
    "unregister-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_UNREGISTER_ROOT,
    T_("Unegister a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "unregister-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_UNREGISTER_SHELL_FILE_TYPES,
    T_("Unregister shell file types."),
    nullptr
  },
#endif

    {
    "update-fndb", 'u',
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_UPDATE_FNDB,
    T_("Update the file name database."),
    T_("ROOT")
  },

  {
    "user-config", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_CONFIG,
    T_("Register the user configuration directory."),
    T_("DIR")
  },

  {
    "user-data", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_DATA,
    T_("Register the user data directory."),
    T_("DIR")
  },

  {
    "user-install", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_INSTALL,
    T_("Register the user installation directory."),
    T_("DIR")
  },

  {
    "user-roots", 'r',
    POPT_ARG_STRING, nullptr,
    OPT_USER_ROOTS,
    T_("Register user root directories."),
    T_("DIRS")
  },

  {
    "verbose", 'v',
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Print information on what is being done."),
    nullptr
  },

  {
    "version", 'V',
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  {
    "xml", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_XML,
    T_("Print XML."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

const struct poptOption IniTeXMFApp::aoption_update[] = {

  {
    "add-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_ADD_FILE,
    T_("Add a file to the file name database."),
    T_("FILE")
  },

  {
    "admin", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ADMIN,
    T_("Run in administration mode."),
    0
  },

  {
    "common-config", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_CONFIG,
    T_("Register the common configuration directory."),
    T_("DIR")
  },

  {
    "common-data", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_DATA,
    T_("Register the common data directory."),
    T_("DIR")
  },

  {
    "common-install", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_INSTALL,
    T_("Register the common installation directory."),
    T_("DIR")
  },

  {
    "common-roots", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_COMMON_ROOTS,
    T_("Register common root directories."),
    T_("DIRS")
  },

  {
    "create-config-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CREATE_CONFIG_FILE,
    T_("Create the specified configuration file."),
    T_("CONFIGFILE")
  },

  {
    "csv", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_CSV,
    T_("Print comma-separated values."),
    nullptr 
  },

  {
    "default-paper-size", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_DEFAULT_PAPER_SIZE,
    T_("Set the default paper size."),
    T_("SIZE"),
  },

  {
    "disable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_DISABLE_INSTALLER,
    T_("Disable the package installer (do not automatically install missing files)."),
    nullptr
  },

  {
    "dump", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_DUMP,
    T_("Create format files."),
    T_("FORMAT")
  },

  {
    "dump-by-name", 0,
    POPT_ARG_STRING, nullptr,
    OPT_DUMP_BY_NAME,
    T_("Create a memory dump file by name."),
    T_("NAME")
  },

  {
    "edit-config-file", 0,
    POPT_ARG_STRING, nullptr,
    OPT_EDIT_CONFIG_FILE,
    T_("Open the specified configuration file in an editor. See the manual, for more information."),
    T_("CONFIGFILE")
  },

  {
    "enable-installer", 0,
    POPT_ARG_NONE, nullptr,
    OPT_ENABLE_INSTALLER,
    T_("Enable the package installer (automatically install missing files)."),
    nullptr
  },

  {
    "engine", 0,
    POPT_ARG_STRING, nullptr,
    OPT_ENGINE,
    T_("Engine to be used."),
    T_("ENGINE")
  },

  {
    "force", 0,
    POPT_ARG_NONE, nullptr,
    OPT_FORCE,
    T_("Force --mklinks to overwrite existing files."),
    nullptr
  },

  {
    "list-directory", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LIST_DIRECTORY,
    T_("List the FNDB contents of a directory."),
    T_("DIR")
  },

  {
    "list-formats", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_FORMATS,
    T_("List formats."),
    nullptr
  },

  {
    "list-modes", 0,
    POPT_ARG_NONE, nullptr,
    OPT_LIST_MODES,
    T_("List METAFONT modes"),
    nullptr
  },

  {
    "log-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_LOG_FILE,
    T_("Append to log file."),
    T_("FILE")
  },

  {
    "mklangs", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MKLANGS,
    T_("Create language.dat, language.dat.lua and language.def."),
    nullptr
  },

  {
    "mklinks", 0,
    POPT_ARG_NONE, nullptr,
    OPT_MKLINKS,
    T_("Create executables."),
    nullptr
  },

  {
    "mkmaps", 0,
    POPT_ARG_NONE, nullptr,
    OPT_MKMAPS,
    T_("Create font map files."),
    nullptr
  },

  {
    "modify-path", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_MODIFY_PATH,
    T_("Modify the environmaent variable PATH."),
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
    "quiet", 'q',
    POPT_ARG_NONE, nullptr,
    OPT_QUIET,
    T_("Suppress screen output."),
    nullptr
  },

  {
    "recursive", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RECURSIVE,
    T_("Operate recursively."),
    nullptr
  },

  {
    "register-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_REGISTER_ROOT,
    T_("Register a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "register-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REGISTER_SHELL_FILE_TYPES,
    T_("Register shell file types."),
    nullptr
  },
#endif

  {
    "remove-file", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_REMOVE_FILE,
    T_("Remove a file from the file name database."),
    T_("FILE")
  },

  {
    "report", 0,
    POPT_ARG_NONE, nullptr,
    OPT_REPORT,
    T_("Create a configuration report."),
    nullptr
  },

  {
    "rmfndb", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_RMFNDB,
    T_("Remove file name database files."),
    nullptr
  },

  {
    "set-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SET_CONFIG_VALUE,
    T_("Set the specified configuration value."),
    T_("[SECTION]VALUENAME=VALUE")
  },

  {
    "show-config-value", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_SHOW_CONFIG_VALUE,
    T_("Show the specified configuration value."),
    T_("[SECTION]VALUENAME")
  },

  {
    "unregister-root", 0,
    POPT_ARG_STRING, nullptr,
    OPT_UNREGISTER_ROOT,
    T_("Unegister a TEXMF root directory."),
    T_("DIR"),
  },

#if defined(MIKTEX_WINDOWS)
  {
    "unregister-shell-file-types", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_UNREGISTER_SHELL_FILE_TYPES,
    T_("Unregister shell file types."),
    nullptr
  },
#endif

  {
    "update-fndb", 'u',
    POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, nullptr,
    OPT_UPDATE_FNDB,
    T_("Update the file name database."),
    T_("ROOT")
  },

  {
    "user-config", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_CONFIG,
    T_("Register the user configuration directory."),
    T_("DIR")
  },

  {
    "user-data", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_DATA,
    T_("Register the user data directory."),
    T_("DIR")
  },

  {
    "user-install", 0,
    POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_USER_INSTALL,
    T_("Register the user installation directory."),
    T_("DIR")
  },

  {
    "user-roots", 'r',
    POPT_ARG_STRING, nullptr,
    OPT_USER_ROOTS,
    T_("Register user root directories."),
    T_("DIRS")
  },

  {
    "verbose", 'v',
    POPT_ARG_NONE, nullptr,
    OPT_VERBOSE,
    T_("Print information on what is being done."),
    nullptr
  },

  {
    "version", 'V',
    POPT_ARG_NONE, nullptr,
    OPT_VERSION,
    T_("Print version information and exit."),
    nullptr
  },

  {
    "xml", 0,
    POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, nullptr,
    OPT_XML,
    T_("Print XML."),
    nullptr
  },

  POPT_AUTOHELP
  POPT_TABLEEND
};

const struct poptOption IniTeXMFApp::aoption_mktexlsr[] = {
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

const struct poptOption IniTeXMFApp::aoption_texlinks[] = {
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
    Finalize();
  }
  catch (const exception &)
  {
  }
}

void IniTeXMFApp::Init(const char * argv0)
{
  Session::InitInfo initInfo(argv0);
#if defined(MIKTEX_WINDOWS)
  initInfo.SetOptions({ Session::InitOption::InitializeCOM });
#endif
  initInfo.SetTraceCallback(this);
  session = Session::Create(initInfo);
  enableInstaller = session->GetConfigValue(MIKTEX_REGKEY_PACKAGE_MANAGER, MIKTEX_REGVAL_AUTO_INSTALL, TriState(TriState::Undetermined));
  PathName xmlFileName;
  if (session->FindFile("initexmf." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
    || session->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
  {
    Utils::SetEnvironmentString("MIKTEX_LOG_DIR", GetLogDir().Get());
    Utils::SetEnvironmentString("MIKTEX_LOG_NAME", "initexmf");
    log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
  }
  else
  {
    log4cxx::BasicConfigurator::configure();
  }
  isLog4cxxConfigured = true;
  LOG4CXX_INFO(logger, "starting: " << Utils::MakeProgramVersionString(TheNameOfTheGame, MIKTEX_COMPONENT_VERSION_STR));
  packageManager = PackageManager::Create(PackageManager::InitInfo(this));
  FindWizards();
  PathName myName = PathName(argv0).GetFileNameWithoutExtension();
  isMktexlsrMode = myName == "mktexlsr" || myName == "texhash";
  isTexlinksMode = myName == "texlinks";
  session->SetFindFileCallback(this);
}

void IniTeXMFApp::Finalize()
{
  if (logStream.IsOpen())
  {
    logStream.Close();
  }
  FlushPendingTraceMessages();
  packageInstaller = nullptr;
  packageManager = nullptr;
  session = nullptr;
}

void IniTeXMFApp::FindWizards()
{
  setupWizardRunning = false;
  updateWizardRunning = false;
  vector<string> invokerNames = Process2::GetInvokerNames();
  for (const string & name : invokerNames)
  {
    if (PathName::Compare(name, MIKTEX_PREFIX "update") == 0
      || PathName::Compare(name, MIKTEX_PREFIX "update" MIKTEX_ADMIN_SUFFIX) == 0)
    {
      updateWizardRunning = true;
    }
    else if (
      name.find("basic-miktex") != string::npos ||
      name.find("BASIC-MIKTEX") != string::npos ||
      name.find("setup") != string::npos ||
      name.find("SETUP") != string::npos ||
      name.find("install") != string::npos ||
      name.find("INSTALL") != string::npos)
    {
      setupWizardRunning = true;
    }
  }
}

void IniTeXMFApp::Verbose(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  string s;
  try
  {
    s = StringUtil::FormatString(lpszFormat, arglist);
  }
  catch (...)
  {
    va_end(arglist);
    throw;
  }
  va_end(arglist);
  if (!printOnly)
  {
    LOG4CXX_INFO(logger, s);
  }
  if (verbose && !printOnly)
  {
    cout << s << endl;
  }
}

void IniTeXMFApp::PrintOnly(const char * lpszFormat, ...)
{
  if (!printOnly)
  {
    return;
  }
  va_list arglist;
  va_start(arglist, lpszFormat);
  string s;
  try
  {
    s = StringUtil::FormatString(lpszFormat, arglist);
  }
  catch (...)
  {
    va_end(arglist);
    throw;
  }
  va_end(arglist);
  cout << s << endl;
}

void IniTeXMFApp::Warning(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  string s;
  try
  {
    s = StringUtil::FormatString(lpszFormat, arglist);
  }
  catch (...)
  {
    va_end(arglist);
    throw;
  }
  va_end(arglist);
  LOG4CXX_WARN(logger, s);
  if (!quiet)
  {
    cerr << PROGNAME << ": " << T_("warning") << ": " << s << endl;
  }
}

static void Sorry(string reason)
{
  if (cerr.fail())
  {
    return;
  }
  cerr << endl;
  if (reason.empty())
  {
    cerr << StringUtil::FormatString(T_("Sorry, but %s did not succeed."), Q_(TheNameOfTheGame)) << endl;
  }
  else
  {
    cerr
      << StringUtil::FormatString(T_("Sorry, but %s did not succeed for the following reason:"), Q_(TheNameOfTheGame)) << endl << endl
      << "  " << reason << endl;
  }
  log4cxx::RollingFileAppenderPtr appender = log4cxx::Logger::getRootLogger()->getAppender(LOG4CXX_STR("RollingLogFile"));
  if (appender != nullptr)
  {
    cerr
      << endl
      << T_("The log file hopefully contains the information to get MiKTeX going again:") << endl
      << endl
      << "  " << PathName(appender->getFile()).ToUnix() << endl;
  }
  cerr
    << endl
    << T_("You may want to visit the MiKTeX project page, if you need help.") << endl;
}

static void Sorry()
{
  Sorry("");
}

MIKTEXNORETURN void IniTeXMFApp::FatalError(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  string s = StringUtil::FormatString(lpszFormat, arglist);
  va_end(arglist);
  LOG4CXX_FATAL(logger, s);
  Sorry(s);
  throw (1);
}

bool IniTeXMFApp::InstallPackage(const string & deploymentName, const PathName & trigger, PathName & installRoot)
{
  if (enableInstaller != TriState::True)
  {
    return false;
  }
  LOG4CXX_INFO(logger, "installing package " << deploymentName << " triggered by " << trigger.ToString());
  Verbose(T_("Installing package %s..."), deploymentName.c_str());
  if (packageInstaller == nullptr)
  {
    packageInstaller = packageManager->CreateInstaller();
    packageInstaller->SetCallback(this);
    packageInstaller->SetNoPostProcessing(true);
  }
  packageInstaller->SetFileLists({ deploymentName }, {});
  packageInstaller->InstallRemove();
  installRoot = session->GetSpecialPath(SpecialPath::InstallRoot);
  return true;
}

bool IniTeXMFApp::TryCreateFile(const MiKTeX::Core::PathName & fileName, MiKTeX::Core::FileType fileType)
{
  return false;
}

bool IniTeXMFApp::ReadDirectory(const char * lpszPath, char * * ppSubDirNames, char * * ppFileNames, char * * ppFileNameInfos)
{
  UNUSED_ALWAYS(lpszPath);
  UNUSED_ALWAYS(ppSubDirNames);
  UNUSED_ALWAYS(ppFileNames);
  UNUSED_ALWAYS(ppFileNameInfos);
  return false;
}

bool IniTeXMFApp::OnProgress(unsigned level, const char * lpszDirectory)
{
#if 0
  if (verbose && level == 1)
  {
    Verbose(T_("Scanning %s"), Q_(lpszDirectory));
  }
  else if (level == 1)
  {
    Message(".");
  }
#endif
  return true;
}

void IniTeXMFApp::UpdateFilenameDatabase(const PathName & root)
{
  // unload the file name database
  if (!printOnly && !session->UnloadFilenameDatabase())
  {
    FatalError(T_("The file name database could not be unloaded."));
  }

  unsigned rootIdx = session->DeriveTEXMFRoot(root);

  // remove the old FNDB file
  PathName path = session->GetFilenameDatabasePathName(rootIdx);
  if (File::Exists(path))
  {
    PrintOnly("rm %s", Q_(path));
    if (!printOnly)
    {
      File::Delete(path, { FileDeleteOption::TryHard });
    }
  }

  // create the FNDB file
  PathName fndbPath = session->GetFilenameDatabasePathName(rootIdx);
  if (session->IsCommonRootDirectory(rootIdx))
  {
    Verbose(T_("Creating fndb for common root directory (%s)..."), Q_(root));
  }
  else
  {
    Verbose(T_("Creating fndb for user root directory (%s)..."), Q_(root));
  }
  PrintOnly("fndbcreate %s %s", Q_(fndbPath), Q_(root));
  if (!printOnly)
  {
    Fndb::Create(fndbPath, root, this);
  }
}

void IniTeXMFApp::UpdateFilenameDatabase(unsigned root)
{
  UpdateFilenameDatabase(session->GetRootDirectory(root));
}

void IniTeXMFApp::ListFormats()
{
  for (const FormatInfo & formatInfo : session->GetFormats())
  {
    cout << formatInfo.key << " (" << formatInfo.description << ")" << endl;
  }
}

void IniTeXMFApp::ListMetafontModes()
{
  MIKTEXMFMODE mode;
  for (unsigned i = 0; session->GetMETAFONTMode(i, &mode); ++i)
  {
    cout << StringUtil::FormatString("%-8s  %5dx%-5d  %s", mode.szMnemonic, mode.iHorzRes, mode.iVertRes, mode.szDescription) << endl;
  }
}

void IniTeXMFApp::RemoveFndb()
{
  size_t nRoots = session->GetNumberOfTEXMFRoots();
  for (unsigned r = 0; r < nRoots; ++r)
  {
    PathName path = session->GetFilenameDatabasePathName(r);
    PrintOnly("rm %s", Q_(path));
    if (!printOnly && File::Exists(path))
    {
      Verbose(T_("Removing fndb (%s)..."), Q_(session->GetRootDirectory(r)));
      File::Delete(path, { FileDeleteOption::TryHard });
    }
  }
}

void IniTeXMFApp::SetTeXMFRootDirectories(
#if defined(MIKTEX_WINDOWS)
  bool noRegistry
#endif
  )
{
  Verbose(T_("Registering root directories..."));
  PrintOnly("regroots ur=%s ud=%s uc=%s ui=%s cr=%s cd=%s cc=%s ci=%s", Q_(startupConfig.userRoots), Q_(startupConfig.userDataRoot), Q_(startupConfig.userConfigRoot), Q_(startupConfig.userInstallRoot), Q_(startupConfig.commonRoots), Q_(startupConfig.commonDataRoot), Q_(startupConfig.commonConfigRoot), Q_(startupConfig.commonInstallRoot));
  if (!printOnly)
  {
    RegisterRootDirectoriesOptionSet options;
#if defined(MIKTEX_WINDOWS)
    if (noRegistry)
    {
      options += RegisterRootDirectoriesOption::NoRegistry;
    }
#endif
    session->RegisterRootDirectories(startupConfig, options);
  }
}

void IniTeXMFApp::RunMakeTeX(const string & makeProg, const CommandLineBuilder & arguments)
{
  PathName exe;

  if (!session->FindFile(makeProg.c_str(), FileType::EXE, exe))
  {
    FatalError(T_("The %s executable could not be found."), Q_(makeProg));
  }

  CommandLineBuilder xArguments(arguments);

  if (printOnly)
  {
    xArguments.AppendOption("--print-only");
  }

  if (verbose)
  {
    xArguments.AppendOption("--verbose");
  }

  if (quiet)
  {
    xArguments.AppendOption("--quiet");
  }

  if (adminMode)
  {
    xArguments.AppendOption("--admin");
  }

  switch (enableInstaller)
  {
  case TriState::True:
    xArguments.AppendOption("--enable-installer");
    break;
  case TriState::False:
    xArguments.AppendOption("--disable-installer");
    break;
  default:
    break;
  }

  LOG4CXX_INFO(logger, "running '" << makeProg << " " << xArguments.ToString() << "'");
  Process::Run(exe, xArguments.ToString());
}

void IniTeXMFApp::MakeFormatFile(const string & formatKey)
{
  if (find(formatsMade.begin(), formatsMade.end(), formatKey) != formatsMade.end())
  {
    return;
  }

  FormatInfo formatInfo;
  if (!session->TryGetFormatInfo(formatKey.c_str(), formatInfo))
  {
    FatalError(T_("Unknown format: %s"), Q_(formatKey));
  }

  string maker;

  CommandLineBuilder arguments;

  if (formatInfo.compiler == "mf")
  {
    maker = MIKTEX_MAKEBASE_EXE;
  }
  else
  {
    maker = MIKTEX_MAKEFMT_EXE;
    arguments.AppendOption("--engine=", formatInfo.compiler);
  }

  arguments.AppendOption("--dest-name=", formatInfo.name);

  if (!formatInfo.preloaded.empty())
  {
    if (PathName::Compare(formatInfo.preloaded, formatKey) == 0)
    {
      LOG4CXX_FATAL(logger, T_("Rule recursion detected for: ") << formatKey);
      FatalError(T_("Format '%s' cannot be built."), formatKey.c_str());
    }
    // RECURSION
    MakeFormatFile(formatInfo.preloaded);
    arguments.AppendOption("--preload=", formatInfo.preloaded);
  }

  if (PathName(formatInfo.inputFile).HasExtension(".ini"))
  {
    arguments.AppendOption("--no-dump");
  }

  arguments.AppendArgument(formatInfo.inputFile);

  if (!formatInfo.arguments.empty())
  {
    arguments.AppendOption("--engine-option=", formatInfo.arguments);
  }

  RunMakeTeX(maker.c_str(), arguments);

  formatsMade.push_back(formatKey);
}

void IniTeXMFApp::MakeFormatFiles(const vector<string> & formats)
{
  if (formats.size() == 0)
  {
    for (const FormatInfo & formatInfo : session->GetFormats())
    {
      if (!formatInfo.exclude)
      {
        MakeFormatFile(formatInfo.key);
      }
    }
  }
  else
  {
    for (const string & fmt : formats)
    {
      MakeFormatFile(fmt);
    }
  }
}

void IniTeXMFApp::MakeFormatFilesByName(const vector<string> & formatsByName, const string & engine)
{
  for (const string & name : formatsByName)
  {
    bool done = false;
    for (const FormatInfo & formatInfo : session->GetFormats())
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
        FatalError(T_("Unknown format name: %s"), Q_(name));
      }
      else
      {
        FatalError(T_("Unknown format name/engine: %s/%s"), Q_(name), engine.c_str());
      }
    }
  }
}

void IniTeXMFApp::CreateLink(const FileLink & fileLink, bool supportsHardLinks, bool overwrite)
{
  LinkType linkType = fileLink.linkType;
  if (linkType == LinkType::Hard && !supportsHardLinks)
  {
    linkType = LinkType::Copy;
  }
  for (const string & linkName : fileLink.linkNames)
  {
    if (File::Exists(linkName))
    {
      if (!overwrite || (linkType == LinkType::Copy && File::Equals(fileLink.target, linkName)))
      {
        continue;
      }
      File::Delete(linkName, { FileDeleteOption::TryHard, FileDeleteOption::UpdateFndb });
    }
    switch (linkType)
    {
    case LinkType::Hard:
      PrintOnly("ln %s %s", Q_(fileLink.target), Q_(linkName));
      if (!printOnly)
      {
        File::CreateLink(fileLink.target, linkName, { CreateLinkOption::UpdateFndb });
      }
      break;
    case LinkType::Copy:
      PrintOnly("cp %s %s", Q_(fileLink.target), Q_(linkName));
      if (!printOnly)
      {
        File::Copy(fileLink.target, linkName, { FileCopyOption::UpdateFndb });
      }
      break;
    default:
      MIKTEX_UNEXPECTED();
    }
    if (logStream.IsOpen())
    {
      logStream.WriteLine(linkName);
    }
  }
}

void IniTeXMFApp::RegisterRoots(const vector<PathName> & roots, bool reg)
{
  string newRoots;

  PathName userInstallRoot;
  PathName userConfigRoot;
  PathName userDataRoot;

  if (!session->IsAdminMode())
  {
    userInstallRoot = session->GetSpecialPath(SpecialPath::UserInstallRoot);
    userConfigRoot = session->GetSpecialPath(SpecialPath::UserConfigRoot);
    userDataRoot = session->GetSpecialPath(SpecialPath::UserDataRoot);
  }

  PathName commonInstallRoot = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
  PathName commonConfigRoot = session->GetSpecialPath(SpecialPath::CommonConfigRoot);
  PathName commonDataRoot = session->GetSpecialPath(SpecialPath::CommonDataRoot);

  for (unsigned r = 0; r < session->GetNumberOfTEXMFRoots(); ++r)
  {
    PathName root = session->GetRootDirectory(r);
    int rootOrdinal = session->DeriveTEXMFRoot(root);
    if (session->IsAdminMode() && !session->IsCommonRootDirectory(rootOrdinal))
    {
      continue;
    }
    if (!session->IsAdminMode()
      && (session->IsCommonRootDirectory(rootOrdinal)
        || root == userInstallRoot
        || root == userConfigRoot
        || root == userDataRoot))
    {
      continue;
    }
    if (root == commonInstallRoot
      || root == commonConfigRoot
      || root == commonDataRoot)
    {
      continue;
    }
    if (!reg)
    {
      bool toBeUnregistered = false;
      for (vector<PathName>::const_iterator it = roots.begin(); it != roots.end() && !toBeUnregistered; ++it)
      {
        if (*it == root)
        {
          toBeUnregistered = true;
        }
      }
      if (toBeUnregistered)
      {
        continue;
      }
    }
    if (!newRoots.empty())
    {
      newRoots += PathName::PathNameDelimiter;
    }
    newRoots += root.Get();
  }

  if (reg)
  {
    for (const PathName r : roots)
    {
      if (!newRoots.empty())
      {
        newRoots += PathName::PathNameDelimiter;
      }
      newRoots += r.ToString();
    }
  }

  session->RegisterRootDirectories(newRoots);

  if (reg)
  {
    for (const PathName & r : roots)
    {
      UpdateFilenameDatabase(r);
    }
  }
}

#if defined(MIKTEX_WINDOWS)

struct ShellFileType {
  const char * lpszComponent;
  const char * lpszExtension;
  const char * lpszUserFriendlyName;
  const char * lpszExecutable;
  int iconIndex;
  bool takeOwnership;
  const char * lpszVerb;
  const char * lpszCommandArgs;
  const char * lpszDdeArgs;
} const shellFileTypes[] = {
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
  for (const ShellFileType & sft : shellFileTypes)
  {
    string progId = Utils::MakeProgId(sft.lpszComponent);
    if (reg)
    {
      PathName exe;
      if (sft.lpszExecutable != nullptr && !session->FindFile(sft.lpszExecutable, FileType::EXE, exe))
      {
        FatalError(T_("Could not find %s."), sft.lpszExecutable);
      }
      string command;
      if (sft.lpszExecutable != nullptr && sft.lpszCommandArgs != nullptr)
      {
        command = '\"';
        command += exe.Get();
        command += "\" ";
        command += sft.lpszCommandArgs;
      }
      string iconPath;
      if (sft.lpszExecutable != 0 && sft.iconIndex != INT_MAX)
      {
        iconPath += exe.Get();
        iconPath += ",";
        iconPath += std::to_string(sft.iconIndex);
      }
      if (sft.lpszUserFriendlyName != 0 || !iconPath.empty())
      {
        Utils::RegisterShellFileType(progId.c_str(), sft.lpszUserFriendlyName, (iconPath.empty() ? nullptr : iconPath.c_str()));
      }
      if (sft.lpszVerb != nullptr && (!command.empty() || sft.lpszDdeArgs != nullptr))
      {
        Utils::RegisterShellVerb(progId.c_str(), sft.lpszVerb, (command.empty() ? nullptr : command.c_str()), sft.lpszDdeArgs);
      }
      if (sft.lpszExtension != nullptr)
      {
        Utils::RegisterShellFileAssoc(sft.lpszExtension, progId.c_str(), sft.takeOwnership);
      }
    }
    else
    {
      Utils::UnregisterShellFileType(progId.c_str());
      if (sft.lpszExtension != nullptr)
      {
        Utils::UnregisterShellFileAssoc(sft.lpszExtension, progId.c_str());
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

vector<FileLink> explicitFileLinks =
{
  { "arctrl" MIKTEX_EXE_FILE_SUFFIX, { "pdfclose", "pdfdde", "pdfopen" } },
  { "mkfntmap" MIKTEX_EXE_FILE_SUFFIX, { "updmap" } },
  { "mthelp" MIKTEX_EXE_FILE_SUFFIX, { "texdoc" } },
  { MIKTEX_BIBTEX_EXE, { "bibtex" } },
  { MIKTEX_DVICOPY_EXE, { "dvicopy" } },
  { MIKTEX_DVIPDFMX_EXE, { "dvipdfm", "dvipdfmx", "ebb", "extractbb", "xbb" } },
  { MIKTEX_DVITYPE_EXE, { "dvitype" } },
  { MIKTEX_GFTODVI_EXE, { "gftodvi" } },
  { MIKTEX_GFTOPK_EXE, { "gftopk" } },
  { MIKTEX_GFTYPE_EXE, { "gftype" } },
  { MIKTEX_LUATEX_EXE, { MIKTEX_PREFIX "texlua", MIKTEX_PREFIX "texluac", "luatex", "texlua", "texluac", MIKTEX_LUALATEX_EXE } },
  { MIKTEX_MAKEBASE_EXE, { "makebase" } },
  { MIKTEX_MAKEFMT_EXE, { "makefmt" } },
  { MIKTEX_MAKEPK_EXE, { "makepk" } },
  { MIKTEX_MAKETFM_EXE, { "maketfm" } },
  { MIKTEX_MFT_EXE, { "mft" } },
  { MIKTEX_MF_EXE, { "mf", "inimf", "virmf" } },
  { MIKTEX_MPOST_EXE, { "dvitomp", "mpost" } },
  { MIKTEX_ODVICOPY_EXE, { "odvicopy" } },
  { MIKTEX_OFM2OPL_EXE, { "ofm2opl" } },
  { MIKTEX_OMEGA_EXE, { "omega" } },
  { MIKTEX_OPL2OFM_EXE, { "opl2ofm" } },
  { MIKTEX_OVF2OVP_EXE, { "ovf2ovp" } },
  { MIKTEX_OVP2OVF_EXE, { "ovp2ovf" } },
  { MIKTEX_PDFTEX_EXE, { "pdftex", MIKTEX_PDFLATEX_EXE } },
  { MIKTEX_PLTOTF_EXE, { "pltotf" } },
  { MIKTEX_POOLTYPE_EXE, { "pooltype" } },
  { MIKTEX_TANGLE_EXE, { "tangle" } },
  { MIKTEX_TEXWORKS_EXE, { "texworks" } },
  { MIKTEX_TEX_EXE, { "tex", "initex", "virtex", MIKTEX_LATEX_EXE } },
  { MIKTEX_TFTOPL_EXE, { "tftopl" } },
  { MIKTEX_TIE_EXE, { "tie" } },
  { MIKTEX_VFTOVP_EXE, { "vftovp" } },
  { MIKTEX_VPTOVF_EXE, { "vptovf" } },
  { MIKTEX_WEAVE_EXE, { "weave" } },
  { MIKTEX_XDVIPDFMX_EXE, { "xdvipdfmx" } },
  { MIKTEX_XETEX_EXE, { "xetex", MIKTEX_XELATEX_EXE } },
#if defined(WITH_KPSEWHICH)
  { MIKTEX_KPSEWHICH_EXE, { "kpsewhich" } },
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
};

vector<FileLink> IniTeXMFApp::CollectLinks(bool overwrite)
{
  vector<FileLink> result;
  PathName pathBinDir = session->GetSpecialPath(SpecialPath::BinDirectory);
  PathName internalBinDir = session->GetSpecialPath(SpecialPath::InternalBinDirectory);
  for (const FileLink & fileLink : explicitFileLinks)
  {
    PathName targetPath = pathBinDir;
    targetPath /= fileLink.target;
    const char * lpszExtension = targetPath.GetExtension();
    if (File::Exists(targetPath))
    {
      vector<string> linkNames;
      for (const string & linkName : fileLink.linkNames)
      {
	PathName linkPath = pathBinDir;
	linkPath /= linkName;
	if (lpszExtension != nullptr)
	{
	  linkPath.SetExtension(lpszExtension);
	}
        linkNames.push_back(linkPath.ToString());
      }
      result.push_back(FileLink(targetPath.ToString(), linkNames, fileLink.linkType));
    }
    else
    {
      Warning(T_("The link target %s does not exist."), Q_(targetPath));
    }
  }

  for (const FormatInfo & formatInfo : session->GetFormats())
  {
    if (formatInfo.noExecutable)
    {
      continue;
    }
    PathName compilerPath;
    if (!session->FindFile((string(MIKTEX_PREFIX) + formatInfo.compiler).c_str(), FileType::EXE, compilerPath))
    {
      Warning(T_("The %s executable could not be found."), formatInfo.compiler.c_str());
      continue;
    }
    PathName tmp;
    if (overwrite || !session->FindFile(formatInfo.name.c_str(), FileType::EXE, tmp))
    {
      PathName exePath(pathBinDir, formatInfo.name, MIKTEX_EXE_FILE_SUFFIX);
      if (!(compilerPath == exePath))
      {
        result.push_back(FileLink(compilerPath.ToString(), { exePath.ToString() }));
      }
    }
  }

  PathName scriptsIni;
  if (!session->FindFile(MIKTEX_PATH_SCRIPTS_INI, MIKTEX_PATH_TEXMF_PLACEHOLDER, scriptsIni))
  {
    FatalError(T_("Script configuration file not found."));
  }
  unique_ptr<Cfg> config(Cfg::Create());
  config->Read(scriptsIni, true);
  for (const shared_ptr<Cfg::Key> & key : config->GetKeys())
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
    for (const shared_ptr<Cfg::Value> & v : key->GetValues())
    {
      PathName pathExe(pathBinDir, v->GetName(), MIKTEX_EXE_FILE_SUFFIX);
      result.push_back(FileLink(wrapper.ToString(), { pathExe.ToString() }));
    }
  }

#if defined(MIKTEX_WINDOWS)
  static PathName const copystarters[] = {
    MIKTEX_PATH_INTERNAL_TASKBAR_ICON_EXE,
    MIKTEX_PATH_INTERNAL_UPDATE_EXE,
  };
  PathName copystart;
  if (session->FindFile(MIKTEX_PATH_INTERNAL_COPYSTART_EXE, MIKTEX_PATH_TEXMF_PLACEHOLDER, copystart))
  {
    for (const PathName & starter : copystarters)
    {
      PathName pathExe(pathBinDir);
      char szFileName[BufferSizes::MaxPath];
      pathExe /= starter.GetFileName(szFileName);
      result.push_back(FileLink(copystart.ToString(), { pathExe.ToString() }));
    }
  }

  static PathName const copystarters_admin[] = {
    MIKTEX_PATH_INTERNAL_UPDATE_ADMIN_EXE,
  };
  PathName copystart_admin;
  if (session->FindFile(MIKTEX_PATH_INTERNAL_COPYSTART_ADMIN_EXE, MIKTEX_PATH_TEXMF_PLACEHOLDER, copystart_admin))
  {
    for (const PathName & starter : copystarters_admin)
    {
      PathName pathExe(pathBinDir);
      char szFileName[BufferSizes::MaxPath];
      pathExe /= starter.GetFileName(szFileName);
      result.push_back(FileLink(copystart_admin.ToString(), { pathExe.ToString() }));
    }
  }
#endif

  return result;
}

void IniTeXMFApp::MakeLinks(bool force)
{
  PathName pathBinDir = session->GetSpecialPath(SpecialPath::BinDirectory);
  PathName internalBinDir = session->GetSpecialPath(SpecialPath::InternalBinDirectory);

  MIKTEX_ASSERT(pathBinDir.GetMountPoint() == internalBinDir.GetMountPoint());

  bool supportsHardLinks = Utils::SupportsHardLinks(pathBinDir);

  if (!Directory::Exists(pathBinDir))
  {
    Directory::Create(pathBinDir);
  }

  if (logStream.IsOpen())
  {
    logStream.WriteLine("[files]");
  }

  for (const FileLink & fileLink : CollectLinks(force))
  {
    CreateLink(fileLink, supportsHardLinks, force);
  }
}

void IniTeXMFApp::MakeLanguageDat(bool force)
{
  Verbose(T_("Creating language.dat, language.dat.lua and language.def..."));

  if (printOnly)
  {
    return;
  }

  PathName dir;

  PathName languageDatPath = session->GetSpecialPath(SpecialPath::ConfigRoot);
  languageDatPath /= MIKTEX_PATH_LANGUAGE_DAT;
  dir = languageDatPath;
  dir.RemoveFileSpec();
  Directory::Create(dir);
  StreamWriter languageDat(languageDatPath);

  PathName languageDatLuaPath = session->GetSpecialPath(SpecialPath::ConfigRoot);
  languageDatLuaPath /= MIKTEX_PATH_LANGUAGE_DAT_LUA;
  dir = languageDatLuaPath;
  dir.RemoveFileSpec();
  Directory::Create(dir);
  StreamWriter languageDatLua(languageDatLuaPath);

  PathName languageDefPath = session->GetSpecialPath(SpecialPath::ConfigRoot);
  languageDefPath /= MIKTEX_PATH_LANGUAGE_DEF;
  dir = languageDefPath;
  dir.RemoveFileSpec();
  Directory::Create(dir);
  StreamWriter languageDef(languageDefPath);

  languageDatLua.WriteLine("return {");
  languageDef.WriteLine("%% e-TeX V2.2");

  for (const LanguageInfo & languageInfo : session->GetLanguages())
  {
    if (languageInfo.exclude)
    {
      continue;
    }

    PathName loaderPath;
    if (!session->FindFile(languageInfo.loader.c_str(), "%r/tex//", loaderPath))
    {
      continue;
    }

    // language.dat
    languageDat.WriteFormattedLine("%s %s", languageInfo.key.c_str(), languageInfo.loader.c_str());
    for (CSVList synonym(languageInfo.synonyms, ','); synonym.GetCurrent() != nullptr; ++synonym)
    {
      languageDat.WriteFormattedLine("=%s", synonym.GetCurrent());
    }

    // language.def
    languageDef.WriteFormattedLine("\\addlanguage{%s}{%s}{}{%d}{%d}", languageInfo.key.c_str(), languageInfo.loader.c_str(), languageInfo.lefthyphenmin, languageInfo.righthyphenmin);

    // language.dat.lua
    languageDatLua.WriteFormattedLine("\t['%s'] = {", languageInfo.key.c_str());
    languageDatLua.WriteFormattedLine("\t\tloader='%s',", languageInfo.loader.c_str());
    languageDatLua.WriteFormattedLine("\t\tlefthyphenmin=%d,", languageInfo.lefthyphenmin);
    languageDatLua.WriteFormattedLine("\t\trighthyphenmin=%d,", languageInfo.righthyphenmin);
    languageDatLua.Write("\t\tsynonyms={ ");
    int nSyn = 0;
    for (CSVList synonym(languageInfo.synonyms, ','); synonym.GetCurrent() != 0; ++synonym)
    {
      languageDatLua.WriteFormatted("%s'%s'", nSyn > 0 ? "," : "", synonym.GetCurrent());
      nSyn++;
    }
    languageDatLua.WriteLine(" },");
    languageDatLua.WriteFormattedLine("\t\tpatterns='%s',", languageInfo.patterns.c_str());
    languageDatLua.WriteFormattedLine("\t\thyphenation='%s',", languageInfo.hyphenation.c_str());
    if (!languageInfo.luaspecial.empty())
    {
      languageDatLua.WriteFormattedLine("\t\tspecial='%s',", languageInfo.luaspecial.c_str());
    }
    languageDatLua.WriteLine("\t},");
  }

  languageDatLua.WriteLine("}");

  languageDatLua.Close();
  Fndb::Add(languageDatLuaPath);

  languageDef.Close();
  Fndb::Add(languageDefPath);

  languageDat.Close();
  Fndb::Add(languageDatPath);
}

void IniTeXMFApp::MakeMaps(bool force)
{
  PathName pathMkfontmap;
  if (!session->FindFile("mkfntmap", FileType::EXE, pathMkfontmap))
  {
    FatalError(T_("The mkfntmap executable could not be found."));
  }
  CommandLineBuilder arguments;
  if (verbose)
  {
    arguments.AppendOption("--verbose");
  }
  if (adminMode)
  {
    arguments.AppendOption("--admin");
  }
  if (force)
  {
    arguments.AppendOption("--force");
  }
  switch (enableInstaller)
  {
  case TriState::True:
    arguments.AppendOption("--enable-installer");
    break;
  case TriState::False:
    arguments.AppendOption("--disable-installer");
    break;
  default:
    break;
  }
  if (printOnly)
  {
    PrintOnly("mkfntmap %s", arguments.ToString().c_str());
  }
  else
  {
    LOG4CXX_INFO(logger, "running: mkfntmap " << arguments.ToString());
    Process::Run(pathMkfontmap, arguments.ToString());
  }
}

void IniTeXMFApp::CreateConfigFile(const string & relPath, bool edit)
{
  PathName configFile(session->GetSpecialPath(SpecialPath::ConfigRoot));
  bool haveConfigFile = false;
  for (const auto & shortCut : configShortcuts)
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
    if (fileName == relPath)
    {
      configFile /= MIKTEX_PATH_MIKTEX_CONFIG_DIR;
    }
    configFile /= relPath;
    configFile.SetExtension(".ini", false);
    haveConfigFile = true;
  }
  if (!File::Exists(configFile))
  {
    if (!session->TryCreateFromTemplate(configFile))
    {
      Directory::Create(PathName(configFile).RemoveFileSpec());
      StreamWriter writer(configFile);
      writer.Close();
      Fndb::Add(configFile);
    }
  }
  if (edit)
  {
    CommandLineBuilder commandLine;
    commandLine.AppendArgument(configFile);
    string editor;
    const char * lpszEditor = getenv("EDITOR");
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
    Process::Start(editor, commandLine.ToString());
  }
}

void IniTeXMFApp::SetConfigValue(const string & valueSpec)
{
  const char * lpsz = valueSpec.c_str();
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
      FatalError(T_("The configuration value '%s' could not be set."), Q_(valueSpec));
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
    FatalError(T_("The configuration value '%s' could not be set."), Q_(valueSpec));
  }
  ++lpsz;
  string value = lpsz;
  session->SetConfigValue(haveSection ? section.c_str() : nullptr, valueName.c_str(), value.c_str());
}

void IniTeXMFApp::ShowConfigValue(const string & valueSpec)
{
  const char * lpsz = valueSpec.c_str();
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
      FatalError(T_("Invalid value: %s."), Q_(valueSpec));
    }
    ++lpsz;
  }
  string valueName = lpsz;
  string value;
  if (session->TryGetConfigValue(haveSection ? section.c_str() : 0, valueName.c_str(), value))
  {
    cout << value << endl;
  }
}

void IniTeXMFApp::ReportMiKTeXVersion()
{
  vector<string> invokerNames = Process2::GetInvokerNames();
  if (xml)
  {
    xmlWriter.StartElement("setup");
    xmlWriter.StartElement("version");
    xmlWriter.Text(Utils::GetMiKTeXVersionString());
    xmlWriter.EndElement();
    xmlWriter.StartElement("sharedsetup");
    xmlWriter.AddAttribute("value", (session->IsSharedSetup() ? "true" : "false"));
    xmlWriter.EndElement();
#if defined(MIKTEX_WINDOWS)
    xmlWriter.StartElement("systemadmin");
    xmlWriter.AddAttribute("value", (session->IsUserAnAdministrator() ? "true" : "false"));
    xmlWriter.EndElement();
    xmlWriter.StartElement("poweruser");
    xmlWriter.AddAttribute("value", (session->IsUserAPowerUser() ? "true" : "false"));
    xmlWriter.EndElement();
    xmlWriter.EndElement();
#endif
  }
  else
  {
    cout << "MiKTeX: " << Utils::GetMiKTeXVersionString() << endl;
    cout << T_("Invokers:") << " ";
    bool first = true;
    for (const string & name : invokerNames)
    {
      if (!first)
      {
        cout << "/";
      }
      first = false;
      cout << name;
    }
    cout << endl;
    cout << "SharedSetup: " << (session->IsSharedSetup() ? T_("yes") : T_("no")) << endl;
#if defined(MIKTEX_WINDOWS)
    cout << "SystemAdmin: " << (session->IsUserAnAdministrator() ? T_("yes") : T_("no")) << endl;
    cout << "PowerUser: " << (session->IsUserAPowerUser() ? T_("yes") : T_("no")) << endl;
#endif
  }
}

void IniTeXMFApp::ReportOSVersion()
{
  if (xml)
  {
    xmlWriter.StartElement("os");
    xmlWriter.StartElement("version");
    xmlWriter.Text(Utils::GetOSVersionString());
    xmlWriter.EndElement();
    xmlWriter.EndElement();
  }
  else
  {
    cout << "OS: " << Utils::GetOSVersionString() << endl;
  }
}

void IniTeXMFApp::ReportRoots()
{
  if (xml)
  {
    xmlWriter.StartElement("roots");
    for (unsigned idx = 0; idx < session->GetNumberOfTEXMFRoots(); ++idx)
    {
      xmlWriter.StartElement("path");
      PathName root = session->GetRootDirectory(idx);
      xmlWriter.AddAttribute("index", std::to_string(idx));
      if (root == session->GetSpecialPath(SpecialPath::UserInstallRoot))
      {
        xmlWriter.AddAttribute("userinstall", "true");
      }
      if (root == session->GetSpecialPath(SpecialPath::UserDataRoot))
      {
        xmlWriter.AddAttribute("userdata", "true");
      }
      if (root == session->GetSpecialPath(SpecialPath::UserConfigRoot))
      {
        xmlWriter.AddAttribute("userconfig", "true");
      }
      if (root == session->GetSpecialPath(SpecialPath::CommonInstallRoot))
      {
        xmlWriter.AddAttribute("commoninstall", "true");
      }
      if (root == session->GetSpecialPath(SpecialPath::CommonDataRoot))
      {
        xmlWriter.AddAttribute("commondata", "true");
      }
      if (root == session->GetSpecialPath(SpecialPath::CommonConfigRoot))
      {
        xmlWriter.AddAttribute("commonconfig", "true");
      }
      xmlWriter.Text(root.Get());
      xmlWriter.EndElement();
    }
    xmlWriter.EndElement();
  }
  else
  {
    for (unsigned idx = 0; idx < session->GetNumberOfTEXMFRoots(); ++idx)
    {
      cout << StringUtil::FormatString(T_("Root #%u"), idx) << ": " << session->GetRootDirectory(idx) << endl;
    }
    cout << "UserInstall: " << session->GetSpecialPath(SpecialPath::UserInstallRoot) << endl;
    cout << "UserData: " << session->GetSpecialPath(SpecialPath::UserDataRoot) << endl;
    cout << "UserConfig: " << session->GetSpecialPath(SpecialPath::UserConfigRoot) << endl;
    cout << "CommonInstall: " << session->GetSpecialPath(SpecialPath::CommonInstallRoot) << endl;
    cout << "CommonData: " << session->GetSpecialPath(SpecialPath::CommonDataRoot) << endl;
    cout << "CommonConfig: " << session->GetSpecialPath(SpecialPath::CommonConfigRoot) << endl;
  }
}

void IniTeXMFApp::ReportFndbFiles()
{
  if (xml)
  {
    xmlWriter.StartElement("fndb");
    for (unsigned idx = 0; idx < session->GetNumberOfTEXMFRoots(); ++idx)
    {
      PathName absFileName;
      if (session->FindFilenameDatabase(idx, absFileName))
      {
        xmlWriter.StartElement("path");
        xmlWriter.AddAttribute("index", std::to_string(idx));
        xmlWriter.Text(absFileName.Get());
        xmlWriter.EndElement();
      }
    }
    unsigned r = session->DeriveTEXMFRoot(session->GetMpmRootPath());
    PathName path;
    if (session->FindFilenameDatabase(r, path))
    {
      xmlWriter.StartElement("mpmpath");
      xmlWriter.Text(path.Get());
      xmlWriter.EndElement();
    }
    xmlWriter.EndElement();
  }
  else
  {
    for (unsigned idx = 0; idx < session->GetNumberOfTEXMFRoots(); ++idx)
    {
      PathName absFileName;
      cout << "fndb #" << idx << ": ";
      if (session->FindFilenameDatabase(idx, absFileName))
      {
        cout << absFileName << endl;
      }
      else
      {
        cout << T_("<does not exist>") << endl;
      }
    }
    unsigned r = session->DeriveTEXMFRoot(session->GetMpmRootPath());
    PathName path;
    cout << "fndbmpm: ";
    if (session->FindFilenameDatabase(r, path))
    {
      cout << path << endl;
    }
    else
    {
      cout << T_("<does not exist>") << endl;
    }
  }
}

#if defined(MIKTEX_WINDOWS)
void IniTeXMFApp::ReportEnvironmentVariables()
{
  wchar_t * lpszEnv = reinterpret_cast<wchar_t*>(GetEnvironmentStringsW());
  if (lpszEnv == nullptr)
  {
    return;
  }
  xmlWriter.StartElement("environment");
  for (wchar_t * p = lpszEnv; *p != 0; p += wcslen(p) + 1)
  {
    Tokenizer tok(StringUtil::WideCharToUTF8(p).c_str(), "=");
    if (tok.GetCurrent() == nullptr)
    {
      continue;
    }
    xmlWriter.StartElement("env");
    xmlWriter.AddAttribute("name", tok.GetCurrent());
    ++tok;
    if (tok.GetCurrent() != nullptr)
    {
      xmlWriter.Text(tok.GetCurrent());
    }
    xmlWriter.EndElement();
  }
  xmlWriter.EndElement();
  FreeEnvironmentStringsW(lpszEnv);
}
#endif

void IniTeXMFApp::ReportBrokenPackages()
{
  vector<string> broken;
  unique_ptr<PackageIterator> pkgIter(packageManager->CreateIterator());
  PackageInfo packageInfo;
  for (int idx = 0; pkgIter->GetNext(packageInfo); ++idx)
  {
    if (!packageInfo.IsPureContainer()
      && packageInfo.IsInstalled()
      && packageInfo.deploymentName.compare(0, 7, "miktex-") == 0)
    {
      if (!(packageManager->TryVerifyInstalledPackage(packageInfo.deploymentName)))
      {
        broken.push_back(packageInfo.deploymentName);
      }
    }
  }
  pkgIter->Dispose();
  if (broken.size() > 0)
  {
    if (xml)
    {
      xmlWriter.StartElement("packages");
      for (const string & name : broken)
      {
        xmlWriter.StartElement("package");
        xmlWriter.AddAttribute("name", name);
        xmlWriter.AddAttribute("integrity", "damaged");
        xmlWriter.EndElement();
      }
      xmlWriter.EndElement();
    }
    else
    {
      for (const string & name : broken)
      {
        cout << name << ": " << T_("needs to be reinstalled") << endl;
      }
    }
  }
}

void IniTeXMFApp::ReportLine(const string & str)
{
  Verbose("%s", str.c_str());
}

bool IniTeXMFApp::OnRetryableError(const string & message)
{
  return false;
}

bool IniTeXMFApp::OnProgress(Notification nf)
{
  UNUSED_ALWAYS(nf);
  return true;
}

#if !defined(MIKTEX_STANDALONE)
void IniTeXMFApp::Configure()
{
  ProcessOutput output;
  int exitCode;
  output.Clear();
  Process::ExecuteSystemCommand("kpsewhich --expand-path \\$TEXMF", &exitCode, &output, 0);
  output.RemoveTrailingNewline();
  if (exitCode == 0 && !output.IsEmpty())
  {
    if (session->IsAdminMode())
    {
      if (!startuconfig.commonRoots.empty())
      {
        startuconfig.commonRoots += PathName::PathNameDelimiter;
      }
      startuconfig.commonRoots += output.ToString();
    }
    else
    {
      if (!startuconfig.userRoots.empty())
      {
        startuconfig.userRoots += PathName::PathNameDelimiter;
      }
      startuconfig.userRoots += output.ToString();
    }
    SetTeXMFRootDirectories();
  }
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
        Verbose(T_("Skipping user root directory (%s)..."), Q_(session->GetRootDirectory(r)));
      }
    }
    else
    {
      if (!session->IsCommonRootDirectory(r))
      {
        UpdateFilenameDatabase(r);
      }
      else
      {
        Verbose(T_("Skipping common root directory (%s)..."), Q_(session->GetRootDirectory(r)));
      }
    }
  }
  packageManager->CreateMpmFndb();
}
#endif

void IniTeXMFApp::CreatePortableSetup(const PathName & portableRoot)
{
  unique_ptr<Cfg> config(Cfg::Create());
  config->PutValue("Auto", "Config", "Portable");
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
}

void IniTeXMFApp::WriteReport()
{
  if (xml)
  {
    xmlWriter.StartDocument();
    xmlWriter.StartElement("miktexreport");
  }
  ReportMiKTeXVersion();
  ReportOSVersion();
  ReportRoots();
  if (xml)
  {
    ReportFndbFiles();
  }
#if defined(MIKTEX_WINDOWS)
  if (xml)
  {
    ReportEnvironmentVariables();
  }
#endif
  ReportBrokenPackages();
  if (xml)
  {
    xmlWriter.EndElement();
  }
}

bool IniTeXMFApp::OnFndbItem(const char * lpszPath, const char * lpszName, const char * lpszInfo, bool isDirectory)
{
  if (recursive)
  {
    PathName path(lpszPath, lpszName);
    const char * lpszRel =
      Utils::GetRelativizedPath(path.Get(), enumDir.Get());
    if (!isDirectory)
    {
      if (lpszInfo == nullptr)
      {
        cout << lpszRel << endl;
      }
      else
      {
        if (csv)
        {
          cout << lpszRel << ";" << lpszInfo << endl;
        }
        else
        {
          cout << lpszRel << " (\"" << lpszInfo << "\")" << endl;
        }
      }
    }
    if (isDirectory)
    {
      Fndb::Enumerate(path, this);
    }
  }
  else
  {
    if (lpszInfo == nullptr)
    {
      cout << (isDirectory ? "D" : " ") << " " << lpszName << endl;
    }
    else
    {
      cout << (isDirectory ? "D" : " ") << " " << lpszName << " (\"" << lpszInfo << "\")" << endl;
    }
  }
  return true;
}

void IniTeXMFApp::Run(int argc, const char * argv[])
{
  vector<string> addFiles;
  vector<string> showConfigValue;
  vector<string> setConfigValues;
  vector<string> createConfigFiles;
  vector<string> editConfigFiles;
  vector<string> formats;
  vector<string> formatsByName;
  vector<string> listDirectories;
  vector<string> removeFiles;
  vector<string> updateRoots;
  vector<PathName> registerRoots;
  vector<PathName> unregisterRoots;
  string defaultPaperSize;
  string engine;
  string logFile;
  string portableRoot;

#if !defined(MIKTEX_STANDALONE)
  bool optConfigure = false;
#endif

  bool optDump = false;
  bool optDumpByName = false;
  bool optForce = false;
  bool optMakeLanguageDat = false;
  bool optMakeMaps = false;
  bool optListFormats = false;
  bool optListModes = false;
  bool optMakeLinks = isTexlinksMode;
#if defined(MIKTEX_WINDOWS)
  bool optNoRegistry = false;
#endif
  bool optPortable = false;
  bool optRegisterShellFileTypes = false;
  bool optModifyPath = false;
  bool optReport = false;
  bool optUnRegisterShellFileTypes = false;
  bool optUpdateFilenameDatabase = isMktexlsrMode;
  bool optVersion = false;

  const struct poptOption * aoptions;

  if (setupWizardRunning)
  {
    aoptions = aoption_setup;
  }
  else if (updateWizardRunning)
  {
    aoptions = aoption_update;
  }
  else if (isMktexlsrMode)
  {
    aoptions = aoption_mktexlsr;
  }
  else if (isTexlinksMode)
  {
    aoptions = aoption_texlinks;
  }
  else
  {
    aoptions = aoption_user;
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

#if !defined(MIKTEX_STANDALONE)
    case OPT_CONFIGURE:
      optConfigure = true;
      break;
#endif

    case OPT_CREATE_CONFIG_FILE:

      createConfigFiles.push_back(optArg);
      break;

    case OPT_CSV:
      csv = true;
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

    case OPT_FORCE:

      optForce = true;
      break;

    case OPT_COMMON_INSTALL:

      startupConfig.commonInstallRoot = optArg;
      break;

    case OPT_USER_INSTALL:

      startupConfig.userInstallRoot = optArg;
      break;

    case OPT_LIST_DIRECTORY:

      listDirectories.push_back(optArg);
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

    case OPT_PRINT_ONLY:

      printOnly = true;
      break;

    case OPT_QUIET:

      quiet = true;
      break;

    case OPT_RECURSIVE:

      recursive = true;
      break;

    case OPT_REGISTER_SHELL_FILE_TYPES:

      optRegisterShellFileTypes = true;
      break;

    case OPT_REGISTER_ROOT:

      registerRoots.push_back(optArg);
      break;

    case OPT_REMOVE_FILE:

      removeFiles.push_back(optArg);
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

      adminMode = true;
      break;

    case OPT_UNREGISTER_ROOT:

      unregisterRoots.push_back(optArg);
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

    case OPT_XML:
      xml = true;
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

  if (!popt.GetLeftovers().empty())
  {
    FatalError(T_("This utility does not accept non-option arguments."));
  }

  if (optVersion)
  {
    cout
      << Utils::MakeProgramVersionString(TheNameOfTheGame, MIKTEX_COMPONENT_VERSION_STR) << endl
      << endl
      << "Copyright (C) 1996-2016 Christian Schenk" << endl
      << "This is free software; see the source for copying conditions.  There is NO" << endl
      << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
    return;
  }

  if (!logFile.empty())
  {
    if (File::Exists(logFile))
    {
      logStream.Attach(File::Open(logFile, FileMode::Append, FileAccess::Write));
    }
    else
    {
      logStream.Attach(File::Open(logFile, FileMode::Create, FileAccess::Write));
    }
  }

  if (adminMode)
  {
    if (!setupWizardRunning && !session->IsSharedSetup())
    {
      FatalError(T_("Option --admin only makes sense for a shared MiKTeX setup."));
    }
    if (!session->RunningAsAdministrator())
    {
#if defined(MIKTEX_WINDOWS)
      FatalError(T_("Option --admin requires administrator privileges."));
#else
      FatalError(T_("Option --admin requires root privileges."));
#endif
    }
    session->SetAdminMode(true, setupWizardRunning);
  }

  if (optPortable)
  {
    CreatePortableSetup(portableRoot);
  }

  if (!startupConfig.userRoots.empty()
    || !startupConfig.userDataRoot.Empty()
    || !startupConfig.userConfigRoot.Empty()
    || !startupConfig.userInstallRoot.Empty()
    || !startupConfig.commonRoots.empty()
    || !startupConfig.commonDataRoot.Empty()
    || !startupConfig.commonConfigRoot.Empty()
    || !startupConfig.commonInstallRoot.Empty())
  {
#if !defined(MIKTEX_STANDALONE)
    optConfigure = true;
#else
#if defined(MIKTEX_WINDOWS)
    SetTeXMFRootDirectories(optNoRegistry);
#else
    SetTeXMFRootDirectories();
#endif
#endif
  }

  if (!defaultPaperSize.empty())
  {
    session->SetDefaultPaperSize(defaultPaperSize.c_str());
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

  if (optMakeLinks)
  {
    MakeLinks(optForce);
  }

  if (optMakeMaps)
  {
    MakeMaps(optForce);
  }

  for (const string & fileName : addFiles)
  {
    Verbose(T_("Adding %s to the file name database..."), Q_(fileName));
    PrintOnly("fndbadd %s", Q_(fileName));
    if (!printOnly)
    {
      if (!Fndb::FileExists(fileName))
      {
        Fndb::Add(fileName);
      }
      else
      {
        Warning(T_("%s is already recorded in the file name database"), Q_(fileName));
      }
    }
  }

  for (const string & fileName : removeFiles)
  {
    Verbose(T_("Removing %s from the file name database..."), Q_(fileName));
    PrintOnly("fndbremove %s", Q_(fileName));
    if (!printOnly)
    {
      if (Fndb::FileExists(fileName))
      {
        Fndb::Remove(fileName);
      }
      else
      {
        Warning(T_("%s is not recorded in the file name database"), Q_(fileName));
      }
    }
  }

  if (removeFndb)
  {
    RemoveFndb();
  }

  if (!unregisterRoots.empty())
  {
    RegisterRoots(unregisterRoots, false);
  }

  if (!registerRoots.empty())
  {
    RegisterRoots(registerRoots, true);
  }

  if (optUpdateFilenameDatabase)
  {
    if (updateRoots.size() == 0)
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
            Verbose(T_("Skipping user root directory (%s)..."), Q_(session->GetRootDirectory(r)));
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
            Verbose(T_("Skipping common root directory (%s)..."), Q_(session->GetRootDirectory(r)));
          }
        }
      }
      packageManager->CreateMpmFndb();
    }
    else
    {
      for (const string & r : updateRoots)
      {
        UpdateFilenameDatabase(r.c_str());
      }
    }
  }

  for (const string & dir : listDirectories)
  {
    enumDir = dir;
    Fndb::Enumerate(dir, this);
  }

  for (const string & fileName : createConfigFiles)
  {
    CreateConfigFile(fileName, false);
  }

  for (const string & v : setConfigValues)
  {
    SetConfigValue(v);
  }

  for (const string & v : showConfigValue)
  {
    ShowConfigValue(v);
  }

  for (const string & fileName : editConfigFiles)
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

#if !defined(MIKTEX_STANDALONE)
  if (optConfigure)
  {
    Configure();
  }
#endif

  if (optReport)
  {
    WriteReport();
  }
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
    IniTeXMFApp app;
    app.Init(newargv[0]);
    app.Run(argc, &newargv[0]);
    app.Finalize();
    return 0;
  }
  catch (const MiKTeXException & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    LOG4CXX_FATAL(logger, "Info: " << e.GetInfo());
    LOG4CXX_FATAL(logger, "Source: " << e.GetSourceFile());
    LOG4CXX_FATAL(logger, "Line: " << e.GetSourceLine());
    Sorry();
    return 1;
  }
  catch (const exception & e)
  {
    LOG4CXX_FATAL(logger, e.what());
    Sorry();
    return 1;
  }
  catch (int exitCode)
  {
    return exitCode;
  }
}
