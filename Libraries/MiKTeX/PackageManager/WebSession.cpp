/* WebSession.cpp:

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

#include "StdAfx.h"

#include "internal.h"

#if defined(MIKTEX_WINDOWS) && defined(WITH_WININET)
#  include "win/winWebSession.h"
static bool USE_WININET = false;
#endif

#if defined(HAVE_LIBCURL)
#  include "CurlWebSession.h"
#endif

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace std;

WebSession::~WebSession()
{
}

WebFile::~WebFile()
{
}

WebSession * WebSession::Create(IProgressNotify_ * pIProgressNotify)
{
#if defined (MIKTEX_WINDOWS) && defined(WITH_WININET)
  if (USE_WININET)
  {
    return new winWebSession;
  }
#endif
#if defined(HAVE_LIBCURL)
  return new CurlWebSession(pIProgressNotify);
#else
  #  warning Unimplemented : WebSession::Create()
    MIKTEX_FATAL_ERROR(T_("libCURL does not seem to be available."));
#endif
}
