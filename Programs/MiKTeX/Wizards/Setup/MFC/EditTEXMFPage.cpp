/* EditTEXMFPage.cpp:

   Copyright (C) 1999-2016 Christian Schenk

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

#include "StdAfx.h"
#include "Setup.h"

#include "EditTEXMFPage.h"
#include "SetupWizard.h"

/* _________________________________________________________________________

   EditTEXMFPage Message Map
   _________________________________________________________________________ */

BEGIN_MESSAGE_MAP(EditTEXMFPage, CPropertyPage)
  ON_BN_CLICKED(IDC_ADD, OnAdd)
  ON_BN_CLICKED(IDC_DELETE, OnDelete)
  ON_BN_CLICKED(IDC_MOVEDOWN, OnDown)
  ON_BN_CLICKED(IDC_MOVEUP, OnUp)
  ON_LBN_SELCHANGE(IDC_LIST_TEXMFROOTS, OnSelChange)
END_MESSAGE_MAP();

/* _________________________________________________________________________

   EditTEXMFPage::EditTEXMFPage
   _________________________________________________________________________ */

EditTEXMFPage::EditTEXMFPage ()
  : CPropertyPage(IDD,
		  0,
		  IDS_HEADER_EDITTREES,
		  IDS_SUBHEADER_EDITTREES),
    pSheet (0)
{
}

/* _________________________________________________________________________

   EditTEXMFPage::OnInitDialog
   _________________________________________________________________________ */

BOOL
EditTEXMFPage::OnInitDialog ()
{
  pSheet = reinterpret_cast<SetupWizard *>(GetParent());
  return (CPropertyPage::OnInitDialog());
}

/* _________________________________________________________________________

   EditTEXMFPage::OnSetActive
   _________________________________________________________________________ */

BOOL
EditTEXMFPage::OnSetActive ()
{
  BOOL ret = (CPropertyPage::OnSetActive());
  if (ret)
    {
      try
	{
	  listBox.ResetContent ();
	  const string roots =
	    theApp.commonUserSetup
	    ? theApp.startupConfig.commonRoots
	    : theApp.startupConfig.userRoots;
	  if (! roots.empty())
	    {
	      for (CSVList root (roots.c_str(), ';');
		   root.GetCurrent();
		   ++ root)
		{
		  int r = listBox.AddString (root.GetCurrent());
		  if (r == LB_ERR || r == LB_ERRSPACE)
		    {
		      FATAL_WINDOWS_ERROR ("CListBox::AddString", 0);
		    }
		  ++ root;
		}
	      OnSelChange ();
	    }
	}
      catch (const MiKTeXException & e)
	{
	  pSheet->ReportError (e);
	  ret = FALSE;
	}
      catch (const exception & e)
	{
	  pSheet->ReportError (e);
	  ret = FALSE;
	}
    }
  return (ret);
}

/* _________________________________________________________________________

   EditTEXMFPage::DoDataExchange
   _________________________________________________________________________ */

void
EditTEXMFPage::DoDataExchange (/*[in]*/ CDataExchange * pDX)
{
  CPropertyPage::DoDataExchange (pDX);
  DDX_Control(pDX, IDC_ADD, addButton);
  DDX_Control(pDX, IDC_DELETE, deleteButton);
  DDX_Control(pDX, IDC_LIST_TEXMFROOTS, listBox);
  DDX_Control(pDX, IDC_MOVEDOWN, downButton);
  DDX_Control(pDX, IDC_MOVEUP, upButton);
}


/* _________________________________________________________________________

   EditTEXMFPage::OnWizardNext
   _________________________________________________________________________ */

LRESULT
EditTEXMFPage::OnWizardNext ()
{
  return (reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_INFOLIST)));
}

/* _________________________________________________________________________

   EditTEXMFPage::OnWizardBack
   _________________________________________________________________________ */

LRESULT
EditTEXMFPage::OnWizardBack ()
{
  return (reinterpret_cast<LRESULT>(MAKEINTRESOURCE(IDD_ADD_TEXMFROOTS)));
}

/* _________________________________________________________________________

   EditTEXMFPage::OnKillActive
   _________________________________________________________________________ */

