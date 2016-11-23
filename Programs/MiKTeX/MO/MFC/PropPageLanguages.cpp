/* PropPageLanguages.cpp:

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

#include "PropPageLanguages.h"
#include "PropSheet.h"
#include "resource.hm"

#if !defined(INSTALL_LANGUAGE_PACKAGES)
#  define INSTALL_LANGUAGE_PACKAGES 1
#endif

#if !defined(SHOW_LANGUAGE_PACKAGES)
#  define SHOW_LANGUAGE_PACKAGES 0
#endif

PropPageLanguages::PropPageLanguages(std::shared_ptr<MiKTeX::Packages::PackageManager> pManager) :
  CPropertyPage(PropPageLanguages::IDD),
  pManager(pManager)
{
  m_psp.dwFlags &= ~(PSP_HASHELP);
}

PropPageLanguages::~PropPageLanguages()
{
}

void PropPageLanguages::DoDataExchange(CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST, listControl);
}

BEGIN_MESSAGE_MAP(PropPageLanguages, CPropertyPage)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemChanged)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST, OnItemChanging)
  ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
  ON_WM_CONTEXTMENU()
  ON_WM_HELPINFO()
END_MESSAGE_MAP()

void PropPageLanguages::InsertColumn(int colIdx, const char * lpszLabel, const char * lpszLongest)
{
  if (listControl.InsertColumn(colIdx, UT_(lpszLabel), LVCFMT_LEFT, listControl.GetStringWidth(UT_(lpszLongest)), colIdx) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertColumn");
  }
}

BOOL PropPageLanguages::OnInitDialog()
{
  BOOL ret = CPropertyPage::OnInitDialog();
  try
  {
    listControl.SetExtendedStyle(listControl.GetExtendedStyle() | LVS_EX_CHECKBOXES);
    int colIdx = 0;
    InsertColumn(colIdx++, T_("Language"), "xxxx german-x-2009-06-19");
    InsertColumn(colIdx++, T_("Synonyms"), "xxxx usenglish,USenglish,american");
#if SHOW_LANGUAGE_PACKAGES
    InsertColumn(colIdx++, T_("Package"), "xxxx miktex-misc");
#endif
    ReadLanguageDat();
    Refresh();
  }
  catch (const MiKTeXException & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  catch (const exception & e)
  {
    ErrorDialog::DoModal(this, e);
  }
  return ret;
}

vector<string> PropPageLanguages::WhichPackage(const char * lpszTeXInputFile)
{
  PathName directoryPattern(session->GetMpmRootPath());
  directoryPattern /= "tex//";
  vector<PathName> paths;
  vector<string> result;
  if (!Fndb::Search(lpszTeXInputFile, directoryPattern.GetData(), false, paths, result))
  {
    result.clear();
  }
  return result;
}

bool PropPageLanguages::InstallLanguagePackages(const vector<string> toBeInstalled)
{
#if INSTALL_LANGUAGE_PACKAGES
  CString str1;
  str1.Format(_T("%u"), toBeInstalled.size());
  CString str;
  AfxFormatString2(str, IDP_UPDATE_MESSAGE, str1, _T("0"));
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
  if (WindowsVersion::IsWindowsVistaOrGreater() && session->IsAdminMode())
  {
    DllProc4<HRESULT, const TASKDIALOGCONFIG *, int *, int *, BOOL *> taskDialogIndirect("comctl32.dll", "TaskDialogIndirect");
    TASKDIALOGCONFIG taskDialogConfig;
    memset(&taskDialogConfig, 0, sizeof(taskDialogConfig));
    taskDialogConfig.cbSize = sizeof(TASKDIALOGCONFIG);
    taskDialogConfig.hwndParent = nullptr;
    taskDialogConfig.hInstance = nullptr;
    taskDialogConfig.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
    taskDialogConfig.pszMainIcon = MAKEINTRESOURCEW(TD_SHIELD_ICON);
    taskDialogConfig.pszWindowTitle = MAKEINTRESOURCEW(AFX_IDS_APP_TITLE);
    taskDialogConfig.pszMainInstruction = T_(L"Do you want to proceed?");
    CStringW strContent(str);
    taskDialogConfig.pszContent = strContent;
    taskDialogConfig.cButtons = 2;
    TASKDIALOG_BUTTON const buttons[] = {
      {IDOK, T_(L"Proceed")},
      {IDCANCEL, T_(L"Cancel")}
    };
    taskDialogConfig.pButtons = buttons;
    taskDialogConfig.nDefaultButton = IDOK;
    int result = 0;
    if (SUCCEEDED(taskDialogIndirect(&taskDialogConfig, &result, nullptr, nullptr)))
    {
      if (IDOK != result)
      {
        return false;
      }
    }
    else
    {
      MIKTEX_UNEXPECTED();
    }
  }
  else
#endif
  {
    if (AfxMessageBox(str, MB_OKCANCEL | MB_ICONINFORMATION) == IDCANCEL)
    {
      return false;
    }
  }
  vector<string> toBeRemoved;
  bool result = (UpdateDialog::DoModal(this, pManager, toBeInstalled, toBeRemoved) == IDOK);
  return result;
#else
  return false;
#endif
}

BOOL PropPageLanguages::OnApply()
{
  try
  {
#if INSTALL_LANGUAGE_PACKAGES
    vector<string> toBeInstalled;
#endif
    for (vector<MyLanguageInfo>::iterator it = languages.begin(); it != languages.end(); ++it)
    {
      if (it->active == it->newActive)
      {
        continue;
      }
      LanguageInfo languageInfo = *it;
      languageInfo.exclude = !it->newActive;
      session->SetLanguageInfo(languageInfo);
#if INSTALL_LANGUAGE_PACKAGES
      if (it->packageNames.size() > 0 && !it->active)
      {
        PackageInfo packageInfo = pManager->GetPackageInfo(it->packageNames[0]);
        if (!packageInfo.IsInstalled() && find(toBeInstalled.begin(), toBeInstalled.end(), it->packageNames[0]) == toBeInstalled.end())
        {
          toBeInstalled.push_back(it->packageNames[0]);
        }
      }
#endif
    }
#if INSTALL_LANGUAGE_PACKAGES
    if (toBeInstalled.size() > 0)
    {
      InstallLanguagePackages(toBeInstalled);
    }
#endif
    ReadLanguageDat();
    Refresh();
    SetElevationRequired(false);
    return TRUE;
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

#define MAKE_ID_HID_PAIR(id) id, H##id

namespace
{
  const DWORD aHelpIDs[] = {
    MAKE_ID_HID_PAIR(IDC_LIST),
    0, 0, };
}

BOOL PropPageLanguages::OnHelpInfo(HELPINFO * pHelpInfo)
{
  return ::OnHelpInfo(pHelpInfo, aHelpIDs, "LanguagesPage.txt");
}

void PropPageLanguages::OnContextMenu(CWnd * pWnd, CPoint point)
{
  try
  {
    DoWhatsThisMenu(pWnd, point, aHelpIDs, "LanguagesPage.txt");
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

void PropPageLanguages::ReadLanguageDat()
{
  languages.clear();
  for (const LanguageInfo & languageInfo : session->GetLanguages())
  {
    MyLanguageInfo myLanguageInfo(languageInfo);
    myLanguageInfo.loaderExists = session->FindFile(languageInfo.loader.c_str(), ".;%r/tex//", myLanguageInfo.loaderPath);
    myLanguageInfo.active = !myLanguageInfo.exclude && myLanguageInfo.loaderExists;
    myLanguageInfo.newActive = myLanguageInfo.active;
    myLanguageInfo.packageNames = WhichPackage(myLanguageInfo.loader.c_str());
    languages.push_back(myLanguageInfo);
  }
}

void PropPageLanguages::InsertLanguage(int idx)
{
  LV_ITEM lvitem;
  lvitem.iItem = idx;
  lvitem.mask = LVIF_PARAM;
  lvitem.lParam = idx;
  lvitem.iSubItem = 0;
  inserting = true;
  if (listControl.InsertItem(&lvitem) < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::InsertItem");
  }
  RefreshRow(idx);
  inserting = false;
}

void PropPageLanguages::Refresh()
{
  if (listControl.DeleteAllItems() < 0)
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::DeleteAllItems");
  }
  int idx = 0;
  for (vector<MyLanguageInfo>::const_iterator it = languages.begin(); it != languages.end(); ++it, ++idx)
  {
    InsertLanguage(idx);
  }
  EnableButtons();
}

void PropPageLanguages::OnDblclkList(NMHDR * pNMHDR, LRESULT * pResult)
{
  UNUSED_ALWAYS(pNMHDR);
  *pResult = 0;
}

int PropPageLanguages::GetSelectedItem()
{
  POSITION pos = listControl.GetFirstSelectedItemPosition();
  if (pos == nullptr)
  {
    MIKTEX_UNEXPECTED();
  }
  return listControl.GetNextSelectedItem(pos);
}

inline bool IsChecked(UINT state)
{
  return ((state & LVIS_STATEIMAGEMASK) >> 12) == 2 ? true : false;
}

void PropPageLanguages::OnItemChanging(NMHDR * pNMHDR, LRESULT * pResult)
{
  *pResult = 0;
  if (inserting)
  {
    return;
  }
  try
  {
    LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    if (pnmv->iItem >= 0 && (pnmv->uChanged & LVIF_STATE) != 0)
    {
      if (!IsChecked(pnmv->uOldState) && IsChecked(pnmv->uNewState))
      {
        if (!languages[pnmv->iItem].loaderExists && languages[pnmv->iItem].packageNames.size() == 0)
        {
          AfxMessageBox(T_(_T("This language is not yet available.")), MB_OK | MB_ICONEXCLAMATION);
          *pResult = TRUE;
        }
      }
      if (IsChecked(pnmv->uOldState) && !IsChecked(pnmv->uNewState))
      {
        if (pnmv->iItem == 0)
        {
          AfxMessageBox(T_(_T("This language cannot be excluded.")), MB_OK | MB_ICONEXCLAMATION);
          *pResult = TRUE;
        }
      }
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

void PropPageLanguages::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
  *pResult = 0;
  if (inserting)
  {
    return;
  }
  try
  {
    LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    if (pnmv->iItem >= 0 && (pnmv->uChanged & LVIF_STATE))
    {
      bool checked = listControl.GetCheck(pnmv->iItem) ? true : false;
      languages[pnmv->iItem].newActive = checked;
      if (languages[pnmv->iItem].active != languages[pnmv->iItem].newActive)
      {
        SetChanged(true);
      }
    }
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

void PropPageLanguages::RefreshRow(int idx)
{
  const MyLanguageInfo & lang = languages[idx];

  int colIdx = 0;

  if (!listControl.SetItemText(idx, colIdx++, UT_(lang.key.c_str())))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
  }

  if (!listControl.SetItemText(idx, colIdx++, UT_(lang.synonyms.c_str())))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
  }

#if SHOW_LANGUAGE_PACKAGES
  CString packageNames;
  if (lang.packageNames.size() == 0)
  {
    packageNames = UT_(T_("not available"));
  }
  else
  {
    for (int idx = 0; idx < lang.packageNames.size(); ++idx)
    {
      if (idx > 0)
      {
        packageNames += _T(", ");
      }
      packageNames += UT_(lang.packageNames[idx]);
    }
  }
  if (!listControl.SetItemText(idx, colIdx++, packageNames))
  {
    MIKTEX_FATAL_WINDOWS_ERROR("CListCtrl::SetItemText");
  }
#endif

  listControl.SetCheck(idx, lang.active ? TRUE : FALSE);
}

void PropPageLanguages::EnableButtons()
{
}

void PropPageLanguages::SetElevationRequired(bool f)
{
  if (WindowsVersion::IsWindowsVistaOrGreater() && session->IsAdminMode())
  {
    HWND hwnd = ::GetDlgItem(::GetParent(m_hWnd), IDOK);
    if (hwnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    Button_SetElevationRequiredState(hwnd, f ? TRUE : FALSE);
    hwnd = ::GetDlgItem(::GetParent(m_hWnd), ID_APPLY_NOW);
    if (hwnd == nullptr)
    {
      MIKTEX_UNEXPECTED();
    }
    Button_SetElevationRequiredState(hwnd, f ? TRUE : FALSE);
  }
}

void PropPageLanguages::SetChanged(bool f)
{
  SetElevationRequired(f);
  if (f)
  {
    PropSheet * pSheet = reinterpret_cast<PropSheet*>(GetParent());
    pSheet->ScheduleBuildFormats();
  }
  SetModified(f ? TRUE : FALSE);
}
