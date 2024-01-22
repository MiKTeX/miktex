/**
 * @file miktex/Util/StringUtil.h
 * @author Christian Schenk
 * @brief StringUtil class
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Util/config.h>

#include <cstddef>
#include <cwchar>

#include <string>
#include <unordered_map>
#include <vector>

/// @namespace MiKTeX::Util
/// @brief Generic utilities.
MIKTEX_UTIL_BEGIN_NAMESPACE;

class Flattener
{

public:

    Flattener() :
        Flattener('\n')
    {
    }

    Flattener(char sep) :
        sep(sep)
    {
    }

    void operator()(const std::string& s)
    {
        if (!result.empty())
        {
            result += sep;
        }
        result += s;
    }

    std::string result;

private:

    char sep;
};

class MIKTEXNOVTABLE StringUtil
{

public:

    StringUtil() = delete;
    StringUtil(const StringUtil& other) = delete;
    StringUtil& operator=(const StringUtil& other) = delete;
    StringUtil(StringUtil&& other) = delete;
    StringUtil& operator=(StringUtil&& other) = delete;
    ~StringUtil() = delete;

    static MIKTEXUTILCEEAPI(std::string) Flatten(const std::vector<std::string>& vec, char sep);
    static MIKTEXUTILCEEAPI(std::vector<std::string>) Split(const std::string& s, char sep);
    static MIKTEXUTILCEEAPI(std::size_t) AppendCeeString(char* dest, std::size_t destSize, const char* source);
    static MIKTEXUTILCEEAPI(std::size_t) CopyCeeString(char* dest, std::size_t destSize, const char* source);
    static MIKTEXUTILCEEAPI(std::size_t) CopyCeeString(wchar_t* dest, std::size_t destSize, const wchar_t* source);
    static MIKTEXUTILCEEAPI(std::size_t) CopyCeeString(char* dest, std::size_t destSize, const wchar_t* source);
    static MIKTEXUTILCEEAPI(std::size_t) CopyCeeString(char16_t* dest, std::size_t destSize, const char* source);
    static MIKTEXUTILCEEAPI(std::size_t) CopyCeeString(wchar_t* dest, std::size_t destSize, const char* source);
    static MIKTEXUTILCEEAPI(bool) Contains(const std::string& list, const std::string& element, const std::string& delims, bool ignoreCase);
    static MIKTEXUTILCEEAPI(std::string) FormatString2(const std::string& message, const std::unordered_map<std::string, std::string>& args);
    static MIKTEXUTILCEEAPI(std::u16string) UTF8ToUTF16(const std::string& utf8Chars);
    static MIKTEXUTILCEEAPI(std::string) UTF16ToUTF8(const std::u16string& utf16Chars);
    static MIKTEXUTILCEEAPI(std::u32string) UTF8ToUTF32(const std::string& utf8Chars);
    static MIKTEXUTILCEEAPI(std::wstring) UTF8ToWideChar(const std::string& utf8);
    static MIKTEXUTILCEEAPI(std::string) WideCharToUTF8(const std::wstring& wideChars);

#if defined(MIKTEX_WINDOWS)
    static MIKTEXUTILCEEAPI(std::string) AnsiToUTF8(const std::string& ansi);
#endif

    static bool Contains(const std::string& list, const std::string& element, const std::string& delims)
    {
        return Contains(list, element, delims, true);
    }

    static bool Contains(const std::string& list, const std::string& element)
    {
        return Contains(list, element, ",;:", true);
    }
};

MIKTEX_UTIL_END_NAMESPACE;
