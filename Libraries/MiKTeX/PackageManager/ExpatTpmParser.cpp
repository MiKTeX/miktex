/**
 * @file ExpatTpmParser.cpp
 * @author Christian Schenk
 * @brief TpmParser implementation using Expat
 *
 * @copyright Copyright © 2001-2022 Christian Schenk
 *
 * This file is part of MiKTeX Package Manager.
 *
 * MiKTeX Package Manager is licensed under GNU General Public License version 2
 * or any later version.
 */

#include "config.h"

#include <locale>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/FileStream>
#include <miktex/Core/Paths>
#include <miktex/Trace/StopWatch>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>
#include <miktex/Util/Tokenizer>

#include "internal.h"
#include "ExpatTpmParser.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

using namespace MiKTeX::Packages::D6AAD62216146D44B580E92711724B78;

#if defined(XML_UNICODE)
#define X_(x) L##x
#else
#define X_(x) x
#endif

ExpatTpmParser::ExpatTpmParser() :
    traceError(TraceStream::Open(MIKTEX_TRACE_ERROR)),
    traceStopWatch(TraceStream::Open(MIKTEX_TRACE_STOPWATCH))
{
}

ExpatTpmParser::~ExpatTpmParser()
{
    try
    {
        if (parser != nullptr)
        {
            XML_ParserFree(reinterpret_cast<XML_Parser>(parser));
            parser = nullptr;
        }
        traceError->Close();
        traceError = nullptr;
        traceStopWatch->Close();
        traceStopWatch = nullptr;
    }
    catch (const exception&)
    {
    }
}

void ExpatTpmParser::GetFiles(const XML_Char* text, vector<string>& files)
{
    MIKTEX_ASSERT(Utils::IsPureAscii(text));
    for (Tokenizer tok(text, X_(";\n\r \t")); tok; ++tok)
    {
        PathName path(*tok);
#if defined(MIKTEX_UNIX)
        path.ConvertToUnix();
#endif
        if (texMFPrefix.empty() || (PathName::Compare(PathName(texMFPrefix), path, texMFPrefix.length()) == 0))
        {
            files.push_back(path.ToString());
        }
    }
}

const XML_Char* GetAttributeValue(const XML_Char** aAttr, const XML_Char* key)
{
    for (size_t i = 0; aAttr[i] != 0; i += 2)
    {
        if (StrCmp(aAttr[i], key) == 0)
        {
            return aAttr[i + 1];
        }
    }
    return nullptr;
}

