/* WindowsVersion.cpp: get Windows version information

   Copyright (C) 1996-2018 Christian Schenk

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "internal.h"

#include "miktex/Core/win/WindowsVersion.h"

using namespace std;

using namespace MiKTeX::Core;

class LazyWindowsVersion
{
private:
  DWORD windowsVersion = 0;
public:
  operator DWORD()
  {
    if (windowsVersion == 0)
    {
      windowsVersion = ::GetVersion();
    }
    return windowsVersion;
  }
};

LazyWindowsVersion windowsVersion;

string WindowsVersion::GetMajorMinorString()
{
  unsigned major = LOBYTE(LOWORD(windowsVersion));
  unsigned minor = HIBYTE(LOWORD(windowsVersion));
  return std::to_string(major) + "." + std::to_string(minor);
}

bool WindowsVersion::IsWindows8OrGreater()
{
  unsigned major = LOBYTE(LOWORD(windowsVersion));
  unsigned minor = HIBYTE(LOWORD(windowsVersion));
  return major > 6 || (major == 6 && minor >= 2);
}

bool WindowsVersion::IsWindows7OrGreater()
{
  unsigned major = LOBYTE(LOWORD(windowsVersion));
  unsigned minor = HIBYTE(LOWORD(windowsVersion));
  return major > 6 || (major == 6 && minor >= 1);
}
