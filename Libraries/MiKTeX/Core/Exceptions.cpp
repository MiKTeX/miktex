/**
 * @file Exceptions.cpp
 * @author Christian Schenk
 * @brief MiKTeX exceptions
 *
 * @copyright Copyright © 1996-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Core Library.
 *
 * The MiKTeX Core Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include "config.h"

#include <fstream>
#include <iostream>

#include <miktex/Core/Cfg>
#include <miktex/Core/Environment>
#include <miktex/Core/Exceptions>
#include <miktex/Core/Urls>

#include "internal.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

MiKTeXException::MiKTeXException():
    errorMessage(T_("Unknown MiKTeX exception."))
{
}

MiKTeXException::MiKTeXException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation):
    description(description),
    errorMessage(errorMessage),
    info(info),
    programInvocationName(programInvocationName),
    sourceLocation(sourceLocation),
    remedy(remedy),
    tag(tag)
{
}

bool MiKTeXException::Save(const string& path) const noexcept
{
    try
    {
        ofstream f;
        f.open(path);
        f
            << "[general]\n"
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
    cfg->Read(PathName(path));
    bool result = false;
    for (const auto& key : *cfg)
    {
        string keyName = key->GetName();
        for (const auto& val : *key)
        {
            string valueName = val->GetName();
            string value = val->AsString();
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
        string str;
        if (Utils::GetEnvironmentString("MIKTEX_PRINT_EXCEPTIONS", str) && str == "cerr")
        {
            cerr << *this << endl;
        }
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
    if (GetLastMiKTeXExceptionPath(path) && File::Exists(PathName(path)))
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

OperationCancelledException::OperationCancelledException():
    MiKTeXException(T_("Operation cancelled."))
{
}

OperationCancelledException::OperationCancelledException(const string& programInvocationName, const string& errorMessage, const KVMAP& info, const SourceLocation& sourceLocation):
    MiKTeXException(programInvocationName, errorMessage, info, sourceLocation)
{
}

IOException::IOException()
{
}

IOException::IOException(const std::string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation):
    MiKTeXException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}

FileExistsException::FileExistsException()
{
}

FileExistsException::FileExistsException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation):
    IOException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}

FileNotFoundException::FileNotFoundException()
{
}

FileNotFoundException::FileNotFoundException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation):
    IOException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}

DirectoryNotEmptyException::DirectoryNotEmptyException()
{
}

DirectoryNotEmptyException::DirectoryNotEmptyException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation):
    IOException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}

BrokenPipeException::BrokenPipeException()
{
}

BrokenPipeException::BrokenPipeException(const string& programInvocationName, const string& errorMessage, const KVMAP& info, const SourceLocation& sourceLocation):
    IOException(programInvocationName, errorMessage, "", "", "", info, sourceLocation)
{
}

UnauthorizedAccessException::UnauthorizedAccessException()
{
}

UnauthorizedAccessException::UnauthorizedAccessException(const string& programInvocationName, const string& errorMessage, const string& description, const string& remedy, const string& tag, const KVMAP& info, const SourceLocation& sourceLocation):
    MiKTeXException(programInvocationName, errorMessage, description, remedy, tag, info, sourceLocation)
{
}
