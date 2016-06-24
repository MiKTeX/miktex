/* ConnectionSettingsDialog.h:                          -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

   This file is part of the MiKTeX UI Library.

   The MiKTeX UI Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX UI Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX UI Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "resource.h"

class ConnectionSettingsDialog : public CDialog
{
private:
  enum { IDD = IDD_CONNECTION_SETTINGS };

private:
  DECLARE_DYNCREATE(ConnectionSettingsDialog);

protected:
  DECLARE_MESSAGE_MAP();

public:
  ConnectionSettingsDialog(CWnd * pParent = nullptr);

protected:
  BOOL OnInitDialog() override;

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  void OnOK() override;

protected:
  afx_msg void OnUseProxy();

protected:
  afx_msg void OnChangeHost();

protected:
  afx_msg void OnChangePort();

private:
  void EnableButtons();

private:
  CWnd * GetControl(int nID);

private:
  CButton useProxyButton;

private:
  CButton proxyAuthenticationRequiredButton;

private:
  int proxyPort;

private:
  BOOL useProxy;

private:
  CString proxyHost;

private:
  BOOL proxyAuthenticationRequired;
};
