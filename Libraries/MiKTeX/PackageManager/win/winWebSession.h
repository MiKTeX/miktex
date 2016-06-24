/* winWebSession.h:                                     -*- C++ -*-

   Copyright (C) 2001-2016 Christian Schenk

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

#include "WebSession.h"

BEGIN_INTERNAL_NAMESPACE;

class winWebSession : public WebSession
{
public:
  virtual ~winWebSession();

public:
  virtual WebFile * OpenUrl(const char * lpszUrl);

public:
  virtual void Dispose();

public:
  winWebSession();

public:
  HINTERNET GetHandle() const
  {
    return hInternet;
  }

public:
  static void GetLastErrorMessage(std::string &  message);

public:
  static bool IsGlobalOffline();

private:
  HINTERNET hInternet = nullptr;
};

END_INTERNAL_NAMESPACE;
