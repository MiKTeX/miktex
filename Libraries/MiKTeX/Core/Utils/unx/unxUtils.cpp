/* unxUtil.cpp: 

   Copyright (C) 1996-2021 Christian Schenk

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

#if defined(__APPLE__)
#  include <mach-o/dyld.h>
#endif

#if defined(HAVE_SYS_UTSNAME_H)
#  include <sys/utsname.h>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Core/File>
#include <miktex/Util/PathName>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

string Utils::GetOSVersionString()
{
  string version;
#if defined(HAVE_UNAME_SYSCALL)
  struct utsname buf;
  if (uname(&buf) < 0)
  {
    MIKTEX_FATAL_CRT_ERROR("uname");
  }
  version = buf.sysname;
  version += ' ';
  version += buf.release;
  version += ' ';
  version += buf.version;
  version += ' ';
  version += buf.machine;
#else
#warning Unimplemented : Utils::GetOSVersionString
  version = "UnkOS 0.1";
#endif
  return version;
}

void Utils::SetEnvironmentString(const string& valueName, const string& value)
{
  string oldValue;
  if (::GetEnvironmentString(valueName, oldValue) && oldValue == value)
  {
    return;
  }
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_config->WriteLine("core", fmt::format(T_("setting env {0}={1}"), valueName, value));
  }
  if (setenv(valueName.c_str(), value.c_str(), 1) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("setenv", "name", valueName);
  }
}

void Utils::RemoveEnvironmentString(const string& valueName)
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    session->trace_config->WriteLine("core", fmt::format(T_("unsetting env {0}"), valueName));
  }
  if (unsetenv(valueName.c_str()) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("unsetenv", "name", valueName);
  }
}

void Utils::CheckHeap()
{
}

void Utils::ShowWebPage(const string& url)
{
  UNIMPLEMENTED();
}

bool Utils::SupportsHardLinks(const PathName& path)
{
  return true;
}

bool Utils::CheckPath(bool repair)
{
#if 1
  // TODO
  if (repair)
  {
    UNIMPLEMENTED();
  }
#endif
  
  shared_ptr<Session> session = Session::Get();

  string envPath;
  if (!Utils::GetEnvironmentString("PATH", envPath))
  {
    return false;
  }
  
  PathName linkTargetDirectory = session->GetSpecialPath(SpecialPath::LinkTargetDirectory);

  string repairedPath;
  bool pathCompetition;
  
  bool pathOkay = !Directory::Exists(linkTargetDirectory) || !FixProgramSearchPath(envPath, linkTargetDirectory, true, repairedPath, pathCompetition);

  bool repaired = false;

  if (!pathOkay && !repair)
  {
    SessionImpl::GetSession()->trace_error->WriteLine("core", T_("Something is wrong with the PATH:"));
    SessionImpl::GetSession()->trace_error->WriteLine("core", envPath.c_str());
  }
  else if (!pathOkay && repair)
  {
    SessionImpl::GetSession()->trace_error->WriteLine("core", T_("Setting new PATH:"));
    SessionImpl::GetSession()->trace_error->WriteLine("core", repairedPath.c_str());
    envPath = repairedPath;
    if (session->IsAdminMode())
    {
      // TODO: edit system configuration
    }
    else
    {
      // TODO: edit user configuration
    }
    pathOkay = true;
    repaired = true;
  }
  return repaired || pathOkay;
}

string Utils::GetExeName()
{
#if defined(__APPLE__)
    CharBuffer<char> buf;
    uint32_t bufSize = buf.GetCapacity();
    if (_NSGetExecutablePath(buf.GetData(), &bufSize) < 0)
    {
        buf.Reserve(bufSize);
        if (_NSGetExecutablePath(buf.GetData(), &bufSize) != 0)
        {
            MIKTEX_UNEXPECTED();
        }
    }
    return buf.GetData();
#else
    return File::ReadSymbolicLink("/proc/self/exe");
#endif
}
