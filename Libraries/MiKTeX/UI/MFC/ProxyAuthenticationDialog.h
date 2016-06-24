/* ProxyAuthenticationDialog.h:                         -*- C++ -*-

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

#include "resource.h"

class ProxyAuthenticationDialog : public CDialog
{
private:
  enum { IDD = IDD_PROXY_AUTHENTICATION };

private:
  DECLARE_DYNAMIC(ProxyAuthenticationDialog)

protected:
  DECLARE_MESSAGE_MAP();

protected:
  void DoDataExchange(CDataExchange * pDX) override;

protected:
  BOOL OnInitDialog() override;

protected:
  afx_msg void OnChangeName();

public:
  ProxyAuthenticationDialog(CWnd * pParent = nullptr);

public:
  std::string GetName() const
  {
    return std::string(MiKTeX::Util::CharBuffer<char>(static_cast<LPCTSTR>(name)).Get());
  }

public:
  std::string GetPassword() const
  {
    return std::string(MiKTeX::Util::CharBuffer<char>(static_cast<LPCTSTR>(password)).Get());
  }

private:
  CString name;

private:
  CString password;
};
