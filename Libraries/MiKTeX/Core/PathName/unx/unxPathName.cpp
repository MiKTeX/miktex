/* unxPathName.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/PathName.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

PathName & PathName::SetToCurrentDirectory()
{
  if (getcwd(GetData(), GetCapacity()) == 0)
  {
    MIKTEX_FATAL_CRT_ERROR("getcwd");
  }
  return *this;
}

PathName & PathName::SetToTempDirectory()
{
  if (!Utils::GetEnvironmentString("TMPDIR", GetData(), GetCapacity()))
  {
#if defined(P_tmpdir)
    StringUtil::CopyString(GetData(), GetCapacity(), P_tmpdir);
#else
    StringUtil::CopyString(GetData(), GetCapacity(), "/tmp");
#endif
  }
  return *this;
}

PathName & PathName::SetToTempFile()
{
  *this = SessionImpl::GetSession()->GetTempDirectory();
  AppendComponent("mikXXXXXX");
  int fd = mkstemp(GetData());
  if (fd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("mkstemp");
  }
  close(fd);
  SessionImpl::GetSession()->trace_tempfile->WriteFormattedLine("core", T_("created temporary file %s"), Q_(Get()));
  return *this;
}