BOOL
EditTEXMFPage::OnKillActive ()
{
  BOOL ret = CPropertyPage::OnKillActive();
  if (ret)
    {
      try
	{
	  string roots = "";
	  int count = listBox.GetCount();
	  if (count == LB_ERR)
	    {
	      FATAL_WINDOWS_ERROR ("CListBox::GetCount", 0);
	    }
	  for (int i = 0; i < count; ++ i)
	    {
	      CString str;
	      listBox.GetText (i, str);
	      roots += str;
	      if (i + 1 < count)
		{
		  roots += ';';
		}
	    }
	  if (theApp.commonUserSetup)
	  {
	    theApp.startupConfig.commonRoots = roots;
	    CheckAddTEXMFDirs (theApp.startupConfig.commonRoots, theApp.addTEXMFDirs);
	  }
	  else
	  {
	    theApp.startupConfig.userRoots = roots;
	    CheckAddTEXMFDirs (theApp.startupConfig.userRoots, theApp.addTEXMFDirs);
	  }
	}
      catch (const MiKTeXException & e)
	{
	  pSheet->ReportError (e);
	  ret = FALSE;
	}
      catch (const exception & e)
	{
	  pSheet->ReportError (e);
	  ret = FALSE;
	}
    }
  return (ret);
}

/* _________________________________________________________________________

   EditTEXMFPage::OnAdd
   _________________________________________________________________________ */

void
EditTEXMFPage::OnAdd ()
{
  try
    {
      BROWSEINFO browseInfo;
      ZeroMemory (&browseInfo, sizeof(browseInfo));
      browseInfo.hwndOwner = GetSafeHwnd();
      browseInfo.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
      LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
      if (pidl == 0)
	{
	  return;
	}
      char szDir[BufferSizes::MaxPath];
      BOOL havePath = SHGetPathFromIDList(pidl, szDir);
      CoTaskMemFree (pidl);
      if (! havePath)
	{
	  UNEXPECTED_CONDITION ("EditTEXMFPage::OnAdd");
	}
      if (listBox.AddString(szDir) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::AddString", 0);
	}
      OnSelChange ();
    }
  catch (const MiKTeXException & e)
    {
      pSheet->ReportError (e);
    }
  catch (const exception & e)
    {
      pSheet->ReportError (e);
    }
}

/* _________________________________________________________________________

   EditTEXMFPage::OnDelete
   _________________________________________________________________________ */

void
EditTEXMFPage::OnDelete ()
{
  try
    {
      int idx = listBox.GetCurSel();
      if (idx < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::GetCurSel", 0);
	}
      if (listBox.DeleteString(idx) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::DeleteString", 0);
	}
      OnSelChange ();
    }
  catch (const MiKTeXException & e)
    {
      pSheet->ReportError (e);
    }
  catch (const exception & e)
    {
      pSheet->ReportError (e);
    }
}

/* _________________________________________________________________________

   EditTEXMFPage::OnDown
   _________________________________________________________________________ */

void
EditTEXMFPage::OnDown ()
{
  try
    {
      int idx = listBox.GetCurSel();
      if (idx < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::GetCurSel", 0);
	}
      CString str;
      listBox.GetText (idx, str);
      if (listBox.DeleteString(idx) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::DeleteString", 0);
	}
      idx += 1;
      if (listBox.InsertString(idx, str) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::InsertString", 0);
	}
      if (listBox.SetCurSel(idx) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::SetCurSel", 0);
	}
      OnSelChange ();
    }
  catch (const MiKTeXException & e)
    {
      pSheet->ReportError (e);
    }
  catch (const exception & e)
    {
      pSheet->ReportError (e);
    }
}

/* _________________________________________________________________________

   EditTEXMFPage::OnUp
   _________________________________________________________________________ */

void
EditTEXMFPage::OnUp ()
{
  try
    {
      int idx = listBox.GetCurSel();
      if (idx < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::GetCurSel", 0);
	}
      CString str;
      listBox.GetText (idx, str);
      if (listBox.DeleteString(idx) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::DeleteString", 0);
	}
      idx -= 1;
      if (listBox.InsertString(idx, str) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::InsertString", 0);
	}
      if (listBox.SetCurSel(idx) < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::SetCurSel", 0);
	}
      OnSelChange ();
    }
  catch (const MiKTeXException & e)
    {
      pSheet->ReportError (e);
    }
  catch (const exception & e)
    {
      pSheet->ReportError (e);
    }
}

/* _________________________________________________________________________

   EditTEXMFPage::OnSelChange
   _________________________________________________________________________ */

void
EditTEXMFPage::OnSelChange ()
{
  try
    {
      int idx = listBox.GetCurSel();
      if (idx < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::GetCurSel", 0);
	}
      int count = listBox.GetCount();
      if (count < 0)
	{
	  FATAL_WINDOWS_ERROR ("CListBox::GetCount", 0);
	}
      deleteButton.EnableWindow (TRUE);
      upButton.EnableWindow (idx > 0);
      downButton.EnableWindow (idx + 1 < count);
    }
  catch (const MiKTeXException & e)
    {
      pSheet->ReportError (e);
    }
  catch (const exception & e)
    {
      pSheet->ReportError (e);
    }
}
