/* Update.cpp:

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of MiKTeX Update Wizard.

   MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Update Wizard; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"
#include "Update.h"

#include "UpdateWizard.h"

UpdateWizardApplication theApp;

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("Update"));

shared_ptr<PackageManager> UpdateWizardApplication::packageManager;

bool UpdateWizardApplication::upgrading = false;

class UpdateWizardCommandLineInfo : public CCommandLineInfo
{
public:
  UpdateWizardCommandLineInfo()
    : dryRun(false)
  {
  }

public:
  bool dryRun;
};

void AddArgument(const CString & argument, int & argc, char ** & argv, int & argMax)
{
  if (argc == argMax)
  {
    argMax += 10;
    argv = reinterpret_cast<char**>(realloc(argv, argMax * sizeof(argv[0])));
  }
  argv[argc++] = strdup(TU_(argument));
}

void GetArguments(const char * lpszCommandLine, const char * lpszExeName, int & argc, char ** & argv)
{
  argc = 0;
  argv = 0;

  int argMax = 0;

  AddArgument(CString(lpszExeName), argc, argv, argMax);

  CString argument;
  bool copying = false;
  bool inQuotedArg = false;

  for (const char * lpsz = lpszCommandLine; *lpsz != 0; ++lpsz)
  {
    if (*lpsz == ' ' && !inQuotedArg)
    {
      if (copying)
      {
        AddArgument(argument, argc, argv, argMax);
        argument = "";
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
      argument += *lpsz;
      copying = true;
    }
  }

  if (copying)
  {
    AddArgument(argument, argc, argv, argMax);
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
  argv = 0;
}

#define OPT_HELP 'h'
#define OPT_DRYRUN 1

const struct option long_options[] =
{
  "dry-run",            no_argument,            0,      OPT_DRYRUN,
  "help",               no_argument,            0,      OPT_HELP,
  0,                    no_argument,            0,      0,
};

void ShowHelpAndExit(int retCode = 0)
{
  ostrstream os;
  os
    << T_("Usage: updatewiz [OPTIONS]") << endl
    << endl
    << "  --dry-run" << endl
    << "  --help" << endl;
  AfxMessageBox(UT_(os.str()));
  exit(retCode);
}

void ParseUpdateCommandLine(const char * lpszCommandLine, UpdateWizardCommandLineInfo & cmdinfo)
{
  int argc;
  char ** argv;

  GetArguments(lpszCommandLine, TU_(AfxGetAppName()), argc, argv);

  int iOptIdx = 0;
  int c;

  cmdinfo.m_nShellCommand = CCommandLineInfo::FileNothing;

  optind = 0;

  while ((c = getopt_long_only(argc, argv, "", long_options, &iOptIdx)) != EOF)
  {
    switch (c)
    {

    case OPT_HELP:
      FreeArguments(argc, argv);
      ShowHelpAndExit();
      break;

    case OPT_DRYRUN:
      cmdinfo.dryRun = true;
      break;

    default:
      FreeArguments(argc, argv);
      ShowHelpAndExit(1);
      break;

    }
  }

  FreeArguments(argc, argv);
}

BEGIN_MESSAGE_MAP(UpdateWizardApplication, CWinApp)
END_MESSAGE_MAP();

UpdateWizardApplication::UpdateWizardApplication()
{
  SetAppID(UT_("MiKTeXorg.MiKTeX.Update." MIKTEX_COMPONENT_VERSION_STR));
}

void UpdateWizardApplication::FlushPendingTraceMessages()
{
  for (vector<TraceCallback::TraceMessage>::const_iterator it = pendingTraceMessages.begin(); it != pendingTraceMessages.end(); ++it)
  {
    TraceInternal(*it);
  }
  pendingTraceMessages.clear();
}

void UpdateWizardApplication::TraceInternal(const TraceCallback::TraceMessage & traceMessage)
{
  log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(string("trace.update.") + traceMessage.facility);

  if (traceMessage.streamName == MIKTEX_TRACE_ERROR)
  {
    LOG4CXX_ERROR(logger, traceMessage.message);
  }
  else
  {
    LOG4CXX_TRACE(logger, traceMessage.message);
  }
}

void UpdateWizardApplication::Trace(const TraceCallback::TraceMessage & traceMessage)
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

BOOL UpdateWizardApplication::InitInstance()
{
#if 0
  if (!Upgrade(upgrading))
  {
    return FALSE;
  }
#endif

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

  try
  {
    Session::InitInfo initInfo(T_("update"));
    initInfo.SetTraceCallback(this);
    shared_ptr<Session> pSession = Session::Create(initInfo);
    PathName xmlFileName;
    if (pSession->FindFile("updatewiz." MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName)
      || pSession->FindFile(MIKTEX_LOG4CXX_CONFIG_FILENAME, MIKTEX_PATH_TEXMF_PLACEHOLDER "/" MIKTEX_PATH_MIKTEX_PLATFORM_CONFIG_DIR, xmlFileName))
    {
      Utils::SetEnvironmentString("MIKTEX_LOG_DIR", pSession->GetSpecialPath(SpecialPath::DataRoot).AppendComponent(MIKTEX_PATH_MIKTEX_LOG_DIR).Get());
      Utils::SetEnvironmentString("MIKTEX_LOG_NAME", "update");
      log4cxx::xml::DOMConfigurator::configure(xmlFileName.ToWideCharString());
      isLog4cxxConfigured = true;
      LOG4CXX_INFO(logger, "starting: " << Utils::MakeProgramVersionString("Update Wizard", MIKTEX_COMPONENT_VERSION_STR));
    }

    if (!pSession->IsMiKTeXPortable() && !Utils::CheckPath(false))
    {
      if (AfxMessageBox(T_(_T("MiKTeX is not correctly configured: the location of the MiKTeX executables is not known to the operating system.\r\n\r\nClick OK to repair the MiKTeX configuration.")), MB_OKCANCEL) == IDOK)
      {
        Utils::CheckPath(true);
      }
      else
      {
        if (pSession ->GetConfigValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_INSIST_ON_REPAIR, true))
        {
          MIKTEX_FATAL_ERROR(T_("MiKTeX is not correctly configured: the environment variable PATH contains no MiKTeX references."));
        }
      }
    }

    packageManager = PackageManager::Create(PackageManager::InitInfo(this));

    // get command-line arguments
    UpdateWizardCommandLineInfo cmdinfo;
    ParseUpdateCommandLine(TU_(m_lpCmdLine), cmdinfo);

    {
      UpdateWizard dlg;
      m_pMainWnd = &dlg;
      dlg.DoModal();
    }

    packageManager->UnloadDatabase();

    packageManager = nullptr;

    pSession = nullptr;
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

bool UpdateWizardApplication::Upgrade(bool & upgrading)
{
  bool done = false;
  upgrading = false;
  try
  {
    Session::InitInfo initInfo("upgrade", { Session::InitOption::NoConfigFiles });
    shared_ptr<Session> pSession = Session::Create(initInfo);
    wchar_t szPath[BufferSizes::MaxPath];
    if (GetModuleFileNameW(0, szPath, BufferSizes::MaxPath) == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("GetModuleFileName");
    }
    PathName path(szPath);
    if (path.GetFileNameWithoutExtension() == "upgrade")
    {
      PathName migrate;
      if (!pSession->FindFile("migrate", FileType::EXE, migrate))
      {
        MIKTEX_UNEXPECTED();
      }
      ProcessOutput<1024> processOutput;
      if (!Process::Run(migrate, nullptr, &processOutput, nullptr, nullptr))
      {
        MIKTEX_UNEXPECTED();
      }
      upgrading = true;
    }
    done = true;
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }
  return done;
}

void ReportError(const MiKTeXException & e)
{
  string str = T_("Sorry, but something went wrong:");
  str += "\n\n";
  str += e.what();
  if (!e.GetInfo().empty())
  {
    str += "\n\n";
    str += e.GetInfo();
  }
  AfxMessageBox(UT_(str), MB_OK | MB_ICONSTOP);
}

void ReportError(const exception & e)
{
  string str = T_("Sorry, but something went wrong:");
  str += "\n\n";
  str += e.what();
  AfxMessageBox(UT_(str), MB_OK | MB_ICONSTOP);
}

void SplitUrl(const string & url, string & protocol, string & host)
{
  wchar_t szProtocol[200];
  wchar_t szHost[200];
  URL_COMPONENTSW url_comp = { 0 };
  url_comp.dwStructSize = sizeof(url_comp);
  url_comp.lpszScheme = szProtocol;
  url_comp.dwSchemeLength = 200;
  url_comp.lpszHostName = szHost;
  url_comp.dwHostNameLength = 200;
  if (!InternetCrackUrlW(UW_(url.c_str()), 0, 0, &url_comp))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("InternetCrackUrlW");
  }
  protocol = WU_(szProtocol);
  host = WU_(szHost);
}
