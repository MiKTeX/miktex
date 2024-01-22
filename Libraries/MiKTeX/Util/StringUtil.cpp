/**
 * @file StringUtil.cpp
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

#include <cstdarg>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include <vector>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/CharBuffer.h"
#include "miktex/Util/StringUtil.h"
#include "miktex/Util/Tokenizer.h"
#include "miktex/Util/inliners.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

string StringUtil::Flatten(const std::vector<std::string>& vec, char sep)
{
    return std::for_each(vec.begin(), vec.end(), Flattener(sep)).result;
}

vector<string> StringUtil::Split(const std::string& s, char sep)
{
    vector<string> result;
    if (s.empty())
    {
        return result;
    }
    string current;
    for (const char& ch : s)
    {
        if (ch == sep)
        {
            result.push_back(std::move(current));
        }
        else
        {
            current += ch;
        }
    }
    result.push_back(std::move(current));
    return result;
}

size_t StringUtil::AppendCeeString(char* dest, size_t destSize, const char* source)
{
    // TODO: MIKTEX_ASSERT_STRING(lpszBuf);
    // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
    // TODO: MIKTEX_ASSERT_STRING(lpszSource);

    size_t length;

    for (length = 0; length < destSize && dest[length] != 0; ++length)
    {
        ;
    }

    if (length == destSize)
    {
        FATAL_ERROR();
    }

    length += CopyCeeString(&dest[length], destSize - length, source);

    return length;
}

template<typename CharType> size_t GenericCopyString(CharType* lpszBuf, size_t bufSize, const CharType* lpszSource)
{
    // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
    // TODO: MIKTEX_ASSERT_STRING(lpszSource);

    size_t length = StrLen(lpszSource);

    if (length >= bufSize)
    {
        FATAL_ERROR();
    }

    memcpy(lpszBuf, lpszSource, sizeof(CharType) * (length + 1));

    return length;
}

size_t StringUtil::CopyCeeString(char* dest, size_t destSize, const char* source)
{
    return GenericCopyString(dest, destSize, source);
}

size_t StringUtil::CopyCeeString(wchar_t* dest, size_t destSize, const wchar_t* source)
{
    return GenericCopyString(dest, destSize, source);
}

size_t StringUtil::CopyCeeString(char* dest, size_t destSize, const wchar_t* source)
{
    return CopyCeeString(dest, destSize, WideCharToUTF8(source).c_str());
}

size_t StringUtil::CopyCeeString(char16_t* dest, size_t destSize, const char* source)
{
    return GenericCopyString(dest, destSize, UTF8ToUTF16(source).c_str());
}

size_t StringUtil::CopyCeeString(wchar_t* dest, size_t destSize, const char* source)
{
    return CopyCeeString(dest, destSize, UTF8ToWideChar(source).c_str());
}

bool StringUtil::Contains(const string& list, const string& element, const string& delims, bool ignoreCase)
{
    for (Tokenizer tok(list, delims); tok; ++tok)
    {
        if (StringCompare((*tok), element, ignoreCase) == 0)
        {
            return true;
        }
    }
    return false;
}

string StringUtil::FormatString2(const string& message, const unordered_map<string, string>& args)
{
    CharBuffer<char> result;
    string tmp;
    bool inPlaceholder = false;
    for (const char& ch : message)
    {
        if (inPlaceholder)
        {
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')
            {
                tmp += ch;
            }
            else
            {
                inPlaceholder = false;
                bool flushTmp = true;
                if (ch == '}')
                {
                    auto it = args.find(tmp);
                    if (it != args.end())
                    {
                        result += it->second;
                        flushTmp = false;
                    }
                }
                if (flushTmp)
                {
                    result += '{';
                    result += tmp;
                    result += ch;
                }
            }
        }
        else if (ch == '{')
        {
            inPlaceholder = true;
            tmp = "";
        }
        else
        {
            result += ch;
        }
    }
    if (inPlaceholder)
    {
        result += '{';
        result += tmp;
    }
    return result.ToString();
}

u16string StringUtil::UTF8ToUTF16(const string& utf8Chars)
{
    try
    {
#if _MSC_VER == 1900 || _MSC_VER >= 1910 && _MSC_VER <= 1921
        wstring_convert<codecvt_utf8_utf16<int16_t>, int16_t> conv;
        u16string result;
        for (auto& ch : conv.from_bytes(utf8Chars))
        {
            result += static_cast<char16_t>(ch);
        }
        return result;
#else
        wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> conv;
        return conv.from_bytes(utf8Chars);
#endif
    }
    catch (const range_error&)
    {
        throw Exception("Conversion from UTF-8 byte sequence to UTF-16 string did not succeed.");
    }
}

string StringUtil::UTF16ToUTF8(const u16string& utf16Chars)
{
    try
    {
#if _MSC_VER == 1900 || _MSC_VER >= 1910 && _MSC_VER <= 1921
        // workround for VS2015 bug: 
        // http://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
        wstring_convert<codecvt_utf8_utf16<int16_t>, int16_t> conv;
        const int16_t* p = reinterpret_cast<const int16_t*>(utf16Chars.c_str());
        return conv.to_bytes(p, p + u16string.length());
#else
        wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> conv;
        return conv.to_bytes(utf16Chars);
#endif
    }
    catch (const range_error&)
    {
        throw Exception("Conversion from UFT-16 string to UTF-8 byte sequence did not succeed.");
    }
}

u32string StringUtil::UTF8ToUTF32(const string& utf8Chars)
{
    try
    {
#if _MSC_VER == 1900 || _MSC_VER >= 1910 && _MSC_VER <= 1921
        wstring_convert<codecvt_utf8<int32_t>, int32_t> conv;
        u32string result;
        for (auto& ch : conv.from_bytes(utf8Chars))
        {
            result += static_cast<char32_t>(ch);
        }
        return result;
#else
        wstring_convert<codecvt_utf8<char32_t>, char32_t> conv;
        return conv.from_bytes(utf8Chars);
#endif
    }
    catch (const range_error&)
    {
        throw Exception("Conversion from UTF-8 byte sequence to UTF-32 string did not succeed.");
    }
}

wstring StringUtil::UTF8ToWideChar(const string& utf8Chars)
{
    try
    {
        wstring_convert<codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(utf8Chars);
    }
    catch (const range_error&)
    {
        throw Exception("Conversion from UTF-8 byte sequence to wide character string did not succeed.");
    }
}

string StringUtil::WideCharToUTF8(const wstring& wideChars)
{
    try
    {
        wstring_convert<codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(wideChars);
    }
    catch (const range_error&)
    {
        throw Exception("Conversion from wide character string to UTF-8 byte sequence did not succeed.");
    }
}
