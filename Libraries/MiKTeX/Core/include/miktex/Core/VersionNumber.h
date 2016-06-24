/* miktex/Core/VersionNumber.h:                         -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#if !defined(EED99FD2335944DC974769575687D215)
#define EED99FD2335944DC974769575687D215

#include <miktex/Core/config.h>

#include <ostream>
#include <string>

MIKTEX_CORE_BEGIN_NAMESPACE;

/// An instance of this struct represents a version number.
struct VersionNumber
{
  VersionNumber()
  {
  }

  /// Initializes an instance of the struct.
  /// @param n1 The first part of the version number.
  /// @param n2 The second part of the version number.
  /// @param n3 The third part of the version number.
  /// @param n4 The fourth part of the version number.
  VersionNumber(unsigned n1, unsigned n2, unsigned n3, unsigned n4) :
    n1(n1),
    n2(n2),
    n3(n3),
    n4(n4)
  {
  }

  VersionNumber(long highWord, long lowWord) :
    n1((static_cast<unsigned long>(highWord) >> 16) & 0xffff),
    n2(static_cast<unsigned long>(highWord) & 0xffff),
    n3((static_cast<unsigned long>(lowWord) >> 16) & 0xffff),
    n4(static_cast<unsigned long>(lowWord) & 0xffff)
  {
  }

  /// Initializes an instance of the struct.
  /// @param lpszVersion The version number as a string.
  VersionNumber(const char * lpszVersion)
  {
    *this = Parse(lpszVersion);
  }

  /// The first part of the version number. Usually the major version.
  unsigned n1 = 0;

  /// The second part of the version number. Usually the minor version.
  unsigned n2 = 0;

  /// The third part of the version number. Usually the j2000 number.
  unsigned n3 = 0;

  /// The fourth part of the version nimber. Usually 0.
  unsigned n4 = 0;

  /// Combines the first two parts of the version number.
  /// @return Returns a 32-bit integer.
  long GetHighWord() const
  {
    unsigned long w = (n2 & 0xffff);
    w |= static_cast<unsigned long>(n1 & 0xffff) << 16;
    return static_cast<long>(w);
  }

  /// Combines the last two parts of the version number.
  /// @return Returns a 32-bit integer.
  long GetLowWord() const
  {
    unsigned long w = (n4 & 0xffff);
    w |= static_cast<unsigned long>(n3 & 0xffff) << 16;
    return static_cast<long>(w);
  }

  /// Compares this version number to another.
  /// @param other The other version number.
  /// @return Returns -1, if this version number is less than the other.
  /// Returns 0, if both version numbers compare equal.
  /// Otherwise returns 1.
  int CompareTo(const VersionNumber & other) const
  {
    long cmp1 = GetHighWord() - other.GetHighWord();
    long cmp2 = GetLowWord() - other.GetLowWord();
    if (cmp1 < 0 || (cmp1 == 0 && cmp2 < 0))
    {
      return -1;
    }
    else if (cmp1 > 0 || (cmp1 == 0 && cmp2 > 0))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

  /// Compares this version number to another.
  /// @param other The other version number (as a string).
  /// @return Returns -1, if this version number is less than the other.
  /// Returns 0, if both version numbers compare equal.
  /// Otherwise returns 1.
  int CompareTo(const char * lpszOther) const
  {
    return CompareTo(Parse(lpszOther));
  }

  /// Converts this version number into a string object.
  /// @return Returns the string object.
  MIKTEXCORETHISAPI(std::string) ToString() const;

  /// Parses the string representation of a version number.
  /// @param lpsz The string to parse.
  /// @param[out] The version number object to fill.
  /// @returns Returns true if the string could be parsed.
  static MIKTEXCORECEEAPI(bool) TryParse(const char * lpsz, VersionNumber & versionNumber);

  /// Parses the string representation of a version number.
  /// @param lpsz The string to parse.
  /// @returns Returns the parsed version number.
  static MIKTEXCORECEEAPI(VersionNumber) Parse(const char * lpsz);
};

/// Compares two version numbers.
/// @param v1 The first version number.
/// @param v2 The second version number.
/// @return Returns true, if both version numbers compare equal.
inline bool operator== (const VersionNumber & v1, const VersionNumber & v2)
{
  return v1.CompareTo(v2) == 0;
}

/// Compares two version numbers.
/// @param v1 The first version number.
/// @param v2 The second version number.
/// @return Returns true, if the first version number is less than the
/// other.
inline bool operator< (const VersionNumber & v1, const VersionNumber & v2)
{
  return v1.CompareTo(v2) < 0;
}

inline std::ostream & operator<<(std::ostream & os, const VersionNumber & v)
{
  return os << v.ToString();
}

MIKTEX_CORE_END_NAMESPACE;

#endif
