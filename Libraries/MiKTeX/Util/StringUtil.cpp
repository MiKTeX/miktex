/* StringUtil.cpp:

   Copyright (C) 1996-2018 Christian Schenk

   This file is part of the MiKTeX Util Library.

   The MiKTeX Util Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Util Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Util Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include <cstdarg>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include <vector>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#  define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXUTILEXPORT
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

size_t StringUtil::AppendString(char* dest, size_t destSize, const char* source)
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

  length += CopyString(&dest[length], destSize - length, source);

  return length;
}

MIKTEXSTATICFUNC(void) CopyString2(char* lpszBuf, size_t bufSize, const char* lpszSource, size_t count)
{
  // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  // TODO: MIKTEX_ASSERT_STRING(lpszSource);

  *lpszBuf = 0;

  if (count == 0)
  {
    return;
  }

  char* lpsz = lpszBuf;

  while (bufSize > 0 && count > 0 && (*lpsz++ = *lpszSource++) != 0)
  {
    --bufSize;
    --count;
  }

  if (bufSize == 0)
  {
    FATAL_ERROR();
  }

  if (count == 0)
  {
    *lpsz = 0;
  }
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

size_t StringUtil::CopyString(char* dest, size_t destSize, const char* source)
{
  return GenericCopyString(dest, destSize, source);
}

size_t StringUtil::CopyString(wchar_t* dest, size_t destSize, const wchar_t* source)
{
  return GenericCopyString(dest, destSize, source);
}

size_t StringUtil::CopyString(char* dest, size_t destSize, const wchar_t* source)
{
  return CopyString(dest, destSize, WideCharToUTF8(source).c_str());
}

size_t StringUtil::CopyString(char16_t* dest, size_t destSize, const char* source)
{
  return GenericCopyString(dest, destSize, UTF8ToUTF16(source).c_str());
}

size_t StringUtil::CopyString(wchar_t* dest, size_t destSize, const char* source)
{
  return CopyString(dest, destSize, UTF8ToWideChar(source).c_str());
}

bool StringUtil::Contains(const char* list, const char* element, const char* delims, bool ignoreCase)
{
  for (Tokenizer tok(list, delims); tok; ++tok)
  {
    if (StringCompare((*tok).c_str(), element, ignoreCase) == 0)
    {
      return true;
    }
  }
  return false;
}

string StringUtil::FormatStringVA(const char* format, va_list arglist)
{
  CharBuffer<char> autoBuffer;
  int n;
#if defined(_MSC_VER)
  n = _vscprintf(format, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  autoBuffer.Reserve(n + 1);
  n = vsprintf_s(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
  {
    FATAL_ERROR();
  }
#else
  n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
  {
    autoBuffer.Reserve(n + 1);
    n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
    if (n < 0)
    {
      FATAL_ERROR();
    }
    else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
    {
      FATAL_ERROR();
    }
  }
#endif
  return autoBuffer.GetData();
}

string StringUtil::FormatString(const char* format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  string str;
  try
  {
    str = FormatStringVA(format, arglist);
  }
  catch (...)
  {
    va_end(arglist);
    throw;
  }
  return str;
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

u16string StringUtil::UTF8ToUTF16(const char* utf8Chars)
{
  try
  {
#if _MSC_VER == 1900 || _MSC_VER >= 1910 && _MSC_VER <= 1915
    wstring_convert<codecvt_utf8_utf16<int16_t>, int16_t> conv;
    u16string result;
    for (auto& ch : conv.from_bytes(utf8Chars))
    {
      result += (char16_t)ch;
    }
    return result;
#else
    wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> conv;
    return conv.from_bytes(utf8Chars);
#endif
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from UTF-8 byte sequence to UTF-16 string did not succeed.");
  }
}

string StringUtil::UTF16ToUTF8(const char16_t* utf16Chars)
{
  try
  {
#if _MSC_VER == 1900 || _MSC_VER >= 1910 && _MSC_VER <= 1915
    // workround for VS2015 bug: 
    // http://stackoverflow.com/questions/32055357/visual-studio-c-2015-stdcodecvt-with-char16-t-or-char32-t
    wstring_convert<codecvt_utf8_utf16<int16_t>, int16_t> conv;
    const int16_t* p = (const int16_t*)utf16Chars;
    return conv.to_bytes(p, p + StrLen(utf16Chars));
#else
    wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> conv;
    return conv.to_bytes(utf16Chars);
#endif
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from UFT-16 string to UTF-8 byte sequence did not succeed.");
  }
}

u32string StringUtil::UTF8ToUTF32(const char* utf8Chars)
{
  try
  {
#if _MSC_VER == 1900 || _MSC_VER >= 1910 && _MSC_VER <= 1915
    wstring_convert<codecvt_utf8<int32_t>, int32_t> conv;
    u32string result;
    for (auto& ch : conv.from_bytes(utf8Chars))
    {
      result += (char32_t)ch;
    }
    return result;
#else
    wstring_convert<codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(utf8Chars);
#endif
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from UTF-8 byte sequence to UTF-32 string did not succeed.");
  }
}

string StringUtil::UTF32ToUTF8(const char32_t* utf32Chars)
{
  try
  {
#if _MSC_VER == 1900 || _MSC_VER >= 1910 && _MSC_VER <= 1915
    wstring_convert<codecvt_utf8<int32_t>, int32_t> conv;
    const int32_t* p = (const int32_t*)utf32Chars;
    return conv.to_bytes(p, p + StrLen(utf32Chars));
#else
    wstring_convert<codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(utf32Chars);
#endif
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from UFT-32 string to UTF-8 byte sequence did not succeed.");
  }
}

wstring StringUtil::UTF8ToWideChar(const char* utf8Chars)
{
  try
  {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(utf8Chars);
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from UTF-8 byte sequence to wide character string did not succeed.");
  }
}

string StringUtil::WideCharToUTF8(const wchar_t* wideChars)
{
  try
  {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wideChars);
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from wide character string to UTF-8 byte sequence did not succeed.");
  }
}
