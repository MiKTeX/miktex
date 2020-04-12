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

#define ENABLE_LEGACY_TRACING 1

pair<string, string> ParseOption(const string& option)
{
  string facility;
  string name;
  size_t pos = option.find(':');
  if (pos == string::npos)
  {
    name = option;
  }
  else
  {
    facility = option.substr(0, pos);
    name = option.substr(pos + 1);
  }
  return make_pair(facility, name);
}

TraceStream::~TraceStream() noexcept
{
}

struct TraceStreamInfo
{
  string name;
  bool isEnabled;
  vector<string> enabledFor;
  vector<TraceCallback*> callbacks;
};

class TraceStreamImpl :
  public TraceStream
{
public:
  void MIKTEXTHISCALL Close() override;

public:
  bool MIKTEXTHISCALL IsEnabled(const std::string& facility) override;

public:
  void MIKTEXCEECALL WriteFormattedLine(const std::string& facility, const char* format, ...) override;

public:
  void MIKTEXTHISCALL WriteLine(const std::string& facility, const std::string& text) override;

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
  void Logger(const string& facility, const string& message);

#if ENABLE_LEGACY_TRACING
private:
  void LegacyLogger(const string& facility, const string& message);
#endif

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
vector<string> TraceStreamImpl::options;

void TraceStreamImpl::Logger(const string& facility, const string& message)
{
#if ENABLE_LEGACY_TRACING
  if (info->callbacks.size() == 0)
  {
    LegacyLogger(facility, message);
    return;
  }
#endif
  for (TraceCallback* callback : info->callbacks)
  {
    callback->Trace(TraceCallback::TraceMessage(info->name, facility, message));
  }
}

#if ENABLE_LEGACY_TRACING

void TraceStreamImpl::LegacyLogger(const string& facility, const string& message)
{
  string str;
  str.reserve(256);
  str += std::to_string(clock());
  str += " [";
#if defined(MIKTEX_WINDOWS)
  wchar_t szPath[_MAX_PATH];
  if (GetModuleFileNameW(nullptr, szPath, _MAX_PATH) != 0)
  {
    wchar_t szName[_MAX_PATH];
#if 0
    PathName(szPath).GetFileNameWithoutExtension(szName);
#else
    _wsplitpath_s(szPath, nullptr, 0, nullptr, 0, szName, _MAX_PATH, nullptr, 0);
#endif
    str += StringUtil::WideCharToUTF8(szName);
  }
#endif
  str += '.';
  if (!facility.empty())
  {
    str += facility;
  }
  str += "]: ";
  str += message;
  str += '\n';
#if defined(MIKTEX_WINDOWS)
  wstring debstr;
  try
  {
    debstr = StringUtil::UTF8ToWideChar(str);
  }
  catch (const exception&)
  {
    debstr = L"???";
  }
  OutputDebugStringW(debstr.c_str());
#else
  if (stderr != nullptr)
  {
    fputs(str.c_str(), stderr);
    fflush(stderr);
  }
#endif
}
#endif

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
    TraceStreamImpl::options = { "error" };
  }
  else
  {
    TraceStreamImpl::options = options;
  }

  for (auto& kv : TraceStreamImpl::traceStreams)
  {
    kv.second->isEnabled = false;
    kv.second->enabledFor.clear();
  }

  for (const string& opt : TraceStreamImpl::options)
  {
    auto p = ParseOption(opt);
    TraceStreamImpl::TraceStreamTable::iterator it = TraceStreamImpl::traceStreams.equal_range(p.second).first;
    if (it != TraceStreamImpl::traceStreams.end() && it->second != nullptr)
    {
      if (p.first.empty())
      {
        it->second->isEnabled = true;
      }
      else
      {
        it->second->enabledFor.push_back(p.first);
      }
    }
  }
}

void TraceStreamImpl::WriteFormattedLine(const string& facility, const char* format, ...)
{
  if (!IsEnabled(facility))
  {
    return;
  }
  va_list marker;
  va_start(marker, format);
  Logger(facility, StringUtil::FormatStringVA(format, marker));
  va_end(marker);
}

void TraceStreamImpl::WriteLine(const string& facility, const string& text)
{
  if (!IsEnabled(facility))
  {
    return;
  }
  Logger(facility, text);
}

unique_ptr<TraceStream> TraceStream::Open(const string& name, TraceCallback* callback)
{
  lock_guard<mutex> lockGuard(TraceStreamImpl::traceStreamsMutex);
  shared_ptr<TraceStreamInfo> traceStreamInfo = TraceStreamImpl::traceStreams[name];
  if (traceStreamInfo == nullptr)
  {
    traceStreamInfo = make_shared<TraceStreamInfo>();
    traceStreamInfo->name = name;
    traceStreamInfo->isEnabled = false;
    for (const string& opt : TraceStreamImpl::options)
    {
      auto p = ParseOption(opt);
      if (name == p.second)
      {
        if (p.first.empty())
        {
          traceStreamInfo->isEnabled = true;
        }
        else
        {
          traceStreamInfo->enabledFor.push_back(p.first);
        }
      }
    }
    TraceStreamImpl::traceStreams[name] = traceStreamInfo;
  }
  return make_unique<TraceStreamImpl>(traceStreamInfo, callback);
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

bool TraceStreamImpl::IsEnabled(const string& facility)
{
  return this->info->isEnabled || find(this->info->enabledFor.begin(), this->info->enabledFor.end(), facility) != this->info->enabledFor.end();
}
