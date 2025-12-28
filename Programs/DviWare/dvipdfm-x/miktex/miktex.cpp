/**
 * @file miktex/dvipdfmx.h
 * @author Christian Schenk
 * @brief MiKTeX DVIPDFMx
 *
 * @copyright Copyright © 2016-2025 Christian Schenk
 *
 * This file is free software; the copyright holder gives unlimited permission
 * to copy and/or distribute it, with or without modifications, as long as this
 * notice is preserved.
 */

#include "dvipdfmx.h"

#include <miktex/App/Application>
#include <miktex/Util/PathName>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/Util/StringUtil>

using namespace MiKTeX::App;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

#include <memory>

extern "C"
{
    // in dvipdfmx.c
    void read_config_file(const char* config);
}

string FormatStringVA(const char* format, va_list arglist)
{
    CharBuffer<char> autoBuffer;
    int n;
#if defined(_MSC_VER)
    n = _vscprintf(format, arglist);
    if (n < 0)
    {
        return "";
    }
    autoBuffer.Reserve(static_cast<size_t>(n) + 1);
    n = vsprintf_s(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
    if (n < 0)
    {
        return "";
    }
    else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
    {
        return "";
    }
#else
    n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
    if (n < 0)
    {
        return "";
    }
    else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
    {
        autoBuffer.Reserve(static_cast<size_t>(n) + 1);
        n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
        if (n < 0)
        {
            return "";
        }
        else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
        {
            return "";
        }
    }
#endif
    return autoBuffer.GetData();
}

extern "C" void miktex_log_error_va(const char* format, va_list args)
{
    Application::GetApplication()->LogError(FormatStringVA(format, args));
}

extern "C" void miktex_log_info_va(const char* format, va_list args)
{
    Application::GetApplication()->LogInfo(FormatStringVA(format, args));
}

extern "C" void miktex_log_warn_va(const char* format, va_list args)
{
    Application::GetApplication()->LogWarn(FormatStringVA(format, args));
}

extern "C" void miktex_read_config_files()
{
    shared_ptr<Session> session = MIKTEX_SESSION();
    vector<PathName> configFiles;
    if (session->FindFile(MIKTEX_PATH_DVIPDFMX_CONFIG, MIKTEX_PATH_TEXMF_PLACEHOLDER, { Session::FindFileOption::All }, configFiles))
    {
        for (std::vector<PathName>::const_reverse_iterator it = configFiles.rbegin(); it != configFiles.rend(); ++it)
        {
            read_config_file(it->GetData());
        }
    }
}
