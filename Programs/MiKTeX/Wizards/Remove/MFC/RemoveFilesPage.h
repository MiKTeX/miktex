/* RemoveFilesPage.h:                                   -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the Remove MiKTeX! Wizard.

   The Remove MiKTeX! Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The Remove MiKTeX! Wizard is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the Remove MiKTeX! Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "Remove.h"

class RemoveFilesPage :
  public CPropertyPage,
  public SetupServiceCallback
{
private:
  enum { IDD = IDD_REMOVE_FILES };

protected:
  DECLARE_MESSAGE_MAP();

public:
  RemoveFilesPage();

public:
  virtual ~RemoveFilesPage();

protected:
  virtual BOOL OnInitDialog();

protected:
  virtual BOOL OnSetActive();

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  virtual BOOL OnQueryCancel();

protected:
  afx_msg LRESULT OnStartRemovingFiles(WPARAM wParam, LPARAM lParam);

protected:
  afx_msg void OnTimer(UINT_PTR nIDEvent);

public:
  bool GetCancelFlag();

public:
  void SetReadyFlag();

private:
  virtual bool MIKTEXTHISCALL OnProcessOutput(const void * pOutput, size_t n);

public:
  virtual void MIKTEXTHISCALL ReportLine(const std::string & str);

public:
  virtual bool MIKTEXTHISCALL OnRetryableError(const std::string & message);

public:
  virtual bool MIKTEXTHISCALL OnProgress(MiKTeX::Setup::Notification nf);

private:
  static UINT WorkerThread(void * pParam);

private:
  void RemoveMiKTeX();

private:
  unique_ptr<SetupService> pSetupService;

private:
  CProgressCtrl progressControl;

private:
  CCriticalSection criticalSectionMonitor;

private:
  CString currentFileName;

private:
  unsigned long progress;

private:
  unsigned long total;

private:
  bool ready;

private:
  HANDLE hWorkerThread = nullptr;

private:
  class RemoveWizard * pSheet = nullptr;
};
