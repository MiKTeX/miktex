/* WelcomePage.h:                                       -*- C++ -*-

   Copyright (C) 2000-2018 Christian Schenk

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

class WelcomePage :
  public CPropertyPage
{
private:
  enum { IDD = IDD_WELCOME };

protected:
  DECLARE_MESSAGE_MAP();

public:
  WelcomePage();

protected:
  virtual BOOL OnInitDialog();

protected:
  virtual BOOL OnSetActive();

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  virtual BOOL OnKillActive();

private:
  void ShowItems();

private:
  CListBox listBox;

private:
  class RemoveWizard * pSheet = nullptr;

private:
  CString oldNextText;

private:
  shared_ptr<Session> session = Session::Get();
};
