/* CoreStopWatch.h:

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

#pragma once

#include <miktex/Trace/StopWatch>

#include "Session/SessionImpl.h"

CORE_INTERNAL_BEGIN_NAMESPACE;

class CoreStopWatch
{
public:
  CoreStopWatch(const std::string& message) :
    stopWatch(MiKTeX::Trace::StopWatch::Start(SESSION_IMPL()->trace_stopwatch.get(), "core", message))
  {
  }

public:
  ~CoreStopWatch()
  {
    try
    {
      stopWatch->Stop();
    }
    catch (const std::exception&)
    {
    }
  }

private:
  std::unique_ptr<MiKTeX::Trace::StopWatch> stopWatch;
};

CORE_INTERNAL_END_NAMESPACE;
