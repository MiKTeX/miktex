/* PropSheet.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of MiKTeX Options.

   MiKTeX Options is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Options is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Options; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "StdAfx.h"
#include "mo.h"

#include "PropSheet.h"

IMPLEMENT_DYNAMIC(PropSheet, CPropertySheet);

PropSheet::PropSheet(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager) :
  pManager(pManager),
  generalPage(pManager),
  packagesPage(pManager),
  languagesPage(pManager)
{
  m_psh.dwFlags &= ~(PSH_HASHELP);
  AddPage(&generalPage);
  AddPage(&texmfRootsPage);
  AddPage(&formatsPage);
  AddPage(&languagesPage);
  AddPage(&packagesPage);
  if (pSession->IsAdminMode())
  {
    SetTitle(T_(_T("MiKTeX Options (Admin)")));
  }
  else
  {
    SetTitle(T_(_T("MiKTeX Options")));
  }
}

PropSheet::~PropSheet()
{
}

BEGIN_MESSAGE_MAP(PropSheet, CPropertySheet)
END_MESSAGE_MAP();

BOOL PropSheet::OnInitDialog()
{
  BOOL result = CPropertySheet::OnInitDialog();
  try
  {
    HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    if (hIcon == nullptr)
    {
      MIKTEX_FATAL_WINDOWS_ERROR("CWinApp::LoadIcon");
    }
    SetIcon(hIcon, TRUE);
    ModifyStyleEx(0, WS_EX_CONTEXTHELP);
    m_psh.dwFlags &= ~(PSH_HASHELP);
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  return result;
}

bool PropSheet::OnProcessOutput(const void * pOutput, size_t n)
{
  processOutput.append(reinterpret_cast<const char*>(pOutput), n);
  return !pProgressDialog->HasUserCancelled();
}

bool PropSheet::RunIniTeXMF(const char * lpszTitle, const CommandLineBuilder & cmdLine, ProgressDialog * pProgressDialog)
{
  PathName exePath;

  if (!pSession->FindFile("initexmf", FileType::EXE, exePath))
  {
    MIKTEX_UNEXPECTED();
  }

  CommandLineBuilder commandLine(cmdLine);

  commandLine.AppendOption("--verbose");

  if (pSession->IsAdminMode())
  {
    commandLine.AppendOption("--admin");
  }

  this->pProgressDialog = pProgressDialog;

  if (pProgressDialog->HasUserCancelled())
  {
    return false;
  }

  pProgressDialog->SetLine(2, lpszTitle);

  pSession->UnloadFilenameDatabase();

  processOutput = "";
  int exitCode;
  Process::Run(exePath, commandLine.ToString(), this, &exitCode, 0);

  this->pProgressDialog = 0;

  if (exitCode == 0)
  {
    return true;
  }

  if (AfxMessageBox(T_(_T("The MiKTeX configuration utility failed. Do you want to see the transcript?")), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
  {
    TextViewerDialog::DoModal(this, T_("Transcript"), processOutput.c_str());
  }

  return false;
}

void PropSheet::BuildFormats()
{
  auto_ptr<ProgressDialog> pProgDlg(ProgressDialog::Create());
  pProgDlg->StartProgressDialog(GetSafeHwnd());
  pProgDlg->SetTitle(T_("MiKTeX Maintenance"));
  pProgDlg->SetLine(1, T_("Creating language.dat, ..."));
  RunIniTeXMF("language.dat", CommandLineBuilder("--mklangs"), pProgDlg.get());
  pProgDlg->SetLine(1, T_("Creating format file for:"));
  for (const FormatInfo & formatInfo : pSession->GetFormats())
  {
    if (formatInfo.exclude)
    {
      continue;
    }
    CommandLineBuilder cmdLine;
    cmdLine.AppendOption("--dump=", formatInfo.name);
    RunIniTeXMF(formatInfo.description.c_str(), cmdLine, pProgDlg.get());
  }
  pProgDlg->StopProgressDialog();
  pProgDlg.reset();;
  mustBuildFormats = false;
}
