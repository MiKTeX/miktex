/* miktex/Util/StringUtil.h:                            -*- C++ -*-

   Copyright (C) 1996-2021 Christian Schenk

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

#pragma once

#if !defined(F6BFCB48DCDA42E992057245151CA1D5)
#define F6BFCB48DCDA42E992057245151CA1D5

#include <miktex/Util/config.h>

#include <cstddef>
#include <cwchar>

#include <string>
#include <vector>
#include <unordered_map>

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
public:
  Flattener(char sep) :
    sep(sep)
  {
  }
public:
  void operator()(const std::string& s)
  {
    if (!result.empty())
    {
      result += sep;
    }
    result += s;
  }
private:
  char sep;
public:
  std::string result;
};

class MIKTEXNOVTABLE StringUtil
{
public:
  StringUtil() = delete;

public:
  StringUtil(const StringUtil& other) = delete;

public:
  StringUtil& operator=(const StringUtil& other) = delete;

public:
  StringUtil(StringUtil&& other) = delete;

public:
  StringUtil& operator=(StringUtil&& other) = delete;

public:
  ~StringUtil() = delete;

public:
  static MIKTEXUTILCEEAPI(std::string) Flatten(const std::vector<std::string>& vec, char sep);

public:
  static MIKTEXUTILCEEAPI(std::vector<std::string>) Split(const std::string& s, char sep);

public:
  static MIKTEXUTILCEEAPI(std::size_t) AppendString(char* dest, std::size_t destSize, const char* source);

public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(char* dest, std::size_t destSize, const char* source);

public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(wchar_t* dest, std::size_t destSize, const wchar_t* source);

public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(char* dest, std::size_t destSize, const wchar_t* source);

public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(char16_t* dest, std::size_t destSize, const char* source);

public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(wchar_t* dest, std::size_t destSize, const char* source);

public:
  static MIKTEXUTILCEEAPI(bool) Contains(const char* list, const char* element, const char* delims, bool ignoreCase);

public:
  static bool Contains(const char* list, const char* element , const char* delims)
  {
    return Contains(list, element, delims, true);
  }

public:
  static bool Contains(const char* list, const char* element)
  {
    return Contains(list, element, ",;:");
  }

public:
  static MIKTEXUTILCEEAPI(std::string) FormatString2(const std::string& message, const std::unordered_map<std::string, std::string>& args);

public:
  static MIKTEXUTILCEEAPI(std::u16string) UTF8ToUTF16(const char* utf8Chars);

public:
  static MIKTEXUTILCEEAPI(std::string) UTF16ToUTF8(const char16_t* utf16Chars);

public:
  static std::string UTF16ToUTF8(const std::u16string& str)
  {
    return UTF16ToUTF8(str.c_str());
  }

public:
  static MIKTEXUTILCEEAPI(std::u32string) UTF8ToUTF32(const char* utf8Chars);

public:
  static MIKTEXUTILCEEAPI(std::string) UTF32ToUTF8(const char32_t* utf32Chars);

public:
  static MIKTEXUTILCEEAPI(std::wstring) UTF8ToWideChar(const char* utf8);

public:
  static std::wstring UTF8ToWideChar(const std::string& str)
  {
    return UTF8ToWideChar(str.c_str());
  }

public:
  static MIKTEXUTILCEEAPI(std::string) WideCharToUTF8(const wchar_t* wideChars);

public:
  static std::string WideCharToUTF8(const std::wstring& wstr)
  {
    return WideCharToUTF8(wstr.c_str());
  }

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXUTILCEEAPI(std::string) AnsiToUTF8(const char* ansi);
#endif
};

MIKTEX_UTIL_END_NAMESPACE;

#endif
