/* miktex/Util/CharBuffer.h:                            -*- C++ -*-

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

#if !defined(DB7B495A23094E01B9F135FC1AAF0D71)
#define DB7B495A23094E01B9F135FC1AAF0D71

#include "config.h"

#include <cstddef>
#include <cwchar>

#include <string>

#include "StringUtil.h"
#include "inliners.h"

MIKTEX_UTIL_BEGIN_NAMESPACE;

/// Instances of this class store characters.
template<typename CharType, int BUFSIZE = 512> class CharBuffer
{
private:
  CharType smallBuffer[BUFSIZE];

private:
  CharType * buffer = smallBuffer;

private:
  std::size_t capacity = BUFSIZE;

public:
  CharBuffer()
  {
    Clear();
  }

  /// Copies another CharBuffer object into a new CharBuffer object.
public:
  CharBuffer(const CharBuffer & other)
  {
    Set(other);
  }

public:
  CharBuffer(const char * lpsz)
  {
    // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpsz);
    Set(lpsz);
  }

public:
  CharBuffer(const wchar_t * lpsz)
  {
    // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpsz);
    Set(lpsz);
  }

public:
  CharBuffer(const std::basic_string<char> & other)
  {
    Set(other);
  }

public:
  CharBuffer(const std::basic_string<wchar_t> & other)
  {
    Set(other);
  }

public:
  CharBuffer(std::size_t n)
  {
    Clear();
    Reserve(n);
  }

public:
  virtual ~CharBuffer()
  {
    try
    {
      Reset();
    }
    catch (const std::exception &)
    {
    }
  }

public:
  void Set(const CharBuffer & rhs)
  {
    if (this != &rhs)
    {
      Reserve(rhs.capacity);
      memcpy(this->buffer, rhs.buffer, rhs.capacity * sizeof(CharType));
    }
  }

public:
  template<typename OtherCharType> void Set(const OtherCharType * lpsz)
  {
    // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpsz);
    if (lpsz == nullptr)
    {
      Reset();
    }
    else
    {
      std::size_t requiredSize;
      if (sizeof(CharType) < sizeof(OtherCharType))
      {
        requiredSize = StrLen(lpsz) * 4 + 1; // worst case: wchar_t to UTF-8
      }
      else
      {
        requiredSize = StrLen(lpsz) + 1;
      }
      Reserve(requiredSize);
      StringUtil::CopyString(buffer, GetCapacity(), lpsz);
    }
  }

public:
  template<typename OtherCharType> void Set(const std::basic_string<OtherCharType> & rhs)
  {
    Set(rhs.c_str());
  }

public:
  void Append(const std::basic_string<CharType> & str)
  {
     Reserve(GetLength() + str.length() + 1);
     StringUtil::AppendString(buffer, GetCapacity(), str.c_str());
  }

public:
  void Append(const CharType * lpsz)
  {
    // TODO: MIKTEX_ASSERT_STRING(lpsz);
    Reserve(GetLength() + StrLen(lpsz) + 1);
    StringUtil::AppendString(buffer, GetCapacity(), lpsz);
  }

public:
  void Append(const CharType * lpsz, std::size_t len)
  {
    std::size_t idx = GetLength();
    Reserve(idx + len + 1);
    for (; len > 0; --len, ++idx, ++lpsz)
    {
      buffer[idx] = *lpsz;
    }
    buffer[idx] = 0;
  }

public:
  void Append(CharType ch)
  {
    std::size_t len = GetLength();
    Reserve(len + 2);
    buffer[len] = ch;
    buffer[len + 1] = 0;
  }

public:
  void Clear()
  {
    buffer[0] = 0;
  }

  /// Tests if the character sequence is empty.
  /// @return Returns true if the character sequence is empty.
public:
  bool Empty() const
  {
    return buffer[0] == 0;
  }

public:
  void Reset()
  {
    if (buffer != smallBuffer)
    {
      delete[] buffer;
      buffer = smallBuffer;
      capacity = BUFSIZE;
    }
    Clear();
  }

public:
  void Reserve(std::size_t newSize)
  {
    if (newSize > BUFSIZE && newSize > capacity)
    {
      CharType * newBuffer = new CharType[newSize];
      memcpy(newBuffer, buffer, capacity * sizeof(CharType));
      if (buffer != smallBuffer)
      {
        delete[] buffer;
      }
      buffer = newBuffer;
      capacity = newSize;
    }
  }

  /// Converts this CharBuffer object into a string object.
  /// @return Returns a string object.
public:
  std::basic_string<CharType> ToString() const
  {
    return std::basic_string<CharType>(buffer);
  }

public:
  CharType * GetData()
  {
    return buffer;
  }

public:
  const CharType * GetData() const
  {
    return buffer;
  }

  // DEPRECATED
public:
  const CharType * Get() const
  {
    return buffer;
  }

public:
  std::size_t GetLength() const
  {
    std::size_t idx;
    for (idx = 0; idx < capacity && buffer[idx] != 0; ++idx)
    {
      ;
    }
    // TODO: MIKTEX_ASSERT(idx < capacity);
    return idx;
  }

public:
  std::size_t GetCapacity() const
  {
    return capacity;
  }

public:
  const CharType & operator[] (std::size_t idx) const
  {
    // TODO: MIKTEX_ASSERT(idx < GetCapacity());
    return buffer[idx];
  }

public:
  CharType & operator[] (std::size_t idx)
  {
    // TODO: MIKTEX_ASSERT(idx < GetCapacity());
    return buffer[idx];
  }

public:
  CharBuffer & operator= (const CharBuffer & rhs)
  {
    Set(rhs);
    return *this;
  }

public:
  template<typename OtherCharType> CharBuffer & operator= (const OtherCharType * lpszRhs)
  {
    Set(lpszRhs);
    return *this;
  }

public:
  template<typename OtherCharType> CharBuffer & operator= (const std::basic_string<OtherCharType> & rhs)
  {
    Set(rhs);
    return *this;
  }

public:
  CharBuffer & operator+= (const CharType * lpszRhs)
  {
    Append(lpszRhs);
    return *this;
  }

public:
  CharBuffer & operator+= (CharType ch)
  {
    Append(ch);
    return *this;
  }
};

MIKTEX_UTIL_END_NAMESPACE;

#endif
