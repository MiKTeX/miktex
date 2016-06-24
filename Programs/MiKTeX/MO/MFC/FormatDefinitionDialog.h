/* FormatDefinitionDialog.h:                            -*- C++ -*-

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

#if defined(_MSC_VER)
#  pragma once
#endif

class FormatDefinitionDialog :
  public CDialog
{
public:
  FormatDefinitionDialog(CWnd * pParent = nullptr)
  {
    UNUSED_ALWAYS(pParent);
  }

public:
  FormatDefinitionDialog(CWnd * pParent, const char * lpszKey);

private:
  enum { IDD = IDD_FMT_DEFINITION };

private:
  CComboBox compilerComboBox;

private:
  CComboBox preloadedFormatComboBox;

private:
  CString formatKey;

private:
  CString formatName;

private:
  CString description;

private:
  BOOL exclude;

private:
  CString inputName;

private:
  CString outputName;

private:
  CString preloadedFormat;

private:
  CString compiler;

public:
  FormatInfo GetFormatInfo() const
  {
    return formatInfo;
  }

protected:
  virtual void DoDataExchange(CDataExchange * pDX);

protected:
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

protected:
  afx_msg void OnContextMenu(CWnd * pWnd, CPoint point);

protected:
  DECLARE_MESSAGE_MAP();

protected:
  virtual BOOL OnInitDialog();

private:
  FormatInfo formatInfo;

private:
  FormatInfo originalFormatInfo;

private:
  std::shared_ptr<MiKTeX::Core::Session> session = MiKTeX::Core::Session::Get();
};
