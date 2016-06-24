/* FileCopyPage.h:                                      -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of the MiKTeX Update Wizard.

   The MiKTeX Update Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Update Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Update Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include "resource.h"

class FileCopyPage :
  public CPropertyPage,
  public IRunProcessCallback,
  public PackageInstallerCallback
{
private:
  DECLARE_DYNCREATE(FileCopyPage);

private:
  enum { IDD = IDD_FILECOPY };

protected:
  DECLARE_MESSAGE_MAP();

public:
  FileCopyPage();

public:
  virtual ~FileCopyPage();

protected:
  virtual BOOL OnInitDialog();

protected:
  virtual BOOL OnSetActive();

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  virtual LRESULT OnWizardNext();

protected:
  virtual BOOL OnKillActive();

protected:
  virtual BOOL OnQueryCancel();

protected:
  afx_msg LRESULT OnStartFileCopy(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg LRESULT OnProgress(WPARAM wParam, LPARAM lParam);

private:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n);

public:
  virtual void MIKTEXTHISCALL ReportLine(const std::string & str);

public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const std::string & message);

public:
  virtual bool MIKTEXTHISCALL OnProgress(Notification nf);

private:
  static UINT WorkerThread(void * pParam);

private:
  void DoTheUpdate();

private:
  void ConfigureMiKTeX();

private:
  void RunMpm(const CommandLineBuilder & cmdLine1);

private:
  void RunIniTeXMF(const CommandLineBuilder & cmdLine1);

private:
  void Report(bool withLog, const char * lpszFormat, ...);

private:
  void CollectFiles(vector<string> & vec, const PathName & dir, const char * lpszExt);

private:
  void RemoveFormatFiles();

private:
  void RemoveFalseConfigFiles();

private:
  void RemoveOldRegistrySettings();

private:
  CWnd * GetControl(UINT controlId);

private:
  void EnableControl(UINT controlId, bool enable);

private:
  void ReportError(const exception & e);

private:
  void ReportError(const MiKTeXException & e);

private:
  CCriticalSection criticalSectionMonitor;

private:
  struct SharedData
  {
    string currentLine;
    string currentLog;
    string report;
    DWORD secondsRemaining = 0;
    bool newPackage = false;
    bool ready = false;
    bool reportUpdate = false;
    bool waitingForClickOnNext = false;
    int progress1Pos = 0;
    int progress2Pos = 0;
    string packageName;
  };

private:
  SharedData sharedData;

private:
  shared_ptr<PackageInstaller> pInstaller;

private:
  class UpdateWizard * pSheet = nullptr;

private:
  HANDLE hWorkerThread = nullptr;

private:
  DWORD timeOfLastProgressRefresh;

private:
  CAnimateCtrl animationControl;

private:
  CEdit reportEditBox;

private:
  CProgressCtrl progressControl1;

private:
  CProgressCtrl progressControl2;

private:
  CString report;

private:
  std::shared_ptr<Session> session = Session::Get();

private:
  log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("FileCopyPage");
};
