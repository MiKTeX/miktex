/* mo.h:                                                -*- C++ -*-

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

#include "resource.h"

class MiKTeXOptionsApplication :
  public CWinApp
{
public:
  MiKTeXOptionsApplication();

public:
  virtual BOOL InitInstance();

public:
  virtual void WinHelp(DWORD data, UINT cmd = HELP_CONTEXT);

public:
  DECLARE_MESSAGE_MAP();
};

BOOL OnHelpInfo(HELPINFO * pHelpInfo, const DWORD * pHelpIDs, const char * lpszTopicFile);

void DoWhatsThisMenu(CWnd * pWnd, CPoint point, const DWORD * pHelpIDs, const char * lpszTopicFile);

#define T_(x) MIKTEXTEXT(x)

#if !(defined(UNICODE) && defined(_UNICODE))
#  error UNICODE compilation required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#if defined(MIKTEX_WINDOWS)
#  define WU_(x) MiKTeX::Util::CharBuffer<char>(x).Get()
#  define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).Get()
#endif