void ExpatTpmParser::OnStartElement(void* pv, const XML_Char* name, const XML_Char** aAttr)
{
    MIKTEX_ASSERT(pv != nullptr);
    ExpatTpmParser* This = reinterpret_cast<ExpatTpmParser*>(pv);
    try
    {
        This->charBuffer.Clear();
        if (StrCmp(name, X_("TPM:Collection")) == 0 || StrCmp(name, X_("TPM:Package")) == 0)
        {
            const XML_Char* packageName;
            if (!This->elementStack.empty()
                && This->elementStack.top() == X_("TPM:Requires")
                && aAttr != nullptr
                && (packageName = GetAttributeValue(aAttr, X_("name"))) != nullptr)
            {
                MIKTEX_ASSERT(Utils::IsPureAscii(packageName));
                vector<string>::const_iterator it = find(This->packageInfo.requiredPackages.begin(), This->packageInfo.requiredPackages.end(), packageName);
                if (it == This->packageInfo.requiredPackages.end())
                {
                    This->packageInfo.requiredPackages.push_back(packageName);
                }
            }
        }
        else if (StrCmp(name, X_("TPM:Copyright")) == 0)
        {
            const XML_Char* lpszOwner;
            if (aAttr != nullptr && (lpszOwner = GetAttributeValue(aAttr, X_("owner"))) != nullptr)
            {
                This->packageInfo.copyrightOwner = lpszOwner;
            }
            const XML_Char* lpszYear;
            if (aAttr != nullptr && (lpszYear = GetAttributeValue(aAttr, X_("year"))) != nullptr)
            {
                This->packageInfo.copyrightYear = lpszYear;
            }
        }
        else if (StrCmp(name, X_("TPM:CTAN")) == 0)
        {
            const XML_Char* lpszPath;
            if (aAttr != nullptr && (lpszPath = GetAttributeValue(aAttr, X_("path"))) != nullptr)
            {
                This->packageInfo.ctanPath = lpszPath;
            }
        }
        else if (StrCmp(name, X_("TPM:DocFiles")) == 0 || StrCmp(name, X_("TPM:RunFiles")) == 0 || StrCmp(name, X_("TPM:SourceFiles")) == 0)
        {
            int size = 0;
            const XML_Char* lpszSize;
            if (aAttr != nullptr && (lpszSize = GetAttributeValue(aAttr, X_("size"))) != nullptr)
            {
                MIKTEX_ASSERT(Utils::IsPureAscii(lpszSize));
                size = atoi(lpszSize);
            }
            if (StrCmp(name, X_("TPM:DocFiles")) == 0)
            {
                This->packageInfo.sizeDocFiles = size;
            }
            else if (StrCmp(name, X_("TPM:RunFiles")) == 0)
            {
                This->packageInfo.sizeRunFiles = size;
            }
            else if (StrCmp(name, X_("TPM:SourceFiles")) == 0)
            {
                This->packageInfo.sizeSourceFiles = size;
            }
            else
            {
                MIKTEX_UNEXPECTED();
            }
        }
        else if (StrCmp(name, X_("TPM:License")) == 0)
        {
            const XML_Char* lpszType;
            if (aAttr != nullptr && (lpszType = GetAttributeValue(aAttr, X_("type"))) != nullptr)
            {
                This->packageInfo.licenseType = lpszType;
            }
        }
        This->elementStack.push(name);
    }
    catch (const exception&)
    {
    }
}

void ExpatTpmParser::OnEndElement(void* pv, const XML_Char* name)
{
    MIKTEX_ASSERT(pv != nullptr);
    ExpatTpmParser* This = reinterpret_cast<ExpatTpmParser*>(pv);
    try
    {
        MIKTEX_ASSERT(!This->elementStack.empty());
        MIKTEX_ASSERT(This->elementStack.top() == name);
        This->elementStack.pop();
        if (StrCmp(name, X_("TPM:Creator")) == 0)
        {
            This->packageInfo.creator = This->charBuffer.GetData();
        }
        else if (StrCmp(name, X_("TPM:Description")) == 0)
        {
            size_t len = This->charBuffer.GetLength();
            This->packageInfo.description.reserve(len);
            XML_Char lastChar = static_cast<XML_Char>(-1);
            for (size_t idx = 0; idx < len; lastChar = This->charBuffer[idx++])
            {
                XML_Char ch = This->charBuffer[idx];
                if (idx > 0
                    && isspace(ch, locale())
                    && isspace(lastChar, locale()))
                {
                    continue;
                }
                if (ch == X_('\r') || ch == X_('\n'))
                {
                    This->packageInfo.description += X_(' ');
                }
                else
                {
                    This->packageInfo.description += ch;
                }
            }
        }
        else if (StrCmp(name, X_("TPM:DocFiles")) == 0)
        {
            This->packageInfo.docFiles.reserve(100);
            This->GetFiles(This->charBuffer.GetData(), This->packageInfo.docFiles);
        }
        else if (StrCmp(name, X_("TPM:MD5")) == 0)
        {
            This->packageInfo.digest = MD5::Parse(This->charBuffer.GetData());
        }
        else if (StrCmp(name, X_("TPM:Name")) == 0)
        {
            This->packageInfo.displayName = This->charBuffer.GetData();
        }
        else if (StrCmp(name, X_("TPM:RunFiles")) == 0)
        {
            This->packageInfo.runFiles.reserve(1000);
            This->GetFiles(This->charBuffer.GetData(), This->packageInfo.runFiles);
            PathName manifestFile(TEXMF_PREFIX_DIRECTORY);
            manifestFile /= MIKTEX_PATH_PACKAGE_MANIFEST_DIR;
            manifestFile /= This->packageInfo.id;
            manifestFile.AppendExtension(MIKTEX_PACKAGE_MANIFEST_FILE_SUFFIX);
            bool haveManifestFile = false;
            for (const auto& file : This->packageInfo.runFiles)
            {
                if (PathName::Compare(PathName(file), manifestFile) == 0)
                {
                    haveManifestFile = true;
                    break;
                }
            }
            if (!haveManifestFile)
            {
                This->packageInfo.runFiles.push_back(manifestFile.ToString());
            }
        }
        else if (StrCmp(name, X_("TPM:SourceFiles")) == 0)
        {
            This->packageInfo.sourceFiles.reserve(100);
            This->GetFiles(This->charBuffer.GetData(), This->packageInfo.sourceFiles);
        }
        else if (StrCmp(name, X_("TPM:TargetSystem")) == 0)
        {
            This->packageInfo.targetSystem = This->charBuffer.GetData();
        }
        else if (StrCmp(name, X_("TPM:TimePackaged")) == 0)
        {
            This->packageInfo.timePackaged = atoi(This->charBuffer.GetData());
        }
        else if (StrCmp(name, X_("TPM:Title")) == 0)
        {
            size_t len = This->charBuffer.GetLength();
            if (len > 0 && This->charBuffer[len - 1] == X_('.'))
            {
                This->charBuffer[len - 1] = 0;
                --len;
            }
            if ((Utils::EqualsIgnoreCase(This->charBuffer.GetData(), X_("no caption")))
                || (Utils::EqualsIgnoreCase(This->charBuffer.GetData(), X_("no description available"))))
            {
                This->charBuffer.Clear();
                len = 0;
            }
            This->packageInfo.title = This->charBuffer.GetData();
        }
        else if (StrCmp(name, X_("TPM:Version")) == 0)
        {
            This->packageInfo.version = This->charBuffer.GetData();
        }
    }
    catch (const exception&)
    {
    }
}

