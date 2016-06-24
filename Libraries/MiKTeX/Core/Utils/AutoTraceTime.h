/* AutoTraceTime.h:

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

#if defined(_MSC_VER)
#  pragma once
#endif

#include "Session/SessionImpl.h"

BEGIN_INTERNAL_NAMESPACE;

class AutoTraceTime
{
public:
  AutoTraceTime(const char * lpsz1, const char * lpsz2) :
    start(clock()),
    str1(lpsz1),
    str2(lpsz2)
  {
  }
public:
  ~AutoTraceTime()
  {
    try
    {
      if (SessionImpl::GetSession()->trace_time->IsEnabled())
      {
        SessionImpl::GetSession()->trace_time->WriteFormattedLine("core", "%s %s %s clock ticks", str1.c_str(), str2.c_str(), std::to_string(clock() - start).c_str());
      }
    }
    catch (const std::exception &)
    {
    }
  }
private:
  clock_t start;
private:
  std::string str1;
private:
  std::string str2;
};

END_INTERNAL_NAMESPACE;
