/* FileCopyPage.cpp: the actual update process

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

#include "FileCopyPage.h"
#include "UpdateWizard.h"

IMPLEMENT_DYNCREATE(FileCopyPage, CPropertyPage);

const SHORT PROGRESS_MAX = 1000;

const UINT WM_STARTFILECOPY = WM_APP + 1;
const UINT WM_PROGRESS = WM_APP + 2;

BEGIN_MESSAGE_MAP(FileCopyPage, CPropertyPage)
  ON_MESSAGE(WM_PROGRESS, OnProgress)
  ON_MESSAGE(WM_STARTFILECOPY, OnStartFileCopy)
END_MESSAGE_MAP();

FileCopyPage::FileCopyPage() :
  CPropertyPage(IDD, 0, IDS_HEADER_UPDATE, IDS_SUBHEADER_UPDATE)
{
}

FileCopyPage::~FileCopyPage()
{
  try
  {
    if (hWorkerThread != nullptr)
    {
      CloseHandle(hWorkerThread);
      hWorkerThread = nullptr;
    }
    if (pInstaller != nullptr)
    {
      pInstaller->Dispose();
      pInstaller = nullptr;
    }
  }
  catch (const exception &)
  {
  }
}

BOOL FileCopyPage::OnInitDialog()
{
  pSheet = reinterpret_cast<UpdateWizard*>(GetParent());
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    pInstaller = UpdateWizardApplication::packageManager->CreateInstaller();
    pInstaller->SetNoPostProcessing(true);
    reportEditBox.LimitText(100000);
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }
  return ret;
}

BOOL FileCopyPage::OnSetActive()
{
  BOOL ret = CPropertyPage::OnSetActive();
  if (ret)
  {
    try
    {
      pSheet->ClearErrorFlag();

      pInstaller->SetRepository(pSheet->GetRepository());

      // disable buttons
      pSheet->SetWizardButtons(0);

      // starting shot
      if (!PostMessage(WM_STARTFILECOPY))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
      }
    }
    catch (const MiKTeXException & e)
    {
      ReportError(e);
      ret = FALSE;
    }
    catch (const exception & e)
    {
      ReportError(e);
      ret = FALSE;
    }
  }

  return ret;
}

void FileCopyPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_ANI, animationControl);
  DDX_Control(pDX, IDC_PROGRESS1, progressControl1);
  DDX_Control(pDX, IDC_PROGRESS2, progressControl2);
  DDX_Control(pDX, IDC_REPORT, reportEditBox);
  DDX_Text(pDX, IDC_REPORT, report);
}

LRESULT FileCopyPage::OnWizardNext()
{
  pSheet->SetCameFrom(0);
  return reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_FINISH));
}

BOOL FileCopyPage::OnKillActive()
{
  BOOL ret = CPropertyPage::OnKillActive();
  return ret;
}

BOOL FileCopyPage::OnQueryCancel()
{
  if (hWorkerThread == nullptr)
  {
    return CPropertyPage::OnQueryCancel();
  }
  try
  {
    LOG4CXX_INFO(logger, "Cancel? ");
    if (AfxMessageBox(IDS_CANCEL_UPDATE, MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
    {
      LOG4CXX_INFO(logger, "  Yes!");
      pSheet->SetCancelFlag();
      if (!PostMessage(WM_PROGRESS))
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
      }
    }
    else
    {
      LOG4CXX_INFO(logger, "  No!");
    }
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }
  return FALSE;
}

LRESULT FileCopyPage::OnStartFileCopy(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    if (!animationControl.Open (pSheet->GetRepositoryType() == RepositoryType::Remote ? IDA_DOWNLOAD : IDA_FILECOPY))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Open");
    }

    if (!animationControl.Play(0, static_cast<UINT>(-1), static_cast<UINT>(-1)))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Play");
    }

    // initialize progress bar controls
    progressControl1.SetRange(0, PROGRESS_MAX);
    progressControl1.SetPos(0);
    progressControl2.SetRange(0, PROGRESS_MAX);
    progressControl2.SetPos(0);

    // create the worker thread
    CWinThread * pThread = AfxBeginThread(WorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    MIKTEX_ASSERT(pThread != nullptr);
    MIKTEX_ASSERT(pThread->m_hThread != nullptr);
    if (!DuplicateHandle(GetCurrentProcess(), pThread->m_hThread, GetCurrentProcess(), &hWorkerThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR(T_("DuplicateHandle"));
    }
    pThread->ResumeThread();
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
  }
  catch (const exception & e)
  {
    ReportError(e);
  }

  return 0;
}

LRESULT FileCopyPage::OnProgress(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);

  try
  {
    CSingleLock singleLock(&criticalSectionMonitor, TRUE);

    // update the report
    if (sharedData.reportUpdate)
    {
      reportEditBox.SetWindowText(UT_(sharedData.report.c_str()));
      reportEditBox.SetSel(100000, 100000);
      sharedData.reportUpdate = false;
    }

    // do we have to finish?
    if (sharedData.ready
      || pSheet->GetCancelFlag()
      || pSheet->GetErrorFlag())
    {
      // check to see if we are already ready
      if (sharedData.waitingForClickOnNext)
      {
        return 0;
      }

      sharedData.waitingForClickOnNext = true;

      // stop the video
      if (!animationControl.Stop())
      {
        MIKTEX_FATAL_WINDOWS_ERROR("CAnimateCtrl::Stop");
      }
      animationControl.Close(); // always returns FALSE
      animationControl.ShowWindow(SW_HIDE);

      // disable controls
      EnableControl(IDC_PROGRESS1_TITLE, false);
      GetControl(IDC_PACKAGE)->SetWindowText(_T(""));
      EnableControl(IDC_PACKAGE, false);
      progressControl1.SetPos(0);
      progressControl1.EnableWindow(FALSE);
      EnableControl(IDC_PROGRESS2_TITLE, false);
      progressControl2.SetPos(0);
      progressControl2.EnableWindow(FALSE);
#if 0
      GetControl(IDC_ETA_TITLE)->ShowWindow(SW_HIDE);
      GetControl(IDC_ETA)->SetWindowText("");
      GetControl(IDC_ETA)->ShowWindow(SW_HIDE);
#endif

      // enable Next button
      pSheet->SetWizardButtons(PSWIZB_NEXT);
    }
    else
    {
      // show the package name
      if (sharedData.newPackage)
      {
        GetControl(IDC_PACKAGE)->SetWindowText(UT_(sharedData.packageName));
        sharedData.newPackage = false;
      }

      // update progress bars
      progressControl1.SetPos(sharedData.progress1Pos);
      progressControl2.SetPos(sharedData.progress2Pos);

#if 0
      // update ETA
      if (GetTickCount() > timeOfLastProgressRefresh + 1000)
      {
        CTimeSpan timeRemaining(sharedData.secondsRemaining);
        CString str;
        if (timeRemaining.GetTotalHours() > 0)
        {
          str.Format(T_("%u hours, %u minutes"),
            timeRemaining.GetTotalHours(),
            timeRemaining.GetMinutes());
        }
        else if (timeRemaining.GetTotalMinutes() > 0)
        {
          str.Format(T_("%u minutes"),
            timeRemaining.GetTotalMinutes());
        }
        else
        {
          str.Format(T_("%u seconds"),
            timeRemaining.GetTotalSeconds());
        }
        timeOfLastProgressRefresh = GetTickCount();
        GetControl(IDC_ETA)->SetWindowText(str);
      }
#endif
    }
  }
  catch (const MiKTeXException & e)
  {
    ReportError(e);
    pSheet->SetWizardButtons(PSWIZB_NEXT);
    sharedData.waitingForClickOnNext = true;
  }
  catch (const exception & e)
  {
    ReportError(e);
    pSheet->SetWizardButtons(PSWIZB_NEXT);
    sharedData.waitingForClickOnNext = true;
  }
  return 0;
}

bool FileCopyPage::OnProcessOutput(const void * pOutput, size_t n)
{
  Report(true, "%.*s", static_cast<int>(n), reinterpret_cast<const char *>(pOutput));
  return !(pSheet->GetErrorFlag() || pSheet->GetCancelFlag());
}

void FileCopyPage::ReportLine(const string & str)
{
  Report(true, "%s\n", str.c_str());
}

bool FileCopyPage::OnRetryableError(const string & message)
{
  UINT uType = MB_ICONSTOP;
  uType |= MB_RETRYCANCEL;
  string str = message;
  str += T_("  Then click Retry to complete the operation.");
  UINT u = ::MessageBoxW(nullptr, UW_(str.c_str()), nullptr, uType);
  return u != IDCANCEL;
}

bool FileCopyPage::OnProgress(Notification nf)
{
  CSingleLock singlelock(&criticalSectionMonitor, TRUE);
  bool visibleProgress = false;
  PackageInstaller::ProgressInfo progressInfo = pInstaller->GetProgressInfo();
  if (nf == Notification::InstallPackageStart || nf == Notification::DownloadPackageStart)
  {
    visibleProgress = true;
    sharedData.newPackage = true;
    sharedData.packageName = progressInfo.displayName;
  }
  if (progressInfo.cbPackageDownloadTotal > 0)
  {
    int oldValue = sharedData.progress1Pos;
    sharedData.progress1Pos = static_cast<int>
      (((static_cast<double>(progressInfo.cbPackageDownloadCompleted)
        / progressInfo.cbPackageDownloadTotal)
        * PROGRESS_MAX));
    visibleProgress = (visibleProgress || (sharedData.progress1Pos != oldValue));
  }
  if (progressInfo.cbDownloadTotal > 0)
  {
    int oldValue = sharedData.progress2Pos;
    sharedData.progress2Pos = static_cast<int>
      (((static_cast<double>(progressInfo.cbDownloadCompleted)
        / progressInfo.cbDownloadTotal)
        * PROGRESS_MAX));
    visibleProgress = (visibleProgress || (sharedData.progress2Pos != oldValue));
  }
  DWORD oldValue = sharedData.secondsRemaining;
  sharedData.secondsRemaining = static_cast<DWORD>(progressInfo.timeRemaining / 1000);
  visibleProgress = (visibleProgress || (sharedData.secondsRemaining != oldValue));
  if (visibleProgress)
  {
    if (!PostMessage(WM_PROGRESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
  return !(pSheet->GetErrorFlag() || pSheet->GetCancelFlag());
}

UINT FileCopyPage::WorkerThread(void * pParam)
{
  FileCopyPage * This = reinterpret_cast<FileCopyPage*>(pParam);

  This->timeOfLastProgressRefresh = 0;

  try
  {
    This->DoTheUpdate();
  }
  catch (const MiKTeXException & e)
  {
    This->ReportError(e);
  }
  catch (const exception & e)
  {
    This->ReportError(e);
  }

  This->sharedData.ready = true;

  try
  {
    if (!This->PostMessage(WM_PROGRESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
  catch (const MiKTeXException & e)
  {
    This->ReportError(e);
  }
  catch (const exception & e)
  {
    This->ReportError(e);
  }

  return 0;
}

void FileCopyPage::DoTheUpdate()
{
  pInstaller->SetCallback(this);

#if 0 // already done that
  // update the package database
  pInstaller->UpdateDb();
#endif

  if (pSheet->GetCancelFlag())
  {
    return;
  }

  pInstaller->SetFileLists(pSheet->GetUpdateList(), pSheet->GetRemoveList());

  // run installer
  pInstaller->InstallRemove();

  if (pSheet->GetCancelFlag())
  {
    return;
  }

  // run IniTeXMF
  ConfigureMiKTeX();
}

void FileCopyPage::ConfigureMiKTeX()
{
#if REMOVE_FORMAT_FILES
  RemoveFormatFiles();
#endif

  if (UpdateWizardApplication::upgrading)
  {
    RemoveFalseConfigFiles();
    RemoveOldRegistrySettings();
  }

  // register components, configure files
  RunMpm(CommandLineBuilder("--register-components"));

  CommandLineBuilder cmdLine;

  cmdLine.Clear();
  cmdLine.AppendOption("--update-fndb");
  RunIniTeXMF(cmdLine);

  cmdLine.Clear();
  cmdLine.AppendOption("--force");
  cmdLine.AppendOption("--mklinks");
  RunIniTeXMF(cmdLine);

  cmdLine.Clear();
#if 0 // experimental
  cmdLine.AppendOption("--force");
#endif
  cmdLine.AppendOption("--mkmaps");
  cmdLine.AppendOption("--mklangs");
  RunIniTeXMF(cmdLine);
}

void FileCopyPage::RunMpm(const CommandLineBuilder & cmdLine1)
{
  PathName exePath;

  if (!session->FindFile(MIKTEX_MPM_EXE, FileType::EXE, exePath))
  {
    MIKTEX_UNEXPECTED();
  }

  // make command line
  CommandLineBuilder cmdLine(cmdLine1);
  cmdLine.AppendOption("--verbose");
  if (session->IsAdminMode())
  {
    cmdLine.AppendOption("--admin");
  }

  // run mpm.exe
  LOG4CXX_INFO(logger, "running 'mpm " << cmdLine.ToString() << "'");

  Process::Run(exePath, cmdLine.ToString(), this);
}

void FileCopyPage::RunIniTeXMF(const CommandLineBuilder & cmdLine1)
{
  PathName exePath;

  if (!session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, exePath))
  {
    MIKTEX_UNEXPECTED();
  }

  // make command line
  CommandLineBuilder cmdLine(cmdLine1);
  cmdLine.AppendOption("--verbose");
  if (session->IsAdminMode())
  {
    cmdLine.AppendOption("--admin");
  }

  // run initexmf.exe
  LOG4CXX_INFO(logger, "running 'initexmf " << cmdLine.ToString() << "'");

  session->UnloadFilenameDatabase();

  Process::Run(exePath, cmdLine.ToString(), this);
}

void FileCopyPage::Report(bool withLog, const char * lpszFmt, ...)
{
  MIKTEX_ASSERT(lpszFmt != nullptr);
  va_list args;
  va_start(args, lpszFmt);
  string str = StringUtil::FormatString(lpszFmt, args);
  va_end(args);
  int len = str.length();
  CSingleLock singlelock(&criticalSectionMonitor, TRUE);
  for (int i = 0; i < len; ++i)
  {
    if (str[i] == '\n')
    {
      if (i == 0 || str[i - 1] != '\r')
      {
        sharedData.currentLine += '\r';
      }
      sharedData.currentLine += '\n';
      sharedData.reportUpdate = true;
      sharedData.report += sharedData.currentLine;
      sharedData.currentLine.clear();
      if (withLog)
      {
        LOG4CXX_INFO(logger, sharedData.currentLog);
        sharedData.currentLog.clear();
      }
    }
    else
    {
      sharedData.currentLine += str[i];
      if (withLog && str[i] != '\r')
      {
        sharedData.currentLog += str[i];
      }
    }
  }
  if (sharedData.reportUpdate)
  {
    if (!PostMessage(WM_PROGRESS))
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWnd::PostMessage");
    }
  }
}

CWnd * FileCopyPage::GetControl(UINT controlId)
{
  CWnd * pWnd = GetDlgItem(controlId);
  if (pWnd == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return pWnd;
}

void FileCopyPage::EnableControl(UINT controlId, bool enable)
{
  GetControl(controlId)->EnableWindow(enable ? TRUE : FALSE);
}

void FileCopyPage::CollectFiles(vector<string> & vec, const PathName & dir, const char * lpszExt)
{
  unique_ptr<DirectoryLister> lister = DirectoryLister::Open(dir, lpszExt);
  DirectoryEntry entry;
  while (lister->GetNext(entry))
  {
    vec.push_back(entry.name);
  }
  lister->Close();
}

#if REMOVE_FORMAT_FILES
void FileCopyPage::RemoveFormatFiles()
{
  PathName pathFmt (session->GetSpecialPath(SpecialPath::DataRoot));
  pathFmt /= MIKTEX_PATH_FMT_DIR;
  if (!Directory::Exists(pathFmt))
  {
    return;
  }
  vector<string> toBeDeleted;
  CollectFiles(toBeDeleted, pathFmt, MIKTEX_FORMAT_FILE_SUFFIX);
  for (const string & f : toBeDeleted)
  {
    File::Delete(PathName(pathFmt, f));
  }
}
#endif

static const char * const configFiles[] = {
  MIKTEX_PATH_PDFTEXCONFIG_TEX,
  T_("tex\\generic\\config\\language.dat"),
  T_("web2c\\updmap.cfg"),
};

static const char * const configDirs[] = {
  MIKTEX_PATH_DVIPDFMX_CONFIG_DIR,
  MIKTEX_PATH_DVIPDFM_CONFIG_DIR,
  MIKTEX_PATH_DVIPS_CONFIG_DIR,
  MIKTEX_PATH_MIKTEX_CONFIG_DIR,
  MIKTEX_PATH_PDFTEX_CONFIG_DIR,
};

void FileCopyPage::RemoveFalseConfigFiles()
{
#if 1
  MIKTEX_UNEXPECTED();
#else
  shared_ptr<Session> pSession = Session::Get();
  PathName installRoot = pSession->GetSpecialPath(SpecialPath::InstallRoot);
  PathName configRoot = pSession->GetSpecialPath(SpecialPath::ConfigRoot);
  unsigned n = pSession->GetNumberOfTEXMFRoots();
  for (unsigned r = 0; r < n; ++r)
  {
    PathName path = pSession->GetRootDirectory(r);
    if (path != installRoot && path != configRoot)
    {
      for (size_t idx = 0; idx < sizeof(configFiles) / sizeof(configFiles[0]); ++idx)
      {
        PathName file(path, configFiles[idx]);
        if (File::Exists(file))
        {
          File::Delete(file);
        }
      }
      for (size_t idx = 0; idx < sizeof(configDirs) / sizeof(configDirs[0]); ++idx)
      {
        PathName dir(path, configDirs[idx]);
        if (Directory::Exists(dir))
        {
          Directory::Delete(dir, true);
        }
      }
    }
  }
#endif
}

void FileCopyPage::RemoveOldRegistrySettings()
{
#if 1
  MIKTEX_UNEXPECTED();
#else
  shared_ptr<Session> pSession = Session::Get();

  CRegKey root25;

  bool sharedSetup = (pSession->IsSharedMiKTeXSetup() == TriState::True);

  if ((root25.Open(sharedSetup ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
    MIKTEX_REGPATH_SERIES "\\" MIKTEX_REGKEY_MIGRATE,
    KEY_READ))
    != ERROR_SUCCESS)
  {
    FATAL_MIKTEX_ERROR("FileCopyPage::RemoveOldRegistrySettings",
      T_("Missing registry settings."),
      MIKTEX_REGKEY_MIGRATE);
  }

  char szVersion[20];
  ULONG nChars = 20;

  if (root25.QueryStringValue(MIKTEX_REGVAL_VERSION, szVersion, &nChars)
    != ERROR_SUCCESS)
  {
    FATAL_MIKTEX_ERROR("FileCopyPage::RemoveOldRegistrySettings",
      T_("Missing registry value."),
      MIKTEX_REGVAL_VERSION);
  }

  string rootKey;
  string subKey;

  if (Utils::Equals("2.4", szVersion))
  {
    rootKey = T_("Software\\MiK\\MiKTeX");
    subKey = T_("CurrentVersion");
  }
  else
  {
    rootKey = MIKTEX_REGPATH_PRODUCT;
    subKey = szVersion;
  }

  CRegKey regRoot;

  if (regRoot.Open(HKEY_CURRENT_USER, rootKey.c_str()) == ERROR_SUCCESS)
  {
    regRoot.RecurseDeleteKey(subKey.c_str());
    regRoot.Close();
  }

  if (sharedSetup
    && (regRoot.Open(HKEY_LOCAL_MACHINE, rootKey.c_str()) == ERROR_SUCCESS))
  {
    regRoot.RecurseDeleteKey(subKey.c_str());
    regRoot.Close();
  }
#endif
}

void FileCopyPage::ReportError(const MiKTeXException & e)
{
  Report(true, T_("\nError: %s\n"), e.what());
  pSheet->ReportError(e);
}

void FileCopyPage::ReportError(const exception & e)
{
  Report(true, T_("\nError: %s\n"), e.what());
  pSheet->ReportError(e);
}
