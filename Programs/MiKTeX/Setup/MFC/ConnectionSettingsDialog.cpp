/* ConnectionSettingsDialog.cpp:

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

#include "StdAfx.h"

#include "ConnectionSettingsDialog.h"

using namespace std;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

IMPLEMENT_DYNCREATE(ConnectionSettingsDialog, CDialog);

BEGIN_MESSAGE_MAP(ConnectionSettingsDialog, CDialog)
  ON_BN_CLICKED(IDC_USE_PROXY, &ConnectionSettingsDialog::OnUseProxy)
  ON_EN_CHANGE(IDC_PROXY_HOST, OnChangeHost)
  ON_EN_CHANGE(IDC_PROXY_PORT, OnChangePort)
END_MESSAGE_MAP();

ConnectionSettingsDialog::ConnectionSettingsDialog(CWnd * pParent) :
  CDialog(ConnectionSettingsDialog::IDD, pParent)
{
  ProxySettings proxySettings;
  if (PackageManager::TryGetProxy(proxySettings))
  {
    proxyPort = proxySettings.port;
    useProxy = (proxySettings.useProxy ? TRUE : FALSE);
    proxyAuthenticationRequired = proxySettings.authenticationRequired ? TRUE : FALSE;
    proxyHost = proxySettings.proxy.c_str();
  }
  else
  {
    proxyPort = 8080;
    useProxy = FALSE;
    proxyAuthenticationRequired = FALSE;
  }
}

BOOL ConnectionSettingsDialog::OnInitDialog()
{
  BOOL ret = CDialog::OnInitDialog();
  try
  {
    OnUseProxy();
  }
  catch (const MiKTeXException & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
  return ret;
}

void ConnectionSettingsDialog::DoDataExchange(CDataExchange * pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_USE_PROXY, useProxyButton);
  DDX_Control(pDX, IDC_AUTH_REQUIRED, proxyAuthenticationRequiredButton);
  DDX_Text(pDX, IDC_PROXY_PORT, proxyPort);
  //DDV_MinMaxInt(pDX, proxyPort, 1, 65535);
  DDX_Check(pDX, IDC_USE_PROXY, useProxy);
  DDX_Text(pDX, IDC_PROXY_HOST, proxyHost);
  DDX_Check(pDX, IDC_AUTH_REQUIRED, proxyAuthenticationRequired);
}

void ConnectionSettingsDialog::OnUseProxy()
{
  try
  {
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
}

void ConnectionSettingsDialog::OnChangeHost()
{
  try
  {
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
}

void ConnectionSettingsDialog::OnChangePort()
{
  try
  {
    EnableButtons();
  }
  catch (const MiKTeXException & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
}

void ConnectionSettingsDialog::OnOK()
{
  try
  {
    CDialog::OnOK();
    ProxySettings proxySettings;
    proxySettings.useProxy = (useProxy ? true : false);
    proxySettings.authenticationRequired = proxyAuthenticationRequired ? true : false;
    proxySettings.proxy = TU_(static_cast<LPCTSTR>(proxyHost));
    proxySettings.port = proxyPort;
    PackageManager::SetProxy(proxySettings);
  }
  catch (const MiKTeXException & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
  catch (const exception & e)
  {
    AfxMessageBox(UT_(e.what()), MB_OK | MB_ICONSTOP);
  }
}

void ConnectionSettingsDialog::EnableButtons()
{
  BOOL useProxy = useProxyButton.GetCheck() == BST_CHECKED;
  GetControl(IDC_STATIC_ADDRESS)->EnableWindow(useProxy);
  GetControl(IDC_PROXY_HOST)->EnableWindow(useProxy);
  GetControl(IDC_STATIC_PORT)->EnableWindow(useProxy);
  GetControl(IDC_PROXY_PORT)->EnableWindow(useProxy);
  GetControl(IDC_AUTH_REQUIRED)->EnableWindow(useProxy);
  CString host;
  CString port;
  GetControl(IDC_PROXY_HOST)->GetWindowText(host);
  GetControl(IDC_PROXY_PORT)->GetWindowText(port);
  GetControl(IDOK)->EnableWindow(!useProxy || !(host.IsEmpty() || port.IsEmpty()));
}

CWnd * ConnectionSettingsDialog::GetControl(int nID)
{
  CWnd * pWnd = GetDlgItem(nID);
  if (nID == 0)
  {
    return nullptr; // FIXME
  }
  return pWnd;
}
