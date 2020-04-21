/* TaceStream.cpp: tracing

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

#if defined(MIKTEX_TRACE_SHARED)
#  define MIKTEXTRACEEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXTRACEEXPORT
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Util/StringUtil>
#include <miktex/Util/Tokenizer>

#define DE9EF9059C8744B48A68345CD5A8A2C8
#include <miktex/Trace/TraceStream.h>

#if defined(MIKTEX_WINDOWS)
#include <Windows.h>
#endif

#include <ctime>
#include <cstdarg>

#include <algorithm>
#include <codecvt>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

const TraceLevel defaultLevel = TraceLevel::Info;
const string defaultOption = "::info";
const vector<string> defaultOptions = { defaultOption };

// option spec: stream[:facility[:level]]
tuple<string, string, TraceLevel> ParseOption(const string& option)
{
  vector<string> spec = StringUtil::Split(option, ':');
  string streamName;
  string facility;
  TraceLevel level = defaultLevel;
  if (spec.size() > 0)
  {
    streamName = spec[0];
  }
  if (spec.size() > 1)
  {
    facility = spec[1];
  }
  if (spec.size() > 2)
  {
    if (spec[2] == "fatal")
    {
      level = TraceLevel::Fatal;
    }
    else if (spec[2] == "error")
    {
      level = TraceLevel::Error;
    }
    else if (spec[2] == "warning")
    {
      level = TraceLevel::Warning;
    }
    else if (spec[2] == "info")
    {
      level = TraceLevel::Info;
    }
    else if (spec[2] == "trace")
    {
      level = TraceLevel::Trace;
    }
    else if (spec[2] == "debug")
    {
      level = TraceLevel::Error;
    }
    else
    {
      // TODO
    }
  }
  return make_tuple(streamName, facility, level);
}

TraceStream::~TraceStream() noexcept
{
}

struct TraceStreamInfo
{
  string name;
  vector<string> enabledFor;
  TraceLevel level;
  vector<TraceCallback*> callbacks;
};

class TraceStreamImpl :
  public TraceStream
{
public:
  void MIKTEXTHISCALL Close() override;

public:
  bool MIKTEXTHISCALL IsEnabled(const std::string& facility, TraceLevel level) override;

public:
  void MIKTEXTHISCALL WriteLine(const std::string& facility, TraceLevel level, const std::string& text) override;

public:
  void MIKTEXTHISCALL WriteLine(const std::string& facility, const std::string& text) override;

public:
  void MIKTEXCEECALL WriteFormattedLine(const std::string& facility, const char* format, ...) override;

public:
  TraceStreamImpl(shared_ptr<TraceStreamInfo> info, TraceCallback* callback) :
    info(info),
    callback(callback)
  {
    if (callback != nullptr)
    {
      info->callbacks.push_back(callback);
    }
  }

public:
  virtual ~TraceStreamImpl()
  {
    try
    {
      Close();
    }
    catch (const exception&)
    {
    }
  }

private:
  shared_ptr<TraceStreamInfo> info;

private:
  TraceCallback* callback;

private:
  void Logger(const string& facility, TraceLevel level, const string& message);

private:
  friend class TraceStream;

private:
  typedef unordered_map<string, shared_ptr<TraceStreamInfo>> TraceStreamTable;

private:
  static mutex traceStreamsMutex;

private:
  static TraceStreamTable traceStreams;

private:
  static vector<string> options;
};

mutex TraceStreamImpl::traceStreamsMutex;
TraceStreamImpl::TraceStreamTable TraceStreamImpl::traceStreams;
vector<string> TraceStreamImpl::options = defaultOptions;

void TraceStreamImpl::Logger(const string& facility, TraceLevel level, const string& message)
{
  if (!IsEnabled(facility, level))
  {
    return;
  }
  for (TraceCallback* callback : info->callbacks)
  {
    callback->Trace(TraceCallback::TraceMessage(info->name, facility, level, message));
    // TODO: if(callback->Trace)
    break;
  }
}

void TraceStream::SetOptions(const string& optionsString)
{
  vector<string> options;
  for (Tokenizer tok(optionsString, ",; \n\t"); tok; ++tok)
  {
    options.push_back(*tok);
  }
  SetOptions(options);
}

void TraceStream::SetOptions(const vector<string>& options)
{
  lock_guard<mutex> lockGuard(TraceStreamImpl::traceStreamsMutex);

  if (options.empty())
  {
    TraceStreamImpl::options = defaultOptions;
  }
  else
  {
    TraceStreamImpl::options = options;
  }

  for (auto& kv : TraceStreamImpl::traceStreams)
  {
    kv.second->level = defaultLevel;
    kv.second->enabledFor.clear();
  }

  for (const string& opt : TraceStreamImpl::options)
  {
    string optStreamName;
    string optFacility;
    TraceLevel optLevel;
    std::tie(optStreamName, optFacility, optLevel) = ParseOption(opt);
    if (optStreamName.empty())
    {
      for (auto& kv : TraceStreamImpl::traceStreams)
      {
        kv.second->level = optLevel;
        if (!optFacility.empty())
        {
          kv.second->enabledFor.push_back(optFacility);
        }
      }
    }
    else
    {
      TraceStreamImpl::TraceStreamTable::iterator it = TraceStreamImpl::traceStreams.equal_range(optStreamName).first;
      if (it != TraceStreamImpl::traceStreams.end() && it->second != nullptr)
      {
        it->second->level = optLevel;
        if (!optFacility.empty())
        {
          it->second->enabledFor.push_back(optFacility);
        }
      }
    }
  }
}

void TraceStreamImpl::WriteLine(const string& facility, TraceLevel level, const string& text)
{
  Logger(facility, level, text);
}

void TraceStreamImpl::WriteLine(const string& facility, const string& text)
{
  WriteLine(facility, TraceLevel::Trace, text);
}

void TraceStreamImpl::WriteFormattedLine(const string& facility, const char* format, ...)
{
  if (!IsEnabled(facility, TraceLevel::Trace))
  {
    return;
  }
  va_list marker;
  va_start(marker, format);
  Logger(facility, TraceLevel::Trace, StringUtil::FormatStringVA(format, marker));
  va_end(marker);
}

unique_ptr<TraceStream> TraceStream::Open(const string& name, TraceLevel level, TraceCallback* callback)
{
  lock_guard<mutex> lockGuard(TraceStreamImpl::traceStreamsMutex);
  shared_ptr<TraceStreamInfo> traceStreamInfo = TraceStreamImpl::traceStreams[name];
  if (traceStreamInfo == nullptr)
  {
    traceStreamInfo = make_shared<TraceStreamInfo>();
    traceStreamInfo->name = name;
    traceStreamInfo->level = level;
    for (const string& opt : TraceStreamImpl::options)
    {
      string optName;
      string optFacility;
      TraceLevel optLevel;
      tie(optName, optFacility, optLevel) = ParseOption(opt);
      if (optName.empty() || name == optName)
      {
        if (!optFacility.empty())
        {
          traceStreamInfo->enabledFor.push_back(optFacility);
        }
        if (optLevel > level)
        {
          traceStreamInfo->level = optLevel;
        }
      }
    }
    TraceStreamImpl::traceStreams[name] = traceStreamInfo;
  }
  return make_unique<TraceStreamImpl>(traceStreamInfo, callback);
}

unique_ptr<TraceStream> TraceStream::Open(const string& name, TraceCallback* callback)
{
  return Open(name, defaultLevel, callback);
}

void TraceStreamImpl::Close()
{
  if (callback != nullptr)
  {
    vector<TraceCallback*>::const_iterator it = find(info->callbacks.begin(), info->callbacks.end(), callback);
    if (it != info->callbacks.end())
    {
      info->callbacks.erase(it);
    }
    callback = nullptr;
  }
}

bool TraceStreamImpl::IsEnabled(const string& facility, TraceLevel level)
{
  return (this->info->enabledFor.empty() || find(this->info->enabledFor.begin(), this->info->enabledFor.end(), facility) != this->info->enabledFor.end())
    && level <= this->info->level;
}

string TraceCallback::TraceMessage::ToString() const
{
  string result;
  switch (this->level)
  {
  case TraceLevel::Fatal:
    result = "FATAL";
    break;
  case TraceLevel::Error:
    result = "ERROR";
    break;
  case TraceLevel::Warning:
    result = "WARNING";
    break;
  case TraceLevel::Info:
    result = "INFO";
    break;
  case TraceLevel::Trace:
    result = "TRACE";
    break;
  case TraceLevel::Debug:
  default:
    result = "DEBUG";
    break;
  }
  result += ": ";
  result += this->message;
  return result;
}

string TraceStream::MakeOption(const string& name, const string& facility, TraceLevel level)
{
  string levelString;
  switch (level)
  {
  case TraceLevel::Fatal:
    levelString = "fatal";
    break;
  case TraceLevel::Error:
    levelString = "debug";
    break;
  case TraceLevel::Warning:
    levelString = "warning";
    break;
  case TraceLevel::Info:
    levelString = "info";
    break;
  case TraceLevel::Trace:
    levelString = "trace";
    break;
  case TraceLevel::Debug:
  default:
    levelString = "debug";
    break;
  }
  return fmt::format("{0}:{1}:{2}", name, facility, levelString);
}
