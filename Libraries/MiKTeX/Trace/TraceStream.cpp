/* TaceStream.cpp: tracing

   Copyright (C) 1996-2016 Christian Schenk

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

TraceStream::~TraceStream()
{
}

struct TraceStreamInfo
{
  string name;
  bool isEnabled;
  vector<TraceCallback*> callbacks;
};

class TraceStreamImpl :
  public TraceStream
{
public:
  virtual void MIKTEXTHISCALL Close();

public:
  virtual void MIKTEXTHISCALL Enable(bool enable);

public:
  virtual bool MIKTEXTHISCALL IsEnabled();

public:
  virtual void MIKTEXCEECALL WriteFormattedLine(const char * lpszFacility, const char * lpszFormat, ...);

public:
  virtual void MIKTEXTHISCALL Write(const char * lpszFacility, const char * lpszText);

public:
  virtual void MIKTEXTHISCALL WriteLine(const char * lpszFacility, const char * lpszText);

public:
  virtual void MIKTEXTHISCALL VTrace(const char * lpszFacility, const char * lpszFormat, va_list arglist);

public:
  TraceStreamImpl(shared_ptr<TraceStreamInfo> info, TraceCallback * callback) :
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
    catch (const exception &)
    {
    }
  }

private:
  shared_ptr<TraceStreamInfo> info;

private:
  TraceCallback * callback;

private:
  void Logger(const string & facility, const string & message, bool appendNewline);

#if ENABLE_LEGACY_TRACING
private:
  void LegacyLogger(const string & facility, const string & message, bool appendNewline);
#endif

private:
  void FormatV(const string & facility, bool appendNewline, const string & format, va_list arglist);

private:
  friend class TraceStream;

private:
  typedef unordered_map<string, shared_ptr<TraceStreamInfo>> TraceStreamTable;

private:
  static mutex traceStreamsMutex;

private:
  static TraceStreamTable traceStreams;

private:
  static string traceFlags;
};

mutex TraceStreamImpl::traceStreamsMutex;
TraceStreamImpl::TraceStreamTable TraceStreamImpl::traceStreams;
string TraceStreamImpl::traceFlags;

void TraceStreamImpl::Logger(const string & facility, const string & message, bool appendNewline)
{
#if ENABLE_LEGACY_TRACING
  if (info->callbacks.size() == 0)
  {
    LegacyLogger(facility, message, appendNewline);
    return;
  }
#endif
  for (TraceCallback * callback : info->callbacks)
  {
    callback->Trace(TraceCallback::TraceMessage(info->name, facility, message));
  }
}

#if ENABLE_LEGACY_TRACING

void TraceStreamImpl::LegacyLogger(const string & facility, const string & message, bool appendNewline)
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
  if (appendNewline)
  {
    str += '\n';
  }
#if defined(MIKTEX_WINDOWS)
  wstring debstr;
  try
  {
    debstr = StringUtil::UTF8ToWideChar(str.c_str());
  }
  catch (const exception &)
  {
    debstr = L"???";
  }
  OutputDebugStringW(debstr.c_str());
#else
  if (stderr != 0)
  {
    fputs(str.c_str(), stderr);
    fflush(stderr);
  }
#endif
}
#endif

void TraceStreamImpl::FormatV(const string & facility, bool appendNewline, const string & format, va_list arglist)
{
  Logger(facility, StringUtil::FormatString(format.c_str(), arglist), appendNewline);
}

void TraceStream::SetTraceFlags(const char * lpszFlags)
{
  // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpszFlags);

  if (lpszFlags == nullptr)
  {
    lpszFlags = "error";
  }

  lock_guard<mutex> lockGuard(TraceStreamImpl::traceStreamsMutex);

  TraceStreamImpl::traceFlags = lpszFlags;

  for (TraceStreamImpl::TraceStreamTable::iterator it = TraceStreamImpl::traceStreams.begin(); it != TraceStreamImpl::traceStreams.end(); ++it)
  {
    it->second->isEnabled = false;
  }

  for (Tokenizer tok(lpszFlags, ",; \n\t"); tok.GetCurrent() != nullptr; ++tok)
  {
    string name(tok.GetCurrent());
    TraceStreamImpl::TraceStreamTable::iterator it = TraceStreamImpl::traceStreams.equal_range(name).first;
    if (it != TraceStreamImpl::traceStreams.end() && it->second != nullptr)
    {
      it->second->isEnabled = true;
    }
  }
}

void TraceStreamImpl::Enable(bool enable)
{
  info->isEnabled = enable;
}

void TraceStreamImpl::WriteFormattedLine(const char * lpszFacility, const char * lpszFormat, ...)
{
  // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpszFacility);
  // TODO: MIKTEX_ASSERT_STRING(lpszFormat);
  if (!IsEnabled())
  {
    return;
  }
  va_list marker;
  va_start(marker, lpszFormat);
  FormatV(lpszFacility == nullptr ? "" : lpszFacility, true, lpszFormat, marker);
  va_end(marker);
}

void TraceStreamImpl::WriteLine(const char * lpszFacility, const char * lpszText)
{
  // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpszFacility);
  // TODO: MIKTEX_ASSERT_STRING(lpszText);
  if (!IsEnabled())
  {
    return;
  }
  Logger(lpszFacility == nullptr ? "" : lpszFacility, lpszText, true);
}

void TraceStreamImpl::Write(const char * lpszFacility, const char * lpszText)
{
  // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpszFacility);
  // TODO: MIKTEX_ASSERT_STRING(lpszText);
  if (!IsEnabled())
  {
    return;
  }
  Logger(lpszFacility == nullptr ? "" : lpszFacility, lpszText, false);
}

void TraceStreamImpl::VTrace(const char * lpszFacility, const char * lpszFormat, va_list arglist)
{
  // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpszFacility);
  // TODO: MIKTEX_ASSERT_STRING(lpszFormat);
  if (!IsEnabled())
  {
    return;
  }
  FormatV(lpszFacility == nullptr ? "" : lpszFacility, true, lpszFormat, arglist);
}

unique_ptr<TraceStream> TraceStream::Open(const string & name, TraceCallback * callback)
{
  lock_guard<mutex> lockGuard(TraceStreamImpl::traceStreamsMutex);
  shared_ptr<TraceStreamInfo> traceStreamInfo = TraceStreamImpl::traceStreams[name];
  if (traceStreamInfo == nullptr)
  {
    traceStreamInfo = make_shared<TraceStreamInfo>();
    traceStreamInfo->name = name;
    bool enable = StringUtil::Contains(TraceStreamImpl::traceFlags.c_str(), name.c_str(), ",; \n\t");
    traceStreamInfo->isEnabled = enable;
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

bool TraceStreamImpl::IsEnabled()
{
  return this->info->isEnabled;
}
