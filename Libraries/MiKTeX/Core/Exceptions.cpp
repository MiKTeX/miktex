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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/Cfg.h"
#include "miktex/Core/Environment.h"
#include "miktex/Core/Exceptions.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

MiKTeXException::MiKTeXException() :
  message(T_("Unknown MiKTeX exception."))
{
}

MiKTeXException::MiKTeXException(const string& programInvocationName, const string& message, const string& desciption, const string& remedy, const KVMAP& info, const SourceLocation& sourceLocation) :
  info(info),
  description(description),
  message(message),
  programInvocationName(programInvocationName),
  remedy(remedy),
  sourceLocation(sourceLocation)
{
}

bool MiKTeXException::Save(const string& path) const
{
  try
  {
    ofstream f;
    f.open(path);
    f << "[general]\n";
    f << "programInvocationName=" << programInvocationName << "\n";
    f << "message=" << message << "\n";
    f << "description=" << description << "\n";
    f << "remedy=" << remedy << "\n";
    f << "[sourceLocation]\n";
    f << "functionName=" << sourceLocation.functionName << "\n";
    f << "fileName=" << sourceLocation.fileName << "\n";
    f << "lineNo=" << sourceLocation.lineNo << "\n";
    f << "tag=" << sourceLocation.tag << "\n";
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

MiKTeXException MiKTeXException::Load(const string& path)
{
  unique_ptr<Cfg> cfg = Cfg::Create();
  cfg->Read(path);
  string programInvocationName;
  string message;
  string description;
  string remedy;
  SourceLocation sourceLocation;
  MiKTeXException::KVMAP info;
  for (const auto& key : cfg->GetKeys())
  {
    string keyName = key->GetName();
    for (const auto& val : key->GetValues())
    {
      string valueName = val->GetName();
      string value = val->GetValue();
      if (keyName == "general")
      {
        if (valueName == "programInvocationName")
        {
          programInvocationName = value;
        }
        else if (valueName == "message")
        {
          message = value;
        }
        else if (valueName == "description")
        {
          description = value;
        }
        else if (valueName == "remedy")
        {
          remedy = value;
        }
      }
      else if (keyName == "sourceLocation")
      {
        if (valueName == "functionName")
        {
          sourceLocation.functionName = value;
        }
        else if (valueName == "fileName")
        {
          sourceLocation.fileName = value;
        }
        else if (valueName == "lineNo")
        {
          sourceLocation.lineNo = std::stoi(value);
        }
        else if (valueName == "tag")
        {
          sourceLocation.tag = value;
        }
      }
      else if (keyName == "info")
      {
        info[valueName] = value;
      }
    }
  }
  return MiKTeXException(programInvocationName, message, description, remedy, info, sourceLocation);
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

bool MiKTeXException::Save() const
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

bool MiKTeXException::Load(MiKTeXException& ex)
{
  string path;
  if (GetLastMiKTeXExceptionPath(path) && File::Exists(path))
  {
    ex = Load(path);
    return true;
  }
  else
  {
    return false;
  }
}

string MiKTeXException::GetDescription() const
{
  return StringUtil::FormatString2(description, info);
}

string MiKTeXException::GetRemedy() const
{
  return StringUtil::FormatString2(remedy, info);
}

OperationCancelledException::OperationCancelledException() :
  MiKTeXException(T_("Operation cancelled."))
{
}

OperationCancelledException::OperationCancelledException(const string& programInvocationName, const string& message, const KVMAP& info, const SourceLocation& sourceLocation) :
  MiKTeXException(programInvocationName, message, info, sourceLocation)
{
}

IOException::IOException()
{
}

IOException::IOException(const std::string& programInvocationName, const std::string& message, const KVMAP& info, const SourceLocation& sourceLocation) :
  MiKTeXException(programInvocationName, message, info, sourceLocation)
{
}

FileNotFoundException::FileNotFoundException()
{
}

FileNotFoundException::FileNotFoundException(const std::string& programInvocationName, const std::string& message, const KVMAP& info, const SourceLocation& sourceLocation) :
  IOException(programInvocationName, message, info, sourceLocation)
{
}

DirectoryNotEmptyException::DirectoryNotEmptyException()
{
}

DirectoryNotEmptyException::DirectoryNotEmptyException(const std::string& programInvocationName, const std::string& message, const KVMAP& info, const SourceLocation& sourceLocation) :
  IOException(programInvocationName, message, info, sourceLocation)
{
}

BrokenPipeException::BrokenPipeException()
{
}

BrokenPipeException::BrokenPipeException(const std::string& programInvocationName, const std::string& message, const KVMAP& info, const SourceLocation& sourceLocation) :
  IOException(programInvocationName, message, info, sourceLocation)
{
}

UnauthorizedAccessException::UnauthorizedAccessException()
{
}

UnauthorizedAccessException::UnauthorizedAccessException(const std::string& programInvocationName, const std::string& message, const KVMAP& info, const SourceLocation& sourceLocation) :
  MiKTeXException(programInvocationName, message, info, sourceLocation)
{
}
