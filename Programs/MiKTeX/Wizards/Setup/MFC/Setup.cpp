/* Setup.cpp:

   Copyright (C) 1999-2016 Christian Schenk

   This file is part of MiKTeX Setup Wizard.

   MiKTeX Setup Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Setup Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Setup Wizard; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"
#include "Setup.h"

#include "SetupWizard.h"

// this runs the wizard
SetupApp setupWizardApplication;

SetupApp * SetupApp::Instance = &setupWizardApplication;

class SetupCommandLineInfo
  : public CCommandLineInfo
{
public:
  string folderName;

public:
  StartupConfig startupConfig;

public:
  PathName localPackageRepository;

public:
  string remotePackageRepository;

public:
  SetupTask task = SetupTask::None;

public:
  bool optAllowUnattendedReboot = false;

public:
  bool optDryRun = false;

#if ENABLE_ADDTEXMF
public:
  bool optNoAddTEXMFDirs = false;
#endif

public:
  bool optNoRegistry = false;

public:
  bool optShared = false;

public:
  bool optPrivate = false;

public:
  bool optPortable = false;

public:
  bool optUnattended = false;

public:
  PackageLevel packageLevel = PackageLevel::None;

#if FEATURE_1874934
public:
  TriState installOnTheFly = TriState::Undetermined;
#endif

#if FEATURE_1874934
public:
  string paperSize;
#endif

public:
  PathName optPortableRoot;
};

void AddArgument (const string & argument, int & argc, char ** & argv, int & argMax)
{
  if (argc == argMax)
  {
    argMax += 10;
    argv = reinterpret_cast<char**>(realloc(argv, argMax * sizeof(argv[0])));
  }
  argv[argc++] = strdup(argument.c_str());
}

void GetArguments (const char * lpszCommandLine, const char * lpszExeName, int & argc, char ** & argv)
{
  argc = 0;
  argv = nullptr;

  int argMax = 0;

  AddArgument(string(lpszExeName), argc, argv, argMax);

  string arg;

  bool copying = false;
  bool inQuotedArg = false;

  const char * lpsz = lpszCommandLine;

  while (*lpsz != 0)
  {
    if (*lpsz == ' ' && !inQuotedArg)
    {
      if (copying)
      {
        AddArgument(arg, argc, argv, argMax);
        arg = "";
        copying = false;
      }
    }
    else if (*lpsz == '"')
    {
      inQuotedArg = !inQuotedArg;
      copying = true;
    }
    else
    {
      arg += *lpsz;
      copying = true;
    }
    ++lpsz;
  }

  if (copying)
  {
    AddArgument(arg, argc, argv, argMax);
  }
}

void FreeArguments(int argc, char ** & argv)
{
  for (int i = 0; i < argc; ++i)
  {
    free(argv[i]);
    argv[i] = 0;
  }
  free(argv);
  argv = nullptr;
}

enum {
  OPT_AAA = 1000,
  OPT_ALLOW_UNATTENDED_REBOOT,
#if FEATURE_1874934
  OPT_AUTO_INSTALL,
#endif
  OPT_COMMON_CONFIG,
  OPT_COMMON_DATA,
  OPT_COMMON_INSTALL,
  OPT_COMMON_ROOTS,
  OPT_DOWNLOAD_ONLY,
  OPT_DRY_RUN,
  OPT_HELP,
  OPT_INSTALL_FROM_LOCAL_REPOSITORY,
  OPT_LOCAL_PACKAGE_REPOSITORY,
  OPT_NO_ADDITIONAL_ROOTS,
  OPT_NO_REGISTRY,
  OPT_PACKAGE_SET,
#if FEATURE_1874934
  OPT_PAPER_SIZE,
#endif
  OPT_PORTABLE,
  OPT_PRIVATE,
  OPT_PROGRAM_FOLDER,
  OPT_REMOTE_PACKAGE_REPOSITORY,
  OPT_SHARED,
  OPT_UNATTENDED,
  OPT_USER_CONFIG,
  OPT_USER_DATA,
  OPT_USER_INSTALL,
  OPT_USER_ROOTS,
};

const struct option long_options[] =
{
  { "allow-unattended-reboot", no_argument, 0, OPT_ALLOW_UNATTENDED_REBOOT },
#if FEATURE_1874934
  { "auto-install", required_argument, 0, OPT_AUTO_INSTALL },
#endif
  { "common-config", required_argument, 0, OPT_COMMON_CONFIG },
  { "common-data", required_argument, 0, OPT_COMMON_DATA },
  { "common-install", required_argument, 0, OPT_COMMON_INSTALL },
  { "common-roots", required_argument, 0, OPT_COMMON_ROOTS },
  { "download-only", no_argument, 0, OPT_DOWNLOAD_ONLY },
  { "dry-run", no_argument, 0, OPT_DRY_RUN },
  { "help", no_argument, 0, OPT_HELP },
  { "install-from-local-repository", no_argument, 0, OPT_INSTALL_FROM_LOCAL_REPOSITORY },
  { "local-package-repository", required_argument, 0, OPT_LOCAL_PACKAGE_REPOSITORY },
  { "no-additional-roots", no_argument, 0, OPT_NO_ADDITIONAL_ROOTS },
  { "no-registry", no_argument, 0, OPT_NO_REGISTRY },
  { "package-set", required_argument, 0, OPT_PACKAGE_SET },
#if FEATURE_1874934
  { "paper-size", required_argument, 0, OPT_PAPER_SIZE },
#endif
  { "portable", required_argument, 0, OPT_PORTABLE },
  { "private", no_argument, 0, OPT_PRIVATE },
  { "program-folder", required_argument, 0, OPT_PROGRAM_FOLDER },
  { "remote-package-repository", required_argument, 0, OPT_REMOTE_PACKAGE_REPOSITORY },
  { "shared", no_argument, 0, OPT_SHARED },
  { "unattended", no_argument, 0, OPT_UNATTENDED },
  { "user-config", required_argument, 0, OPT_USER_CONFIG },
  { "user-data", required_argument, 0, OPT_USER_DATA },
  { "user-install", required_argument, 0, OPT_USER_INSTALL },
  { "user-roots", required_argument, 0, OPT_USER_ROOTS },
  { 0, no_argument, 0, 0 }
};

void ShowHelpAndExit(int retCode = 0)
{
  ostringstream os;
  os
    << T_("Usage: setupwiz [OPTIONS]") << endl
    << endl
    << T_("Options:") << endl
    << endl
    << "  --allow-unattended-reboot" << endl
#if FEATURE_1874934
    << "  --auto-install=yes" << endl
    << "  --auto-install=no" << endl
    << "  --auto-install=ask" << endl
#endif
    << "  --common-config=DIR" << endl
    << "  --common-data=DIR" << endl
    << "  --common-install=DIR" << endl
    << "  --common-roots=DIRS" << endl
    << "  --download-only" << endl
    << "  --dry-run" << endl
    << "  --help" << endl
    << "  --install-from-local-repository" << endl
    << "  --local-package-repository=DIR" << endl
    << "  --no-additional-roots" << endl
    << "  --no-registry" << endl
    << "  --package-set=SET" << endl
#if FEATURE_1874934
    << "  --paper-size=A4" << endl
    << "  --paper-size=Letter" << endl
#endif
    << "  --portable=DIR" << endl
    << "  --private" << endl
    << "  --program-folder=NAME" << endl
    << "  --remote-package-repository=URL" << endl
    << "  --shared" << endl
    << "  --unattended" << endl
    << "  --user-config=DIR" << endl
    << "  --user-data=DIR" << endl
    << "  --user-install=DIR" << endl
    << "  --user-roots=DIRS" << endl
    << endl
    << T_("setupwiz reads its arguments from setupwiz.opt, if such a file exists.") << endl
    << T_("See the MiKTeX Manual for more information.") << endl;
  AfxMessageBox(UT_(os.str()));
  exit(retCode);
}

void CheckStartupConfig(StartupConfig & startupConfig)
{
#if 1
  string commonRoots;
  for (CSVList tok(startupConfig.commonRoots, ';'); tok.GetCurrent() != nullptr; ++tok)
  {
    PathName path(tok.GetCurrent());
    if (path.Empty())
    {
      continue;
    }
    if (startupConfig.commonConfigRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --common-roots value collides with --common-config value."), "path", path.ToString());
  }
    if (startupConfig.commonDataRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --common-roots value collides with --common-data value."), "path", path.ToString());
    }
    if (startupConfig.commonInstallRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --common-roots value collides with --common-install value."), "path", path.ToString());
    }
    if (startupConfig.userConfigRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --common-roots value collides with --user-config value."), "path", path.ToString());
    }
    if (startupConfig.userDataRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --common-roots value collides with --user-data value."), "path", path.ToString());
    }
    if (startupConfig.userInstallRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --common-roots value collides with --user-install value."), "path", path.ToString());
    }
    if (!commonRoots.empty())
    {
      commonRoots += ';';
    }
    commonRoots += path.Get();
  }
  startupConfig.commonRoots = commonRoots;

  string userRoots;
  for (CSVList tok(startupConfig.userRoots, ';'); tok.GetCurrent() != nullptr; ++tok)
  {
    PathName path(tok.GetCurrent());
    if (path.Empty())
    {
      continue;
    }
    if (StringUtil::Contains(startupConfig.commonRoots.c_str(), path.Get(), ";", true))
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --user-roots value collides with --common-roots value."), "path", path.ToString());
    }
    if (startupConfig.commonConfigRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --user-roots value collides with --common-config value."), "path", path.ToString());
    }
    if (startupConfig.commonDataRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --user-roots value collides with --common-data value."), "path", path.ToString());
    }
    if (startupConfig.commonInstallRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --user-roots value collides with --common-install value."), "path", path.ToString());
    }
    if (startupConfig.userConfigRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --user-roots value collides with --user-config value."), "path", path.ToString());
    }
    if (startupConfig.userDataRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --user-roots value collides with --user-data value."), "path", path.ToString());
    }
    if (startupConfig.userInstallRoot == path)
    {
      MIKTEX_FATAL_ERROR_2(T_("Improper options: --user-roots value collides with --user-install value."), "path", path.ToString());
    }
    if (!userRoots.empty())
    {
      userRoots += ';';
    }
    userRoots += path.Get();
  }
  startupConfig.userRoots = userRoots;
#endif
}

void ParseSetupCommandLine(int argc, char ** argv, SetupCommandLineInfo & cmdinfo)
{
  shared_ptr<Session> session = Session::Get();

  int iOptIdx = 0;
  int c;

  cmdinfo.m_nShellCommand = CCommandLineInfo::FileNothing;

  optind = 0;

  while ((c = getopt_long_only(argc, argv, "", long_options, &iOptIdx)) != EOF)
  {
    switch (c)
    {

    case OPT_ALLOW_UNATTENDED_REBOOT:
      cmdinfo.optAllowUnattendedReboot = true;
      break;

#if FEATURE_1874934
    case OPT_AUTO_INSTALL:
      if (Utils::EqualsIgnoreCase(optarg, "yes"))
      {
        cmdinfo.installOnTheFly = TriState::True;
      }
      else if (Utils::EqualsIgnoreCase(optarg, "no"))
      {
        cmdinfo.installOnTheFly = TriState::False;
      }
      else if (Utils::EqualsIgnoreCase(optarg, "ask"))
      {
        cmdinfo.installOnTheFly = TriState::Undetermined;
      }
      else
      {
        MIKTEX_FATAL_ERROR(T_("--auto-install value must be one of: yes, no, ask."));
      }
      break;
#endif

    case OPT_COMMON_CONFIG:
      if (!(session->RunningAsAdministrator() || session->RunningAsPowerUser()))
      {
        MIKTEX_FATAL_ERROR(T_("You must have administrator privileges to set up a common configuration directory."));
      }
      cmdinfo.startupConfig.commonConfigRoot = optarg;
      break;

    case OPT_COMMON_DATA:
      if (!(session->RunningAsAdministrator() || session->RunningAsPowerUser()))
      {
        MIKTEX_FATAL_ERROR(T_("You must have administrator privileges to set up a common data directory."));
      }
      cmdinfo.startupConfig.commonDataRoot = optarg;
      break;

    case OPT_COMMON_INSTALL:
      if (!(session->RunningAsAdministrator() || session->RunningAsPowerUser()))
      {
        MIKTEX_FATAL_ERROR(T_("You must have administrator privileges to set up a common installation directory."));
      }
      cmdinfo.startupConfig.commonInstallRoot = optarg;
      break;

    case OPT_COMMON_ROOTS:
      if (!(session->RunningAsAdministrator() || session->RunningAsPowerUser()))
      {
        MIKTEX_FATAL_ERROR(T_("You must have administrator privileges to set up common root directories."));
      }
      cmdinfo.startupConfig.commonRoots = optarg;
      break;

    case OPT_DOWNLOAD_ONLY:
      cmdinfo.task = SetupTask::Download;
      break;

    case OPT_DRY_RUN:
      cmdinfo.optDryRun = true;
      break;

    case OPT_HELP:
      FreeArguments(argc, argv);
      ShowHelpAndExit();
      break;

    case OPT_INSTALL_FROM_LOCAL_REPOSITORY:
      cmdinfo.task = SetupTask::InstallFromLocalRepository;
      break;

    case OPT_LOCAL_PACKAGE_REPOSITORY:
      cmdinfo.localPackageRepository = optarg;
      break;

    case OPT_NO_ADDITIONAL_ROOTS:
#if ENABLE_ADDTEXMF
      cmdinfo.optNoAddTEXMFDirs = true;
#endif
      break;

    case OPT_NO_REGISTRY:
      cmdinfo.optNoRegistry = true;
      break;

#if FEATURE_1874934
    case OPT_PAPER_SIZE:
      if (Utils::EqualsIgnoreCase(optarg, "A4"))
      {
        cmdinfo.paperSize = "A4";
      }
      else if (Utils::EqualsIgnoreCase(optarg, "Letter"))
      {
        cmdinfo.paperSize = "Letter";
      }
      else
      {
        MIKTEX_FATAL_ERROR(T_("Value must be one of: A4, Letter."));
      }
      break;
#endif

    case OPT_PACKAGE_SET:
      if (Utils::Equals(optarg, "essential"))
      {
        cmdinfo.packageLevel = PackageLevel::Essential;
      }
      else if (Utils::Equals(optarg, "basic"))
      {
        cmdinfo.packageLevel = PackageLevel::Basic;
      }
      else if (Utils::Equals(optarg, "advanced"))
      {
        cmdinfo.packageLevel = PackageLevel::Advanced;
      }
      else if (Utils::Equals(optarg, "complete"))
      {
        cmdinfo.packageLevel = PackageLevel::Complete;
      }
      else
      {
        MIKTEX_FATAL_ERROR_2(T_("Invalid package set."), "packageSet", optarg);
      }
      break;

    case OPT_PORTABLE:
      cmdinfo.optPortable = true;
      cmdinfo.optPortableRoot = optarg;
      break;

    case OPT_PRIVATE:
      cmdinfo.optPrivate = true;
      break;

    case OPT_PROGRAM_FOLDER:
      cmdinfo.folderName = optarg;
      break;

    case OPT_REMOTE_PACKAGE_REPOSITORY:
      cmdinfo.remotePackageRepository = optarg;
      break;

    case OPT_SHARED:
      if (!(session->RunningAsAdministrator() || session->RunningAsPowerUser()))
      {
        MIKTEX_FATAL_ERROR(T_("You must have administrator privileges to set up a shared MiKTeX system."));
      }
      cmdinfo.optShared = true;
      break;

    case OPT_UNATTENDED:
      cmdinfo.optUnattended = true;
      break;

    case OPT_USER_CONFIG:
      cmdinfo.startupConfig.userConfigRoot = optarg;
      break;

    case OPT_USER_DATA:
      cmdinfo.startupConfig.userDataRoot = optarg;
      break;

    case OPT_USER_INSTALL:
      cmdinfo.startupConfig.userInstallRoot = optarg;
      break;

    case OPT_USER_ROOTS:
      cmdinfo.startupConfig.userRoots = optarg;
      break;

    default:
      FreeArguments(argc, argv);
      ShowHelpAndExit(1);
      break;
    }
  }
}

bool FindFile(const PathName & fileName, PathName & result)
{
  std::shared_ptr<Session> session = Session::Get();

  // try my directory
  result = session->GetMyLocation();
  result /= fileName;
  if (File::Exists(result))
  {
    return true;
  }

  // try the current directory
  result.SetToCurrentDirectory();
  result /= fileName;
  if (File::Exists(result))
  {
    return true;
  }

  return false;
}

bool ReadSetupWizIni(SetupCommandLineInfo & cmdinfo)
{
  PathName fileName;
  if (!FindFile("setupwiz.opt", fileName))
  {
    return false;
  }
  StreamReader reader(fileName);
  string commandLine;
  string line;
  while (reader.ReadLine(line))
  {
    commandLine += ' ';
    commandLine += line;
  }
  reader.Close();
  int argc;
  char ** argv;
  GetArguments(commandLine.c_str(), TU_(AfxGetAppName()), argc, argv);
  ParseSetupCommandLine(argc, argv, cmdinfo);
  FreeArguments(argc, argv);
  return true;
}

BEGIN_MESSAGE_MAP(SetupApp, CWinApp)
#if 0
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
#endif
END_MESSAGE_MAP();

SetupApp::SetupApp()
{
  SetAppID(UT_("MiKTeXorg.MiKTeX.Setup." MIKTEX_COMPONENT_VERSION_STR));
}

#if ENABLE_ADDTEXMF
void CheckAddTEXMFDirs(vector<PathName> & vec)
{
  CSVList path(directories, ';');
  vec.clear();
  directories = "";
  for (; path.GetCurrent() != nullptr; ++path)
  {
    if (!(
      SetupApp::Instance->GetStartupConfig().userDataRoot == path.GetCurrent() ||
      SetupApp::Instance->GetStartupConfig().userConfigRoot == path.GetCurrent() ||
      SetupApp::Instance->GetStartupConfig().userInstallRoot == path.GetCurrent() ||
      SetupApp::Instance->GetStartupConfig().commonDataRoot == path.GetCurrent() ||
      SetupApp::Instance->GetStartupConfig().commonConfigRoot == path.GetCurrent() ||
      SetupApp::Instance->GetStartupConfig().commonInstallRoot == path.GetCurrent()))
    {
      if (vec.size() > 0)
      {
        directories += ';';
      }
      vec.push_back(path.GetCurrent());
      directories += path.GetCurrent();
    }
  }
}
#endif

void SetupGlobalVars(const SetupCommandLineInfo & cmdinfo)
{
  std::shared_ptr<Session> session = Session::Get();

  SetupOptions options = SetupApp::Instance->Service->GetOptions();

  options.IsPrefabricated = false;
  options.IsRegisterPathEnabled = true;

  options.IsDryRun = cmdinfo.optDryRun;
  options.PackageLevel = cmdinfo.packageLevel;
  options.IsPortable = cmdinfo.optPortable;
  options.PortableRoot = cmdinfo.optPortableRoot;

  SetupApp::Instance->AllowUnattendedReboot = cmdinfo.optAllowUnattendedReboot;
  SetupApp::Instance->IsUnattended = cmdinfo.optUnattended;

  // check to see whether setup is started from a MiKTeXDirect location
  SetupApp::Instance->IsMiKTeXDirect = SetupService::IsMiKTeXDirect(options.MiKTeXDirectRoot);
  if (SetupApp::Instance->IsMiKTeXDirect)
  {
    SetupApp::Instance->Service->Log(T_("started from MiKTeXDirect location\n"));
  }

  // startup configuration
  options.Config = cmdinfo.startupConfig;
  if (options.Config.commonInstallRoot.Empty())
  {
    options.Config.commonInstallRoot = SetupService::GetDefaultCommonInstallDir();
  }

  options.IsRegistryEnabled = !cmdinfo.optNoRegistry;

  // shared setup
  options.IsCommonSetup = options.IsCommonSetup && !cmdinfo.optPrivate ||
    cmdinfo.optShared ||
    !cmdinfo.startupConfig.commonRoots.empty() ||
    !cmdinfo.startupConfig.commonInstallRoot.Empty() ||
    !cmdinfo.startupConfig.commonDataRoot.Empty() ||
    !cmdinfo.startupConfig.commonConfigRoot.Empty();

  // auto install
#if FEATURE_1874934
  if (cmdinfo.installOnTheFly != TriState::Undetermined)
  {
    options.IsInstallOnTheFlyEnabled = cmdinfo.installOnTheFly;
  }
#endif

  // paper size
#if FEATURE_1874934
  if (!cmdinfo.paperSize.empty())
  {
    options.PaperSize = cmdinfo.paperSize;
  }
#endif

  if (!cmdinfo.folderName.empty())
  {
    options.FolderName = cmdinfo.folderName;
  }

  // local package repository
  if (!cmdinfo.localPackageRepository.Empty())
  {
    options.LocalPackageRepository = cmdinfo.localPackageRepository;
  }
  if (!options.LocalPackageRepository.Empty())
  {
    if (cmdinfo.task != SetupTask::Download)
    {
      PackageLevel foundPackageLevel = SetupService::TestLocalRepository(options.LocalPackageRepository, options.PackageLevel);
      if (foundPackageLevel == PackageLevel::None)
      {
        MIKTEX_FATAL_ERROR(T_("The specified local repository does not exist."));
      }
      if (options.PackageLevel == PackageLevel::None)
      {
        options.PackageLevel = foundPackageLevel;
      }
      options.IsPrefabricated = true;
    }
  }
  else
  {
    PackageLevel foundPackageLevel = SetupService::SearchLocalRepository(options.LocalPackageRepository, options.PackageLevel, options.IsPrefabricated);
    if (foundPackageLevel != PackageLevel::None)
    {
      if (options.PackageLevel == PackageLevel::None)
      {
        options.PackageLevel = foundPackageLevel;
      }
      if (options.IsPrefabricated)
      {
        SetupApp::Instance->prefabricatedPackageLevel = foundPackageLevel;
      }
      }
    else
    {
      // check the default location
      options.LocalPackageRepository = SetupService::GetDefaultLocalRepository();
      PackageLevel foundPackageLevel = SetupService::TestLocalRepository(options.LocalPackageRepository, options.PackageLevel);
      if (options.PackageLevel == PackageLevel::None)
      {
        options.PackageLevel = foundPackageLevel;
      }
    }
    }

  // setup task
  options.Task = cmdinfo.task;
  if (options.Task == SetupTask::None)
  {
    if (SetupApp::Instance->IsMiKTeXDirect)
    {
      options.Task = SetupTask::InstallFromCD;
    }
    else if (!options.LocalPackageRepository.Empty() && options.PackageLevel != PackageLevel::None)
    {
      options.Task = SetupTask::InstallFromLocalRepository;
    }
  }

  // remote package repository
  options.RemotePackageRepository = cmdinfo.remotePackageRepository;
  if (options.RemotePackageRepository.empty())
  {
    string str;
    if (SetupApp::Instance->packageManager->TryGetRemotePackageRepository(str))
    {
      options.RemotePackageRepository = str;
    }
  }

  // check variables, if started in unattended mode
  if (SetupApp::Instance->IsUnattended)
  {
    if (options.Task == SetupTask::None)
    {
      MIKTEX_FATAL_ERROR(T_("No setup task has been specified."));
    }
    if (options.PackageLevel == PackageLevel::None)
    {
      MIKTEX_FATAL_ERROR(T_("No package set has been specified."));
  }
    if (options.Task == SetupTask::InstallFromLocalRepository || options.Task == SetupTask::Download)
    {
      if (options.LocalPackageRepository.Empty())
      {
        MIKTEX_FATAL_ERROR(T_("No local package repository has been specified."));
      }
    }
    if (options.Task == SetupTask::InstallFromRemoteRepository || options.Task == SetupTask::Download)
    {
      if (options.RemotePackageRepository.empty())
      {
        MIKTEX_FATAL_ERROR(T_("No remote package repository has been specified."));
      }
    }
  }

  options = SetupApp::Instance->Service->SetOptions(options);
}

BOOL SetupApp::InitInstance()
{
  INITCOMMONCONTROLSEX initCtrls;

  initCtrls.dwSize = sizeof(initCtrls);
  initCtrls.dwICC = ICC_WIN95_CLASSES;

  if (!InitCommonControlsEx(&initCtrls))
  {
    AfxMessageBox(T_(_T("The application could not be initialized (1).")), MB_ICONSTOP | MB_OK);
    return FALSE;
  }

  if (FAILED(CoInitialize(nullptr)))
  {
    AfxMessageBox(T_(_T("The application could not be initialized (2).")), MB_ICONSTOP | MB_OK);
    return FALSE;
  }

  AfxInitRichEdit2();

  try
  {
    // create a scratch root directory
    PathName tempDirectory;
    tempDirectory.SetToTempDirectory();
    tempDirectory /= "miktex-setup";
    if (!Directory::Exists(tempDirectory))
    {
      Directory::Create(tempDirectory);
    }
    unique_ptr<TemporaryDirectory> scratchRoot = TemporaryDirectory::Create(tempDirectory);

    // create a MiKTeX session
    StartupConfig startupConfig;
    startupConfig.userInstallRoot = scratchRoot->GetPathName();
    startupConfig.userDataRoot = scratchRoot->GetPathName();
    startupConfig.userConfigRoot = scratchRoot->GetPathName();
    startupConfig.commonDataRoot = scratchRoot->GetPathName();
    startupConfig.commonConfigRoot = scratchRoot->GetPathName();
    startupConfig.commonInstallRoot = scratchRoot->GetPathName();
    Session::InitInfo initInfo("setup", { Session::InitOption::NoConfigFiles });
    initInfo.SetStartupConfig(startupConfig);
    shared_ptr<Session> session = Session::Create(initInfo);

    // create package manager
    packageManager = PackageManager::Create();

    // create setup service
    Service = SetupService::Create();
    SetupOptions options = Service->GetOptions();

    CString banner;
    if (!banner.LoadString(IDS_SETUPWIZ))
    {
      MIKTEX_UNEXPECTED();
    }
    options.Banner = TU_(banner);
    options.Version = MIKTEX_COMPONENT_VERSION_STR;
    options = Service->SetOptions(options);
   
    // set trace options
    traceStream = TraceStream::Open("setupwiz");
    TraceStream::SetTraceFlags("error,extractor,mpm,process,config,setupwiz,setup");

    // extract package archive files
    unique_ptr<TemporaryDirectory> sfxDir = SetupService::ExtractFiles();
    if (sfxDir == nullptr)
    {
      Service->Log("this is a network installer\n");
    }
    else
    {
      Service->Log("this is a self-extracting installer; sfxDir=%s\n", Q_(sfxDir->GetPathName()));
      options.LocalPackageRepository = sfxDir->GetPathName();
      options = Service->SetOptions(options);
    }

    // get command-line arguments
    int argc;
    char ** argv;
    GetArguments(TU_(m_lpCmdLine), TU_(AfxGetAppName()), argc, argv);
    SetupCommandLineInfo cmdinfo;
    ReadSetupWizIni(cmdinfo);
    ParseSetupCommandLine(argc, argv, cmdinfo);
    FreeArguments(argc, argv);
    CheckStartupConfig(cmdinfo.startupConfig);
    if (cmdinfo.optPrivate && cmdinfo.optShared)
    {
      MIKTEX_FATAL_ERROR(T_("You cannot specify --private along with --shared."));
    }
    if (cmdinfo.optPortable && cmdinfo.optShared)
    {
      MIKTEX_FATAL_ERROR(T_("You cannot specify --portable along with --shared."));
    }
    if (cmdinfo.optPrivate && !(
      cmdinfo.startupConfig.commonDataRoot.Empty() &&
      cmdinfo.startupConfig.commonRoots.empty() &&
      cmdinfo.startupConfig.commonInstallRoot.Empty() &&
      cmdinfo.startupConfig.commonConfigRoot.Empty()))
    {
      MIKTEX_FATAL_ERROR(T_("You cannot specify --private along with --common-config, --common-data, --common-install or --common-roots."));
    }
    if (cmdinfo.optPortable && !(
      cmdinfo.startupConfig.commonDataRoot.Empty() &&
      cmdinfo.startupConfig.commonRoots.empty() &&
      cmdinfo.startupConfig.commonInstallRoot.Empty() &&
      cmdinfo.startupConfig.commonConfigRoot.Empty()))
    {
      MIKTEX_FATAL_ERROR(T_("You cannot specify --portable along with --common-config, --common-data, --common-install or --common-roots."));
    }
    if (cmdinfo.optPortable && !(
      cmdinfo.startupConfig.userDataRoot.Empty() &&
      cmdinfo.startupConfig.userRoots.empty() &&
      cmdinfo.startupConfig.userInstallRoot.Empty() &&
      cmdinfo.startupConfig.userConfigRoot.Empty()))
    {
      MIKTEX_FATAL_ERROR(T_("You cannot specify --portable along with --user-config, --user-data, --user-install or --user-roots."));
    }
    SetupGlobalVars(cmdinfo);

    // open the log file
    Service->OpenLog();

    INT_PTR dlgRet;

    // run the wizard
    {
      SetupWizard dlg(packageManager);
      m_pMainWnd = &dlg;
      dlgRet = dlg.DoModal();
    }

    // clean up
    PathName pathLogFile = Service->CloseLog(dlgRet == IDCANCEL);
    if (SetupApp::Instance->ShowLogFileOnExit && !pathLogFile.Empty())
    {
      INT_PTR r = reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", pathLogFile.ToWideCharString().c_str(), nullptr, nullptr, SW_SHOWNORMAL));
      if (r <= 32)
      {
        Process::Start("notepad.exe", pathLogFile.Get());
      }
    }
    traceStream.reset();
    packageManager->UnloadDatabase();
    packageManager = nullptr;
    Service = nullptr;
    session->UnloadFilenameDatabase();
    scratchRoot = nullptr;
    sfxDir = nullptr;
    session = nullptr;
  }

  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }

  CoUninitialize();

  return FALSE;
}

#if 0
bool Reboot()
{
  HANDLE hToken;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
  {
    return false;
  }
  TOKEN_PRIVILEGES tkp;
  LookupPrivilegeValue(0, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
  tkp.PrivilegeCount = 1;
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 0, 0);
  if (GetLastError() != ERROR_SUCCESS)
  {
    return false;
  }
  if (!ExitWindowsEx(EWX_REBOOT, 0))
  {
    return false;
  }
  return true;
}
#endif

void DDV_Path(CDataExchange * pDX, const CString & str)
{
  if (!pDX->m_bSaveAndValidate)
  {
    return;
  }
  CString str2;
  if (isalpha(str[0]) && str[1] == ':' && IsDirectoryDelimiter(str[2]))
  {
    CString driveRoot = str.Left(3);
    if (!Directory::Exists(PathName(driveRoot)))
    {
      CString message;
      message.Format(T_(_T("The specified path is invalid because the root directory %s does not exist.")), static_cast<LPCTSTR>(driveRoot));
      AfxMessageBox(message, MB_ICONEXCLAMATION);
      message.Empty();
      pDX->Fail();
    }
    str2 = str.GetString() + 3;
  }
  else
  {
    PathName uncRoot;
    if (!Utils::GetUncRootFromPath(TU_(str), uncRoot))
    {
      CString message;
      message.Format(T_(_T("The specified path is invalid because it is not fully qualified.")));
      AfxMessageBox(message, MB_ICONEXCLAMATION);
      message.Empty();
      pDX->Fail();
    }
    if (!Directory::Exists(uncRoot))
    {
      CString message;
      message.Format(T_(_T("The specified path is invalid because the UNC root directory %s does not exist.")), uncRoot.Get());
      AfxMessageBox(message, MB_ICONEXCLAMATION);
      message.Empty();
      pDX->Fail();
    }
    str2 = str;
  }
  int i = str2.FindOneOf(_T(":*?\"<>|;="));
  if (i >= 0)
  {
    CString message;
    message.Format(T_(_T("The specified path is invalid because it contains an invalid character (%c).")), str2[i]);
    AfxMessageBox(message, MB_ICONEXCLAMATION);
    message.Empty();
    pDX->Fail();
  }
}

void ReportError(const MiKTeXException & e)
{
  try
  {
    string str = T_("The operation could not be completed for the following reason: ");
    str += "\n\n";
    str += e.what();
    if (!e.GetInfo().empty())
    {
      str += "\n\n";
      str += T_("Details: ");
      str += e.GetInfo();
    }
    AfxMessageBox(UT_(str), MB_OK | MB_ICONSTOP);
    if (SetupApp::Instance->Service != nullptr)
    {
      SetupApp::Instance->Service->Log(T_("\nAn error occurred:\n"));
      SetupApp::Instance->Service->Log(T_("  source file: %s\n"), e.GetSourceFile().c_str());
      SetupApp::Instance->Service->Log(T_("  source line: %d\n"), e.GetSourceLine());
      SetupApp::Instance->Service->Log(T_("  message: %s\n"), e.what());
      SetupApp::Instance->Service->Log(T_("  info: %s\n"), e.GetInfo().c_str());
    }
  }
  catch (const exception &)
  {
  }
}

void ReportError(const exception & e)
{
  try
  {
    string str = T_("The operation could not be completed for the following reason: ");
    str += "\n\n";
    str += e.what();
    SetupApp::Instance->Service->Log("\n%s\n", str.c_str());
    AfxMessageBox(UT_(str), MB_OK | MB_ICONSTOP);
  }
  catch (const exception &)
  {
  }
}
