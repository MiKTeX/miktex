/* Remove.h:                                            -*- C++ -*-

   Copyright (C) 2000-2016 Christian Schenk

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

#include "resource.h"

class RemoveWizardApp :
  public CWinApp
{
protected:
  DECLARE_MESSAGE_MAP();

public:
  RemoveWizardApp();

protected:
  virtual BOOL InitInstance();
};

#define UW_(x) MiKTeX::Util::StringUtil::UTF8ToWideChar(x).c_str()
#define WU_(x) MiKTeX::Util::StringUtil::WideCharToUTF8(x).c_str()

#define T_(x) MIKTEXTEXT(x)

#if !defined(UNICODE)
#  error UNICODE compilation required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#define STR_BYT_SIZ(lpsz) ((StrLen(lpsz) + 1) * sizeof(*lpsz))

void ReportError(const MiKTeXException & e);

void ReportError(const exception & e);

vector<PathName>
GetRoots();
