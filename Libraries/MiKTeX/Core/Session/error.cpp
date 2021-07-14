/* error.cpp: error handling

   Copyright (C) 1996-2020 Christian Schenk

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Session>
#include <miktex/Core/Utils>

#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

SourceLocation::SourceLocation(const string& functionName, const string& fileName, int lineNo) :
  functionName(functionName),
  fileName(GetShortSourceFile(fileName.c_str())),
  lineNo(lineNo)
{
  string baseName = PathName(fileName).GetFileNameWithoutExtension().ToString();
  ostringstream stream;
  stream << std::uppercase << baseName.substr(0, 2) << lineNo;
  tag = stream.str();
}

void Session::FatalMiKTeXError(const string& message, const string& description, const string& remedy, const string& tag, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation)
{
  string programInvocationName = Utils::GetExeName();
  auto trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR);
  trace_error->WriteLine("core", TraceLevel::Fatal, message);
  trace_error->WriteLine("core", TraceLevel::Fatal, fmt::format("Data: {0}", info));
  trace_error->WriteLine("core", TraceLevel::Fatal, fmt::format("Source: {0}:{1}", sourceLocation.fileName, sourceLocation.lineNo));
  string env;
  if (Utils::GetEnvironmentString("MIKTEX_DEBUG_BREAK", env) && env == "1")
  {
    DEBUG_BREAK();
  }
  throw MiKTeXException(programInvocationName, message, description, remedy, tag, info, sourceLocation);
}

MIKTEXINTERNALFUNC(bool) GetCrtErrorMessage(int functionResult, string& errorMessage)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
  const size_t BUFSIZE = 512;
  wchar_t buffer[BUFSIZE];
  if (_wcserror_s(buffer, BUFSIZE, functionResult) != 0)
  {
    return false;
  }
  errorMessage = StringUtil::WideCharToUTF8(buffer);
#  else
  errorMessage = strerror(functionResult);
#endif
  return (true);
}

void Session::FatalCrtError(const string& functionName, int errorCode, const MiKTeXException::KVMAP& info, const SourceLocation& sourceLocation)
{
  string errorMessage;
  if (!GetCrtErrorMessage(errorCode, errorMessage))
  {
    errorMessage = T_("runtime error ") + std::to_string(errorCode);
  }
  string infoString = info.ToString();
  if (!infoString.empty())
  {
    errorMessage += ": " + infoString;
  }
  string programInvocationName = Utils::GetExeName();
  auto trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR);
  trace_error->WriteLine("core", TraceLevel::Fatal, errorMessage);
  trace_error->WriteLine("core", TraceLevel::Fatal, fmt::format(T_("Function: {0}"), functionName));
  trace_error->WriteLine("core", TraceLevel::Fatal, fmt::format(T_("Result: {0}"), errorCode));
  trace_error->WriteLine("core", TraceLevel::Fatal, fmt::format(T_("Data: {0}"), infoString.empty() ? "<no data>" : infoString));
  trace_error->WriteLine("core", TraceLevel::Fatal, fmt::format(T_("Source: {0}"), sourceLocation));
  string env;
  if (Utils::GetEnvironmentString("MIKTEX_DEBUG_BREAK", env) && env == "1")
  {
    DEBUG_BREAK();
  }
  switch (errorCode)
  {
  case EACCES:
    throw UnauthorizedAccessException(programInvocationName, errorMessage, "", "", "", info, sourceLocation);
  case EEXIST:
    throw FileExistsException(programInvocationName, errorMessage, "", "", "", info, sourceLocation);
  case ENOENT:
    throw FileNotFoundException(programInvocationName, errorMessage, "", "", "", info, sourceLocation);
  case EPIPE:
    throw BrokenPipeException(programInvocationName, errorMessage, info, sourceLocation);
  default:
    throw MiKTeXException(programInvocationName, errorMessage, info, sourceLocation);
  }
}
