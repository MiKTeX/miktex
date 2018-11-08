/* Exceptions.cpp: MiKTeX exceptions

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

#if defined(HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <fstream>

#include "internal.h"

#include "miktex/Core/Cfg.h"
#include "miktex/Core/Environment.h"
#include "miktex/Core/Exceptions.h"
#include "miktex/Core/Urls.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

MiKTeXException::MiKTeXException() :
  errorMessage(T_("Unknown MiKTeX exception."))
{
}

MiKTeXException::MiKTeXException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation) :
  info(info),
  description(description),
  errorMessage(errorMessage),
  programInvocationName(programInvocationName),
  remedy(remedy),
  sourceLocation(sourceLocation),
  tag(tag)
{
}

bool MiKTeXException::Save(const string& path) const noexcept
{
  try
  {
    ofstream f;
    f.open(path);
    f << "[general]\n"
      << "programInvocationName=" << programInvocationName << "\n"
      << "errorMessage=" << errorMessage << "\n"
      << "description=" << description << "\n"
      << "remedy=" << remedy << "\n"
      << "tag=" << tag << "\n"
      << "[sourceLocation]\n"
      << "functionName=" << sourceLocation.functionName << "\n"
      << "fileName=" << sourceLocation.fileName << "\n"
      << "lineNo=" << sourceLocation.lineNo << "\n"
      << "tag=" << sourceLocation.tag << "\n";
    if (!info.empty())
    {
      f << "[info]\n";
      for (const auto& kv : info)
      {
        f << kv.first << "=" << kv.second << "\n";
      }
    }
    f.close();
    return true;
  }
  catch (const exception&)
  {
    return false;
  }
}

bool MiKTeXException::Load(const string& path, MiKTeXException& ex)
{
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(path);
  bool result = false;
  for (const auto& key : *cfg)
  {
    string keyName = key->GetName();
    for (const auto& val : *key)
    {
      string valueName = val->GetName();
      string value = val->GetValue();
      if (keyName == "general")
      {
        result = true;
        if (valueName == "programInvocationName")
        {
          ex.programInvocationName = value;
        }
        else if (valueName == "errorMessage")
        {
          ex.errorMessage = value;
        }
        else if (valueName == "description")
        {
          ex.description = value;
        }
        else if (valueName == "remedy")
        {
          ex.remedy = value;
        }
        else if (valueName == "tag")
        {
          ex.tag = value;
        }
      }
      else if (keyName == "sourceLocation")
      {
        result = true;
        if (valueName == "functionName")
        {
          ex.sourceLocation.functionName = value;
        }
        else if (valueName == "fileName")
        {
          ex.sourceLocation.fileName = value;
        }
        else if (valueName == "lineNo")
        {
          ex.sourceLocation.lineNo = std::stoi(value);
        }
        else if (valueName == "tag")
        {
          ex.sourceLocation.tag = value;
        }
      }
      else if (keyName == "info")
      {
        result = true;
        ex.info[valueName] = value;
      }
    }
  }
  return result;
}

bool GetLastMiKTeXExceptionPath(string& path)
{
  string env;
  if (GetEnvironmentString(MIKTEX_ENV_EXCEPTION_PATH, env) && !env.empty())
  {
    path = env;
    return true;
  }
  else if (GetEnvironmentString("TMPDIR", env) && !env.empty())
  {
    char lastch = env.back();
#if defined(MIKTEX_WINDOWS)
    if (lastch == '\\')
    {
      lastch = '/';
    }
#endif
    if (lastch != '/')
    {
      env += '/';
    }
    path = env + "lastMiKTeXException";
    return true;
  }
  else
  {
    return false;
  }
}

bool MiKTeXException::Save() const noexcept
{
  try
  {
    string path;
    if (GetLastMiKTeXExceptionPath(path))
    {
      return Save(path);
    }
    else
    {
      return false;
    }
  }
  catch (const exception&)
  {
    return false;
  }
}

bool MiKTeXException::Load(MiKTeXException& ex)
{
  string path;
  if (GetLastMiKTeXExceptionPath(path) && File::Exists(path))
  {
    return Load(path, ex);
  }
  else
  {
    return false;
  }
}

string MiKTeXException::GetErrorMessage() const
{
  return StringUtil::FormatString2(errorMessage, info);
}

string MiKTeXException::GetDescription() const
{
  return StringUtil::FormatString2(description, info);
}

string MiKTeXException::GetRemedy() const
{
  return StringUtil::FormatString2(remedy, info);
}

string MiKTeXException::GetUrl() const
{
  string url;
  if (!tag.empty())
  {
    url = MIKTEX_URL_WWW_KNOWLEDGE_BASE + "/fix-"s + tag;
  }
  return url;
}

OperationCancelledException::OperationCancelledException() :
  MiKTeXException(T_("Operation cancelled."))
{
}

OperationCancelledException::OperationCancelledException(const string& programInvocationName, const string& errorMessage, const KVMAP& info, const SourceLocation& sourceLocation) :
  MiKTeXException(programInvocationName, errorMessage, info, sourceLocation)
{
}

IOException::IOException()
{
}

IOException::IOException(const std::string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation) :
  MiKTeXException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}

FileNotFoundException::FileNotFoundException()
{
}

FileNotFoundException::FileNotFoundException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation) :
  IOException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}

DirectoryNotEmptyException::DirectoryNotEmptyException()
{
}

DirectoryNotEmptyException::DirectoryNotEmptyException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation) :
  IOException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}

BrokenPipeException::BrokenPipeException()
{
}

BrokenPipeException::BrokenPipeException(const string& programInvocationName, const string& errorMessage, const KVMAP& info, const SourceLocation& sourceLocation) :
  IOException(programInvocationName, errorMessage, "", "", "", info, sourceLocation)
{
}

UnauthorizedAccessException::UnauthorizedAccessException()
{
}

UnauthorizedAccessException::UnauthorizedAccessException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation) :
  MiKTeXException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}
