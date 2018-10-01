/* MacOsVersion.cpp: 

   Copyright (C) 2018 Christian Schenk

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

#include "miktex/Core/PathName.h"
#include "miktex/Core/Process.h"
#include "miktex/Core/Utils.h"
#include "miktex/Core/mac/MacOsVersion.h"

using namespace MiKTeX::Core;
using namespace std;

void rtrim(std::string& str)
{
}

void ltrim(std::string& str) {
  size_t startpos = str.find_first_not_of(" \t\r\n");
  if(std::string::npos != startpos )
    str = str.substr(startpos);
}

MacOsVersion RunSwVers()
{
  PathName sw_vers;
  if (!Utils::FindProgram("sw_vers", sw_vers))
  {
    MIKTEX_FATAL_ERROR("sw_vers not found");
  }
  MacOsVersion result;
  vector<string> args{ "sw_vers", "-productVersion" };
  ProcessOutput<80> swversOutput;
  int exitCode;
  if (Process::Run(sw_vers, args, &swversOutput, &exitCode, nullptr) && exitCode == 0)
  {
    string str = swversOutput.StdoutToString();
    size_t endpos = str.find_last_not_of(" \t\r\n");
    if(endpos != string::npos)
    {
      str = str.substr(0, endpos + 1);
    }
    result.productVersion = str;
  }
  return result;
}

class LazyMacOsVersion
{
private:
  MacOsVersion macOsVersion;
public:
  operator MacOsVersion()
  {
    if (macOsVersion.productVersion.empty())
    {
      macOsVersion = RunSwVers();
    }
    return macOsVersion;
  }
};

MacOsVersion MacOsVersion::Get()
{
  static LazyMacOsVersion macOsVersion;
  return macOsVersion;
}
