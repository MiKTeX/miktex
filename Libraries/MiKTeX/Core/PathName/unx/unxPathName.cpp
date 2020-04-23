/* unxPathName.cpp:

   Copyright (C) 1996-2020 Christian Schenk

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

#include "config.h"

#include <unistd.h>

#include <miktex/Core/PathName>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

PathName& PathName::SetToCurrentDirectory()
{
  while (getcwd(GetData(), GetCapacity()) == nullptr)
  {
    if (errno == ERANGE)
    {
      Reserve(GetCapacity() * 2);
    }
    else
    {
      MIKTEX_FATAL_CRT_ERROR("getcwd");
    }
  }
  return *this;
}

PathName& PathName::SetToTempDirectory()
{
  string tmpdir;
  if (Utils::GetEnvironmentString("TMPDIR", tmpdir))
  {
    *this = tmpdir;
  }
  else
  {
#if defined(P_tmpdir)
    *this = P_tmpdir;
#else
    *this = "/tmp";
#endif
  }
  return *this;
}

PathName& PathName::SetToTempFile()
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    *this = SessionImpl::GetSession()->GetTempDirectory();
  }
  else
  {
    SetToTempDirectory();
  }    
  AppendComponent("mikXXXXXX");
  int fd = mkstemp(GetData());
  if (fd < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("mkstemp");
  }
  close(fd);
  if (session != nullptr)
  {
    session->trace_tempfile->WriteFormattedLine("core", T_("created temporary file %s"), Q_(GetData()));
  }
  return *this;
}

PathName PathName::GetMountPoint() const
{
  UNIMPLEMENTED();
}
