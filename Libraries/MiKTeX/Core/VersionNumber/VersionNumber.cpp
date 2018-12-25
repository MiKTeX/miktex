/* VersionNumber.cpp: version number operations

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

#include "config.h"

#include <miktex/Core/VersionNumber>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;

string VersionNumber::ToString() const
{
  string str = std::to_string(n1);
  str += '.';
  str += std::to_string(n2);
  if (n3 > 0 || n4 > 0)
  {
    str += '.';
    str += std::to_string(n3);
    if (n4 > 0)
    {
      str += '.';
      str += std::to_string(n4);
    }
  }
  return str;
}

bool VersionNumber::TryParse(const string& versionString, VersionNumber& versionNumber)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  define SScanF sscanf_s
#else
#  define SScanF sscanf
#endif
  if (SScanF(versionString.c_str(), "%u.%u.%u.%u", &versionNumber.n1, &versionNumber.n2, &versionNumber.n3, &versionNumber.n4) == 4)
  {
    return true;
  }
  else if (SScanF(versionString.c_str(), "%u.%u.%u", &versionNumber.n1, &versionNumber.n2, &versionNumber.n3) == 3)
  {
    versionNumber.n4 = 0;
    return true;
  }
  else if (SScanF(versionString.c_str(), "%u.%u", &versionNumber.n1, &versionNumber.n2) == 2)
  {
    versionNumber.n3 = 0;
    versionNumber.n4 = 0;
    return true;
  }
  else
  {
    return false;
  }
#undef SScanF
}

VersionNumber VersionNumber::Parse(const string& versionString)
{
  VersionNumber versionNumber;
  if (!TryParse(versionString, versionNumber))
  {
    MIKTEX_UNEXPECTED();
  }
  return versionNumber;
}
