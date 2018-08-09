/* AddChangeEditorDialog.cpp:

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of Yap.

   Yap is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Yap is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Yap; if not, write to the Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "StdAfx.h"

#include "yap.h"

#include "ErrorDialog.h"

#include "AddChangeEditorDialog.h"

BEGIN_MESSAGE_MAP(AddChangeEditorDialog, CDialog)
  ON_BN_CLICKED(IDC_BROWSE, &AddChangeEditorDialog::OnBrowse)
  ON_EN_CHANGE(IDC_ARGUMENTS, &AddChangeEditorDialog::OnChangeArguments)
  ON_EN_CHANGE(IDC_PATH, &AddChangeEditorDialog::OnChangePath)
  ON_EN_CHANGE(IDC_NAME, &AddChangeEditorDialog::OnChangeName)
END_MESSAGE_MAP()

AddChangeEditorDialog::AddChangeEditorDialog(CWnd* pParent, const vector<EditorInfo>& editors, int firstCustomIdx) :
  CDialog(AddChangeEditorDialog::IDD, pParent),
  editors(editors),
  firstCustomIdx(firstCustomIdx),
  currentIdx(-1),
  name(T_("New Editor")),
  adding(true)
{
}

AddChangeEditorDialog::AddChangeEditorDialog(CWnd* pParent, const vector<EditorInfo>& editors, int firstCustomIdx, int currentIdx) :
  CDialog(AddChangeEditorDialog::IDD, pParent),
  editors(editors),
  firstCustomIdx(firstCustomIdx),
  currentIdx(currentIdx),
  name(editors[currentIdx].name.c_str()),
  program(editors[currentIdx].program.c_str()),
  arguments(editors[currentIdx].arguments.c_str()),
  adding(false)
{
}

BOOL AddChangeEditorDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();

  try
  {
    HResult hr = SHAutoComplete(programEdit.m_hWnd, SHACF_FILESYSTEM);
    EnableButtons();
    if (hr.Failed())
    {
#if 1
      TraceError(T_("Auto completion error: %s"), hr.GetText());
#else
      MIKTEX_FATAL_ERROR_2(T_("Auto completion error."), "hr", hr.GetText());
#endif

    }
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(0, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(0, e);
  }

  return ret;
}

void AddChangeEditorDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_NAME, nameEdit);
  DDX_Text(pDX, IDC_NAME, name);
  DDX_Control(pDX, IDC_PATH, programEdit);
  DDX_Text(pDX, IDC_PATH, program);
  DDX_Control(pDX, IDC_ARGUMENTS, argumentsEdit);
  DDX_Text(pDX, IDC_ARGUMENTS, arguments);
  if (pDX->m_bSaveAndValidate)
  {
    pDX->PrepareEditCtrl(IDC_NAME);
    for (int idx = firstCustomIdx; idx < static_cast<int>(editors.size()); ++idx)
    {
      if (idx != currentIdx && Utils::EqualsIgnoreCase(editors[idx].name.c_str(), TU_(name)))
      {
        AfxMessageBox(T_(_T("The specified editor name already exists.")));
        pDX->Fail();
    }
  }
    pDX->PrepareEditCtrl(IDC_PATH);
    if (!File::Exists(PathName(TU_(program))))
    {
      AfxMessageBox(T_(_T("The specified program file does not exist.")));
      pDX->Fail();
    }
    pDX->PrepareEditCtrl(IDC_ARGUMENTS);
    if (!(arguments.Find(_T("%f")) >= 0))
    {
      AfxMessageBox(T_(_T("Missing file name (%f) in argument string.")));
      pDX->Fail();
    }
    if (!(arguments.Find(_T("%l")) >= 0))
    {
      AfxMessageBox(T_(_T("Missing line number (%l) in argument string.")));
      pDX->Fail();
    }
}
}

void AddChangeEditorDialog::OnBrowse()
{
  try
  {
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetSafeHwnd();
    ofn.hInstance = nullptr;
    wstring filter;
    filter += T_(L"Executables (*.exe)");
    filter += L'\0';
    filter += T_(L"*.exe");
    filter += L'\0';
    filter += T_(L"All files (*.*)");
    filter += L'\0';
    filter += L"*.*";
    filter += L'\0';
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrCustomFilter = nullptr;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    wchar_t szFileName[BufferSizes::MaxPath];
    szFileName[0] = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = BufferSizes::MaxPath;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    PathName pathSample;
    PathName pathInitialDir;
    ofn.lpstrInitialDir = nullptr;
    ofn.lpstrTitle = nullptr;
    ofn.Flags = 0;
    ofn.Flags |= OFN_FILEMUSTEXIST;
    ofn.Flags |= OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L".exe";
    if (!GetOpenFileNameW(&ofn))
    {
      // TODO: check error condition with CommDlgExtendedError
      return;
    }
    programEdit.SetWindowText(CW2T(ofn.lpstrFile));
    EnableButtons();
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void AddChangeEditorDialog::OnChangeName()
{
  try
  {
    EnableButtons();
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void AddChangeEditorDialog::OnChangePath()
{
  try
  {
    EnableButtons();
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void AddChangeEditorDialog::OnChangeArguments()
{
  try
  {
    EnableButtons();
  }
  catch (const MiKTeXException& e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception& e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void AddChangeEditorDialog::EnableButtons()
{
  CWnd* pOk = GetDlgItem(IDOK);
  if (pOk == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  CString name;
  nameEdit.GetWindowText(name);
  CString program;
  programEdit.GetWindowText(program);
  CString arguments;
  argumentsEdit.GetWindowText(arguments);
  pOk->EnableWindow(!(name.IsEmpty() || program.IsEmpty() || arguments.IsEmpty())
      && arguments.Find(_T("%f")) >= 0
      && arguments.Find(_T("%l")) >= 0);
}

EditorInfo AddChangeEditorDialog::GetEditorInfo()
{
  return EditorInfo(TU_(name), TU_(program), TU_(arguments));
}