void ExpatTpmParser::OnCharacterData(void* pv, const XML_Char* lpsz, int len)
{
    MIKTEX_ASSERT(pv != nullptr);
    ExpatTpmParser* This = reinterpret_cast<ExpatTpmParser*>(pv);
    try
    {
        This->charBuffer.Append(lpsz, len);
    }
    catch (const exception&)
    {
    }
}

void ExpatTpmParser::Parse(const PathName& path, const string& texmfPrefix)
{
    unique_ptr<StopWatch> stopWatch = StopWatch::Start(traceStopWatch.get(), TRACE_FACILITY, path.GetFileName().ToString());

    this->texMFPrefix = texmfPrefix;

    packageInfo = PackageInfo();

    packageInfo.id = path.GetFileNameWithoutExtension().ToString();

    size_t bytesToRead = File::GetSize(path);

    FileStream stream(File::Open(path, FileMode::Open, FileAccess::Read, false));

    if (parser == nullptr)
    {
        parser = reinterpret_cast<void*>(XML_ParserCreate(nullptr));
        if (parser == nullptr)
        {
            MIKTEX_UNEXPECTED();
        }
    }
    else
    {
        XML_ParserReset(reinterpret_cast<XML_Parser>(parser), nullptr);
    }

    XML_Parser p = reinterpret_cast<XML_Parser>(parser);

    XML_SetElementHandler(p, OnStartElement, OnEndElement);
    XML_SetCharacterDataHandler(p, OnCharacterData);
    XML_SetUserData(p, reinterpret_cast<void*>(this));

    char buf[1024 * 4];
    size_t n;
    while ((n = stream.Read(buf, sizeof(buf))) > 0)
    {
        bytesToRead -= n;
        XML_Status st = XML_Parse(p, buf, static_cast<int>(n), (bytesToRead == 0));
        if (st == XML_STATUS_ERROR)
        {
            traceError->WriteLine(TRACE_FACILITY, TraceLevel::Error, XML_ErrorString(XML_GetErrorCode(p)));
            MIKTEX_FATAL_ERROR_2(T_("The package manifest file could not be parsed."), "path", path.ToString(), "line", std::to_string(XML_GetCurrentLineNumber(p)), "column", std::to_string(XML_GetCurrentColumnNumber(p)));
        }
    }

    stream.Close();
}
