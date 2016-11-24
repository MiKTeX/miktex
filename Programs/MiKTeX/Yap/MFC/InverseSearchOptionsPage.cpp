/* InverseSearchOptionsPage.cpp:

   Copyright (C) 1996-2016 Christian Schenk

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

/* todo:

   Free
   ====

   texshell /l=%l %f
   "C:\Program Files\LEd\LEd.exe" %f:%l
   "C:\Program Files\wscite\SciTE.exe" "%f" -goto:%l

   Non-free
   ========

*/

#include "StdAfx.h"

#include "yap.h"

#include "InverseSearchOptionsPage.h"

#include "AddChangeEditorDialog.h"

bool ReadPath(HKEY hkeyRoot, const wchar_t * lpszSubKey, const wchar_t * lpszValueName, PathName & path)
{
  HKEY hkey;
  REGSAM samDesired = KEY_READ;
  if (RegOpenKeyExW(hkeyRoot, lpszSubKey, 0, samDesired, &hkey) != ERROR_SUCCESS)
  {
#if defined(MIKTEX_WINDOWS_64)
    samDesired |= KEY_WOW64_32KEY;
#else
    samDesired |= KEY_WOW64_64KEY;
#endif
    if (RegOpenKeyExW(hkeyRoot, lpszSubKey, 0, samDesired, &hkey) != ERROR_SUCCESS)
    {
      return false;
    }
  }
  AutoHKEY autoHKEY(hkey);
  wchar_t szPath[_MAX_PATH];
  unsigned long size = sizeof(szPath);
  unsigned long type;
  long res = RegQueryValueExW(hkey, lpszValueName, nullptr, &type, reinterpret_cast<LPBYTE>(&szPath[0]), &size);
  if (res != ERROR_SUCCESS || type != REG_SZ)
  {
    return false;
  }
  path = szPath;
  return true;
}

