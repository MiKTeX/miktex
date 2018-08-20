/* FileCopyPage.h:                                      -*- C++ -*-

   Copyright (C) 1999-2018 Christian Schenk

   This file is part of the MiKTeX Setup Wizard.

   The MiKTeX Setup Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Setup Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Setup Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

class FileCopyPage :
  public CPropertyPage,
  public SetupServiceCallback
{
private:
  enum { IDD = IDD_FILECOPY };

protected:
  DECLARE_MESSAGE_MAP();

public:
  FileCopyPage();

public:
  ~FileCopyPage() override;

protected:
  BOOL OnInitDialog() override;

protected:
  BOOL OnSetActive() override;

protected:
  void DoDataExchange(CDataExchange* dx) override;

protected:
  LRESULT OnWizardNext() override;

protected:
  BOOL OnKillActive() override;

protected:
  BOOL OnQueryCancel() override;

protected:
  afx_msg LRESULT OnStartFileCopy(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg LRESULT OnReport(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg LRESULT OnProgress(WPARAM wParam, LPARAM lParam);

private:
  bool MIKTEXTHISCALL OnProcessOutput(const void* output, size_t n) override;

public:
  void MIKTEXTHISCALL ReportLine(const std::string& str) override;

public:
  bool MIKTEXTHISCALL OnRetryableError(const std::string& message) override;

public:
  bool MIKTEXTHISCALL OnProgress(MiKTeX::Setup::Notification nf) override;

private:
  static UINT WorkerThread(void* fileCopyPage);

private:
  void Report(bool writeLog, const char* format, ...);

private:
  CWnd* GetControl(UINT id);

private:
  void EnableControl(UINT id, bool enable);

private:
  void ReportError(const exception& e);

private:
  void ReportError(const MiKTeXException& e);

private:
  CAnimateCtrl animationControl;

private:
  CProgressCtrl progressControl1;

private:
  CProgressCtrl progressControl2;

private:
  CRichEditCtrl reportControl;

private:
  CCriticalSection criticalSectionMonitor;

private:
  struct SharedData
  {
    DWORD secondsRemaining = 0;
    bool newPackage = false;
    bool ready = false;
    bool waitingForClickOnNext = false;
    int progress1Pos = 0;
    int progress2Pos = 0;
    string packageName;
    string currentLine;
  };

private:
  SharedData sharedData;

private:
  HANDLE hWorkerThread = nullptr;

private:
  DWORD timeOfLastProgressRefresh;

private:
  class SetupWizard* sheet = nullptr;

private:
  shared_ptr<Session> session = Session::Get();
};
