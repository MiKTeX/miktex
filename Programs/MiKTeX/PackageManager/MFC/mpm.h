/* mpm.h:                                               -*- C++ -*-

   Copyright (C) 2002-2016 Christian Schenk

   This file is part of MiKTeX Package Manager.

   MiKTeX Package Manager is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   MiKTeX Package Manager is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Package Manager; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "resource.h"

#define T_(x) MIKTEXTEXT(x)

#if !defined(UNICODE)
#  error UNICODE compilation required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

class PackageManagerApplication :
  public CWinApp
{
protected:
  DECLARE_MESSAGE_MAP();

public:
  PackageManagerApplication();

public:
  virtual BOOL InitInstance();

public:
  afx_msg void OnAppAbout();

public:
  virtual int ExitInstance();

private:
  shared_ptr<Session> pSession;
};

extern PackageManagerApplication theApp;