bool LocateNTEmacs(PathName & ntEmacs, const char * lpszName)
{
  PathName path;
  if (!ReadPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\GNU\\Emacs", L"emacs_dir", path))
  {
    return false;
  }
  path /= "bin";
  path /= lpszName;
  path.SetExtension(".exe");
  if (!File::Exists(path))
  {
    return false;
  }
  ntEmacs = path;
  return true;
}

bool MakeNTEmacsCommandLine(string & program, string & arguments)
{
  PathName pathEmacs;
  if (!LocateNTEmacs(pathEmacs, "runemacs.exe") && !LocateNTEmacs(pathEmacs, "emacs.exe"))
  {
    return false;
  }
  program = pathEmacs.GetData();
  arguments = "+%l \"%f\"";
  return true;
}

bool MakeNTEmacsClientCommandLine(string & program, string & arguments)
{
  PathName pathEmacs;
  if (!LocateNTEmacs(pathEmacs, "gnuclientw.exe"))
  {
    wchar_t szEmacs[_MAX_PATH];
    wchar_t * lpszFileName;
    if (!SearchPathW(0, L"gnuclientw.exe", 0, _MAX_PATH, szEmacs, &lpszFileName))
    {
      return false;
    }
    pathEmacs = szEmacs;
  }
  program = pathEmacs.GetData();
  arguments = "-F +%l \"%f\"";
  return true;
}

bool MakeXEmacsCommandLine(string & program, string & arguments)
{
  PathName emacsDir;
  if (!ReadPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\xemacs.exe", L"Path", emacsDir))
  {
    return false;
  }
  PathName pathEmacs;
  pathEmacs = emacsDir;
  pathEmacs /= "runemacs.exe";
  if (!File::Exists(pathEmacs))
  {
    pathEmacs = emacsDir;
    pathEmacs /= "xemacs.exe";
    if (!File::Exists(pathEmacs))
    {
      return false;
    }
  }
  program = pathEmacs.ToString();
  arguments = "+%l \"%f\"";
  return true;
}

bool LocateTeXnicCenter(PathName & tc)
{
  PathName path;
  if (ReadPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\ToolsCenter\\TeXnicCenter", L"AppPath", path))
  {
    path /= "TEXCNTR.EXE";;
    if (!File::Exists(path))
    {
      return false;
    }
  }
  else if (ReadPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\ToolsCenter\\TeXnicCenterNT", L"AppPath", path))
  {
    path /= "TeXnicCenter.exe";;
    if (!File::Exists(path))
    {
      return false;
    }
  }
  else
  {
    return false;
  }
  tc = path;
  return true;
}

bool MakeTeXnicCenterCommandLine(string & program, string & arguments)
{
  PathName tc;
  if (!LocateTeXnicCenter(tc))
  {
    return false;
  }
  program = tc.ToString();
  arguments = "/ddecmd \"[goto('%f', '%l')]\"";
  return true;
}

bool LocateProgram(const char * lpszSubDir, const char * lpszFileName, PathName & path)
{
  path = Utils::GetFolderPath(CSIDL_PROGRAM_FILESX86, CSIDL_PROGRAM_FILES, true);
  path /= lpszSubDir;
  path /= lpszFileName;
  return File::Exists(path);
}

bool MakeVisualTeXCommandLine(string & program, string & arguments)
{
  PathName path;
  if (!LocateProgram("Visual-TeX", "vtex.exe", path))
  {
    return false;
  }
  program = path.ToString();
  arguments = "\"%f\" /line:%l";
  return true;
}

bool LocateWinEdt(PathName & winEdt)
{
  if (ReadPath(HKEY_CURRENT_USER, L"Software\\WinEdt", L"Install Root", winEdt))
  {
    winEdt /= "winedt.exe";
    if (File::Exists(winEdt))
    {
      return true;
    }
  }
  return ReadPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\WinEdt.exe", 0, winEdt);
}

bool MakeWinEdtCommandLine(string & program, string & arguments)
{
  PathName pathWinEdt;
  if (!LocateWinEdt(pathWinEdt))
  {
    return false;
  }
  program = pathWinEdt.ToString();
  arguments = "\"[Open(|%f|);SelPar(%l,8)]\"";
  return true;
}

bool MakeLaTeXMngCommandLine(string & program, string & arguments)
{
  PathName path;
  if (!LocateProgram("LaTeXMng", "LaTeXMng.exe", path))
  {
    return false;
  }
  program = path.ToString();
  //latexmng -l%l %f
  arguments = "-l%l \"%f\"";
  return true;
}

bool MakeWinTeXXPCommandLine(string & program, string & arguments)
{
  PathName path;
  if (!LocateProgram("WinTeX", "wintex.exe", path))
  {
    return false;
  }
  program = path.ToString();
  arguments = "-f \"%f\" -l %l";
  return true;
}

bool MakeWinShellCommandLine(string & program, string & arguments)
{
  PathName path;
  if (!LocateProgram("WinShell", "WinShell.exe", path))
  {
    return false;
  }
  program = path.ToString();
  arguments = "-c \"%f\" -l %l";
  return true;
}

bool MakeLaTeXWIDECommandLine(string & program, string & arguments)
{
  PathName path;
  if (!LocateProgram("LaTeX WIDE", "LWide.exe", path))
  {
    return false;
  }
  program = path.ToString();
  arguments = "\"%f\" /l%l";
  return true;
}

BEGIN_MESSAGE_MAP(InverseSearchOptionsPage, CPropertyPage)
  ON_CBN_SELCHANGE(IDC_LISTBOX_EDITOR, OnSelChangeEditor)
  ON_BN_CLICKED(IDC_ADD, &InverseSearchOptionsPage::OnAddEditor)
  ON_BN_CLICKED(IDC_DELETE, &InverseSearchOptionsPage::OnDeleteEditor)
  ON_BN_CLICKED(IDC_CHANGE, &InverseSearchOptionsPage::OnChangeEditor)
END_MESSAGE_MAP();

InverseSearchOptionsPage::InverseSearchOptionsPage() :
  CPropertyPage(IDD)
{
}

BOOL InverseSearchOptionsPage::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();

  try
  {
    CollectEditors();

    int idx = 0;

    for (vector<EditorInfo>::const_iterator it = editors.begin(); it != editors.end(); ++it, ++idx)
    {
      if (g_pYapConfig->inverseSearchCommandLine == editors[idx].GetCommandLine())
      {
        commandLineIdx = idx;
      }
      CString str = UT_(it->name.c_str());
      if (idx < firstCustomIdx)
      {
        str += T_(_T(" (auto-detected)"));
      }
      if (editorListBox.AddString(str) < 0)
      {
        MIKTEX_UNEXPECTED();
      }
    }

    if (commandLineIdx >= 0)
    {
      if (editorListBox.SetCurSel(commandLineIdx) < 0)
      {
        MIKTEX_UNEXPECTED();
      }
      commandLineDisplay.SetWindowText(UT_(editors[commandLineIdx].GetCommandLine().c_str()));
    }

    EnableButtons();
  }

  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  catch (const exception & e)
  {
    ErrorDialog::DoModal(0, e);
  }

  return ret;
}

void InverseSearchOptionsPage::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_LBIndex(pDX, IDC_LISTBOX_EDITOR, commandLineIdx);
  DDX_Control(pDX, IDC_LISTBOX_EDITOR, editorListBox);
  DDX_Control(pDX, IDC_COMMAND_LINE, commandLineDisplay);
  DDX_Control(pDX, IDC_DELETE, deleteButton);
  DDX_Control(pDX, IDC_CHANGE, changeButton);
}

void InverseSearchOptionsPage::OnSelChangeEditor()
{
  try
  {
    int idx = editorListBox.GetCurSel();
    if (idx >= static_cast<int>(editors.size()))
    {
      MIKTEX_UNEXPECTED();
    }
    commandLineIdx = idx;
    if (idx >= 0)
    {
      commandLineDisplay.SetWindowText(UT_(editors[idx].GetCommandLine().c_str()));
    }
    else
    {
      commandLineDisplay.SetWindowText(_T(""));
    }
    SetModified(TRUE);
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void InverseSearchOptionsPage::OnAddEditor()
{
  try
  {
    AddChangeEditorDialog dlg(this, editors, firstCustomIdx);
    if (dlg.DoModal() == IDOK)
    {
      EditorInfo editorInfo = dlg.GetEditorInfo();
      editors.push_back(editorInfo);
      if (editorListBox.AddString(UT_(editorInfo.name.c_str())) < 0)
      {
        MIKTEX_UNEXPECTED();
      }
      mustWrite = true;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void InverseSearchOptionsPage::OnChangeEditor()
{
  try
  {
    AddChangeEditorDialog dlg(this, editors, firstCustomIdx, commandLineIdx);
    if (dlg.DoModal() == IDOK)
    {
      editors[commandLineIdx] = dlg.GetEditorInfo();
      OnSelChangeEditor();
      mustWrite = true;
    }
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

void InverseSearchOptionsPage::OnDeleteEditor()
{
  try
  {
    if (editorListBox.DeleteString(commandLineIdx) < 0)
    {
      MIKTEX_UNEXPECTED();
    }
    editors.erase(editors.begin() + commandLineIdx);
    commandLineIdx = -1;
    OnSelChangeEditor();
    mustWrite = true;
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
}

BOOL InverseSearchOptionsPage::OnApply()
{
  try
  {
    PathName pathEditorsIni(session->GetSpecialPath(SpecialPath::ConfigRoot), MIKTEX_PATH_EDITORS_INI);
    if (mustWrite)
    {
      unique_ptr<Cfg> pCfg(Cfg::Create());
      for (int idx = firstCustomIdx; idx < static_cast<int>(editors.size()); ++idx)
      {
        pCfg->PutValue(editors[idx].name, "program", editors[idx].program);
        pCfg->PutValue(editors[idx].name, "arguments", editors[idx].arguments);
      }
      Directory::Create(PathName(pathEditorsIni).RemoveFileSpec());
      pCfg->Write(pathEditorsIni);
      mustWrite = false;
      pCfg = nullptr;
      if (!Fndb::FileExists(pathEditorsIni))
      {
        Fndb::Add(pathEditorsIni);
      }
    }
    if (commandLineIdx >= 0)
    {
      g_pYapConfig->inverseSearchCommandLine = editors[commandLineIdx].GetCommandLine();
    }
    else
    {
      g_pYapConfig->inverseSearchCommandLine.clear();
    }
    return CPropertyPage::OnApply();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
    return FALSE;
  }
}

void InverseSearchOptionsPage::EnableButtons()
{
  deleteButton.EnableWindow(commandLineIdx >= 0 && commandLineIdx >= firstCustomIdx);
  changeButton.EnableWindow(commandLineIdx >= 0 && commandLineIdx >= firstCustomIdx);
}

void InverseSearchOptionsPage::CollectEditors()
{
  string program;
  string arguments;

  /// free editors

  if (MakeXEmacsCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("XEmacs"), program, arguments));
  }

  if (MakeNTEmacsCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("NTEmacs"), program, arguments));
  }

  if (MakeNTEmacsClientCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("NTEmacs (Single Instance)"), program, arguments));
  }

  if (MakeTeXnicCenterCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("TeXnicCenter"), program, arguments));
  }

  if (MakeVisualTeXCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("Visual TeX"), program, arguments));
  }

  /// non-free editors

  if (MakeWinEdtCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("WinEdt"), program, arguments));
  }

  if (MakeLaTeXMngCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("LaTeXMng"), program, arguments));
  }

  if (MakeWinTeXXPCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("WinTeX XP"), program, arguments));
  }

  if (MakeWinShellCommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("WinShell"), program, arguments));
  }

  if (MakeLaTeXWIDECommandLine(program, arguments))
  {
    editors.push_back(EditorInfo(T_("LaTeX WIDE"), program, arguments));
  }

  /// user-defined

  firstCustomIdx = static_cast<int>(editors.size());

  for (unsigned r = session->GetNumberOfTEXMFRoots(); r > 0; --r)
  {
    PathName editorsIni = session->GetRootDirectory(r - 1);
    editorsIni /= MIKTEX_PATH_EDITORS_INI;
    if (File::Exists(editorsIni))
    {
      unique_ptr<Cfg> pCfg(Cfg::Create());
      pCfg->Read(editorsIni);
      for (const shared_ptr<Cfg::Key> & key : pCfg->GetKeys())
      {
        editors.push_back(EditorInfo(key->GetName(), pCfg->GetValue(key->GetName(), "program"), pCfg->GetValue(key->GetName(), "arguments")));
      }
    }
  }
}
