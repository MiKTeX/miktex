/* StringUtil.cpp:

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

#include "internal.h"

size_t StringUtil::AppendString(char * lpszBuf, size_t bufSize, const char * lpszSource)
{
  // TODO: MIKTEX_ASSERT_STRING(lpszBuf);
  // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  // TODO: MIKTEX_ASSERT_STRING(lpszSource);

  size_t length;

  for (length = 0; length < bufSize && lpszBuf[length] != 0; ++length)
  {
    ;
  }

  if (length == bufSize)
  {
    FATAL_ERROR();
  }

  length += CopyString(&lpszBuf[length], bufSize - length, lpszSource);

  return length;
}

MIKTEXSTATICFUNC(void) CopyString2(char * lpszBuf, size_t bufSize, const char * lpszSource, size_t count)
{
  // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  // TODO: MIKTEX_ASSERT_STRING(lpszSource);

  *lpszBuf = 0;

  if (count == 0)
  {
    return;
  }

  char * lpsz = lpszBuf;

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

void StringUtil::ReplaceString(char * lpszBuf, size_t & bufSize, const char * lpszSource, const char * lpszString1, const char * lpszString2)
{
  // TODO: MIKTEX_ASSERT_STRING(lpszSource);
  // TODO: MIKTEX_ASSERT_STRING(lpszString1);
  // TODO: MIKTEX_ASSERT_STRING(lpszString2);
  if (bufSize == 0)
  {
    // TODO: MIKTEX_ASSERT(lpszBuf == 0);
  }
  else
  {
    // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  }
  while (*lpszSource != 0)
  {
    size_t n;
    for (n = 0; lpszSource[n] != 0 && lpszString1[n] != 0 && lpszSource[n] == lpszString1[n]; ++n)
    {
      ;
    }
    if (n == 0)
    {
      while (*lpszSource != 0 && *lpszSource != lpszString1[0])
      {
        if (lpszBuf == nullptr)
        {
          bufSize += 1;
        }
        else
        {
          *lpszBuf = *lpszSource;
          ++lpszBuf;
          bufSize -= 1;
          if (bufSize == 0)
          {
            FATAL_ERROR();
          }
        }
        ++lpszSource;
      }
    }
    else if (lpszString1[n] == 0)
    {
      lpszSource += n;
      n = strlen(lpszString2);
      if (lpszBuf == nullptr)
      {
        bufSize += n;
      }
      else
      {
        StringUtil::CopyString(lpszBuf, bufSize, lpszString2);
        lpszBuf += n;
        bufSize -= n;
      }
    }
    else
    {
      if (lpszBuf == 0)
      {
        bufSize += n;
      }
      else
      {
        CopyString2(lpszBuf, bufSize, lpszSource, n);
        lpszBuf += n;
        bufSize -= n;
      }
      lpszSource += n;
    }
  }
  if (lpszBuf == nullptr)
  {
    ++bufSize;
  }
  else
  {
    *lpszBuf = 0;
  }
}

template<typename CharType> size_t GenericCopyString(CharType * lpszBuf, size_t bufSize, const CharType * lpszSource)
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

size_t StringUtil::CopyString(char * lpszBuf, size_t bufSize, const char * lpszSource)
{
  return GenericCopyString(lpszBuf, bufSize, lpszSource);
}

size_t StringUtil::CopyString(wchar_t * lpszBuf, size_t bufSize, const wchar_t * lpszSource)
{
  return GenericCopyString(lpszBuf, bufSize, lpszSource);
}

size_t StringUtil::CopyString(char * lpszBuf, size_t bufSize, const wchar_t * lpszSource)
{
  return CopyString(lpszBuf, bufSize, WideCharToUTF8(lpszSource).c_str());
}

size_t StringUtil::CopyString(wchar_t * lpszBuf, size_t bufSize, const char * lpszSource)
{
  return CopyString(lpszBuf, bufSize, UTF8ToWideChar(lpszSource).c_str());
}

bool StringUtil::Contains(const char * lpszList, const char * lpszElement, const char * lpszDelims, bool ignoreCase)
{
  for (Tokenizer tok(lpszList, lpszDelims); tok.GetCurrent() != nullptr; ++tok)
  {
    if (StringCompare(tok.GetCurrent(), lpszElement, ignoreCase) == 0)
    {
      return true;
    }
  }
  return false;
}

string StringUtil::FormatString(const char * lpszFormat, va_list arglist)
{
  CharBuffer<char> autoBuffer;
  int n;
#if defined(_MSC_VER)
  n = _vscprintf(lpszFormat, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  autoBuffer.Reserve(n + 1);
  n = vsprintf_s(autoBuffer.GetData(), autoBuffer.GetCapacity(), lpszFormat, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
  {
    FATAL_ERROR();
  }
#else
  n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), lpszFormat, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
  {
    autoBuffer.Reserve(n + 1);
    n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), lpszFormat, arglist);
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

string StringUtil::FormatString(const char * lpszFormat, ...)
{
  va_list arglist;
  va_start(arglist, lpszFormat);
  string str;
  try
  {
    str = FormatString(lpszFormat, arglist);
  }
  catch (...)
  {
    va_end(arglist);
    throw;
  }
  return str;
}

wstring StringUtil::UTF8ToWideChar(const char * lpszUtf8)
{
  try
  {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(lpszUtf8);
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from UTF-8 byte sequence to wide character string did not succeed.");
  }
}

string StringUtil::WideCharToUTF8(const wchar_t * lpszWideChar)
{
  try
  {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(lpszWideChar);
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from wide character string to UTF-8 byte sequence did not succeed.");
  }
}
