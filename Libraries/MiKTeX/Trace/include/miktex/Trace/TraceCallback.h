/* miktex/Trace/TraceCallback.h:                        -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

   This file is part of the MiKTeX Trace Library.

   The MiKTeX Trace Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Trace Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Trace Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#if !defined(C603C4D3DC8B49F7BF8E691D3A08A925)
#define C603C4D3DC8B49F7BF8E691D3A08A925

#include "config.h"

#include <ostream>
#include <string>

MIKTEX_TRACE_BEGIN_NAMESPACE;

enum class TraceLevel
{
  Fatal,
  Error,
  Warning,
  Info,
  Trace,
  Debug
};

class MIKTEXNOVTABLE TraceCallback
{
public:
  struct TraceMessage
  {
    TraceMessage(const std::string& streamName, const std::string& facility, TraceLevel level, const std::string& message) :
      streamName(streamName),
      facility(facility),
      level(level),
      message(message)
    {
    }
    MIKTEXTRACETHISAPI(std::string) ToString() const;
    std::string streamName;
    std::string facility;
    TraceLevel level;
    std::string message;
  };

public:
  virtual bool MIKTEXTHISCALL Trace(const TraceMessage& traceMessage) = 0;
};

inline std::ostream& operator<<(std::ostream& os, const TraceCallback::TraceMessage& traceMessage)
{
  return os << traceMessage.ToString();
}

MIKTEX_TRACE_END_NAMESPACE;

#endif
