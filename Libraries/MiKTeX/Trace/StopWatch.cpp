/* StopWatch.cpp: tracing

   Copyright (C) 1996-2018 Christian Schenk

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

#if defined(MIKTEX_TRACE_SHARED)
#  define MIKTEXTRACEEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXTRACEEXPORT
#endif

#include <chrono>
#include <ratio>

#include <fmt/format.h>

#define DE9EF9059C8744B48A68345CD5A8A2C8
#include <miktex/Trace/StopWatch.h>

using namespace MiKTeX::Trace;
using namespace std;

StopWatch::~StopWatch() noexcept
{
}

class StopWatchImpl :
  public StopWatch
{
public:
  StopWatchImpl(TraceStream* traceStream, const string& facility, const string& message) :
    traceStream(traceStream),
    facility(facility),
    message(message)
  {
    if (traceStream != nullptr)
    {
      traceStream->WriteLine(facility, fmt::format("stopwatch START: {}", message));
    }
  }

public:
  ~StopWatchImpl() noexcept override
  {
    try
    {
      if (!stopped)
      {
        Stop();
      }
    }
    catch (const exception&)
    {
    }
  }

public:
  double Stop() override
  {
    if (stopped)
    {
      return 0;
    }
    stopped = true;
    chrono::time_point<chrono::high_resolution_clock> stop = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsedTime = chrono::duration_cast<chrono::duration<double>>(stop - start);
    if (traceStream != nullptr)
    {
      traceStream->WriteLine(facility, fmt::format("stopwatch STOP: {} ({:.4f} seconds)", message, elapsedTime.count()));
      traceStream = nullptr;
    }
    return elapsedTime.count();
  }

private:
  TraceStream* traceStream;

private:
  string facility;

private:
  string message;

private:
  chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();

private:
  bool stopped = false;
};

unique_ptr<StopWatch> StopWatch::Start()
{
  return make_unique<StopWatchImpl>(nullptr, "", "");
}

unique_ptr<StopWatch> StopWatch::Start(TraceStream* traceStream, const string& facility, const string& message)
{
  return make_unique<StopWatchImpl>(traceStream, facility, message);
}
