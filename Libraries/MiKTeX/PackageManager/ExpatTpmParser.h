/**
 * @file ExpatTpmParser.h
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

#pragma once

#include <stack>
#include <vector>

#include <expat.h>

#include <miktex/Util/PathName>
#include <miktex/Util/CharBuffer>

#include <miktex/PackageManager/PackageManager>

#include "TpmParser.h"

MPM_INTERNAL_BEGIN_NAMESPACE;

class ExpatTpmParser :
    public TpmParser
{

public:

    ExpatTpmParser();
    ~ExpatTpmParser() override;

    void Parse(const MiKTeX::Util::PathName& path, const std::string& texmfPrefix) override;

    const MiKTeX::Packages::PackageInfo& GetPackageInfo() const override
    {
        return packageInfo;
    }

private:

    void GetFiles(const XML_Char* text, std::vector<std::string>& files);

    static void OnStartElement(void* pv, const XML_Char* name, const XML_Char** aAttr);
    static void OnEndElement(void* pv, const XML_Char* name);
    static void OnCharacterData(void* pv, const XML_Char* lpsz, int len);

    MiKTeX::Util::CharBuffer<XML_Char, 8192> charBuffer;
    std::stack<std::string> elementStack;
    MiKTeX::Packages::PackageInfo packageInfo;
    void* parser = nullptr;
    std::string texMFPrefix;
    std::unique_ptr<MiKTeX::Trace::TraceStream> traceError;
    std::unique_ptr<MiKTeX::Trace::TraceStream> traceStopWatch;
};

MPM_INTERNAL_END_NAMESPACE;
