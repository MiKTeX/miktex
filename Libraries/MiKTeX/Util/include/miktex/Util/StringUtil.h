/* miktex/Util/StringUtil.h:                            -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

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

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(F6BFCB48DCDA42E992057245151CA1D5)
#define F6BFCB48DCDA42E992057245151CA1D5

#include "config.h"

#include <cstddef>
#include <cwchar>

#include <string>

MIKTEX_UTIL_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE StringUtil
{
public:
  /// Concatenates a string to the end of another string.
  /// @param lpszBuf The null-terminated destination string buffer.
  /// @param bufSize Size (in characters) of the destination
  /// string buffer.
  /// @param lpszSource The null-terminated string to be appended.
  /// @return Returns the length (in characters) of the result.
  static MIKTEXUTILCEEAPI(std::size_t) AppendString(char * lpszBuf, std::size_t bufSize, const char * lpszSource);

  /// Replaces all occurences of a sub-string within a string.
  /// @param[out] lpszBuf The destination string buffer.
  /// @param bufSize The size (in characters) of the destination buffer.
  /// @param lpszSource The string to be searched.
  /// @param lpszString1 The sub-string to be replaced.
  /// @param lpszString2 The replacement sub-string.
public:
  static MIKTEXUTILCEEAPI(void) ReplaceString(char * lpszBuf, std::size_t & bufSize, const char * lpszSource, const char * lpszString1, const char * lpszString2);

  /// Copies a string.
  /// @param[out] lpszBuf The destination string buffer.
  /// @param bufSize Size (in characters) of the destination string buffer.
  /// @param lpszSource The null-terminated string to be copied.
  /// @return Returns the length (in characters) of the result.
public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(char * lpszBuf, std::size_t bufSize, const char * lpszSource);

  /// Copies a wide string into another wide string.
  /// @param[out] lpszBuf The destination string buffer.
  /// @param bufSize Size (in characters) of the destination string buffer.
  /// @param lpszSource The null-terminated string to be copied.
  /// @return Returns the length (in characters) of the result.
public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(wchar_t * lpszBuf, std::size_t bufSize, const wchar_t * lpszSource);

  /// Copies a wide string into a single-byte string.
  /// @param[out] lpszBuf The destination string buffer.
  /// @param bufSize Size (in characters) of the destination string buffer.
  /// @param lpszSource The null-terminated string to be copied.
  /// @return Returns the length (in characters) of the result.
public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(char * lpszBuf, std::size_t bufSize, const wchar_t * lpszSource);

public:
  static MIKTEXUTILCEEAPI(std::size_t) CopyString(wchar_t * lpszBuf, std::size_t bufSize, const char * lpszSource);

public:
  static MIKTEXUTILCEEAPI(bool) Contains(const char * lpszList, const char * lpszElement, const char * lpszDelim, bool ignoreCase);

public:
  static bool Contains(const char * lpszList, const char * lpszElement, const char * lpszDelim)
  {
    return Contains(lpszList, lpszElement, lpszDelim, true);
  }

public:
  static bool Contains(const char * lpszList, const char * lpszElement)
  {
    return Contains(lpszList, lpszElement, ",;:");
  }

  /// Creates a formatted string object.
  /// @param lpszFormat The format of the string (printf() syntax).
  /// @param arglist Argument list.
  /// @return Returns a string object.
public:
  static MIKTEXUTILCEEAPI(std::string) FormatString(const char * lpszFormat, va_list arglist);

public:
  static MIKTEXUTILCEEAPI(std::string) FormatString(const char * lpszFormat, ...);

public:
  static MIKTEXUTILCEEAPI(std::wstring) UTF8ToWideChar(const char * lpszUtf8);

public:
  static std::wstring UTF8ToWideChar(const std::string & str)
  {
    return UTF8ToWideChar(str.c_str());
  }

public:
  static MIKTEXUTILCEEAPI(std::string) WideCharToUTF8(const wchar_t * lpszWideChar);

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXUTILCEEAPI(std::string) AnsiToUTF8(const char * lpszAnsi);
#endif
};

MIKTEX_UTIL_END_NAMESPACE;

#endif
