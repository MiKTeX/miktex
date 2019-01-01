/* appnames.cpp: managing application names

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

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

inline void AppendTag(string& str, const string& tag)
{
  if (!str.empty())
  {
    str += PathName::PathNameDelimiter;
  }
  str += tag;
}

void SessionImpl::PushAppName(const string& name)
{
  MIKTEX_ASSERT(name.find(PathName::PathNameDelimiter) == string::npos);
  string newApplicationNames = name;
  for (const string& tag : StringUtil::Split(applicationNames, PathName::PathNameDelimiter))
  {
    // stop at the miktex application tag; this is always the last tag
    if (Utils::EqualsIgnoreCase(tag, "miktex"))
    {
      break;
    }
    if (Utils::EqualsIgnoreCase(tag, name))
    {
      continue;
    }
    AppendTag(newApplicationNames, tag);
  }
  AppendTag(newApplicationNames, "miktex");
  if (Utils::EqualsIgnoreCase(newApplicationNames, applicationNames))
  {
    return;
  }
  fileTypes.clear();
  applicationNames = newApplicationNames;
  trace_config->WriteLine("core", T_("application tags: ") + applicationNames);
}

void SessionImpl::PushBackAppName(const string& name)
{
  MIKTEX_ASSERT(name.find(PathName::PathNameDelimiter) == string::npos);
  fileTypes.clear();
  string newApplicationNames;
  for (const string& tag : StringUtil::Split(applicationNames, PathName::PathNameDelimiter))
  {
    // stop at the miktex application tag; this is always the last tag
    if (Utils::EqualsIgnoreCase(tag, "miktex"))
    {
      break;
    }
    if (Utils::EqualsIgnoreCase(tag, name))
    {
      continue;
    }
    AppendTag(newApplicationNames, tag);
  }
  AppendTag(newApplicationNames, name);
  AppendTag(newApplicationNames, "miktex");
  if (Utils::EqualsIgnoreCase(newApplicationNames, applicationNames))
  {
    return;
  }
  applicationNames = newApplicationNames;
  trace_config->WriteLine("core", T_("application tags: ") + applicationNames);
}
