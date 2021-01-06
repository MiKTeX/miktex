/* miktex/Util/PathNameUtil.h:                          -*- C++ -*-

   Copyright (C) 2020-2021 Christian Schenk

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

#if !defined(E3DF059408FD47B89E84E189A4C529E7)
#define E3DF059408FD47B89E84E189A4C529E7

#include <miktex/Util/config.h>

#include <string>

MIKTEX_UTIL_BEGIN_NAMESPACE;

class MIKTEXNOVTABLE PathNameUtil
{
public:
  PathNameUtil() = delete;

public:
  PathNameUtil(const PathNameUtil& other) = delete;

public:
  PathNameUtil& operator=(const PathNameUtil& other) = delete;

public:
  PathNameUtil(PathNameUtil&& other) = delete;

public:
  PathNameUtil& operator=(PathNameUtil&& other) = delete;

public:
  ~PathNameUtil() = delete;

public:
  static constexpr char DosDirectoryDelimiter{ '\\' };

public:
  static constexpr char UnixDirectoryDelimiter{ '/' };

public:
  static constexpr char DosPathNameDelimiter{ ';' };

public:
  static constexpr char UnixPathNameDelimiter{ ':' };

public:
  static constexpr char DosVolumeDelimiter{ ':' };

#if defined(MIKTEX_WINDOWS)
public:
  static constexpr char AltDirectoryDelimiter{ UnixDirectoryDelimiter };
#endif

public:
#if defined(MIKTEX_WINDOWS)
  static constexpr char DirectoryDelimiter{ DosDirectoryDelimiter };
#elif defined(MIKTEX_UNIX)
  static constexpr char DirectoryDelimiter{ UnixDirectoryDelimiter };
#endif

public:
#if defined(MIKTEX_WINDOWS)
  static constexpr char PathNameDelimiter{ DosPathNameDelimiter };
#elif defined(MIKTEX_UNIX)
  static constexpr char PathNameDelimiter{ UnixPathNameDelimiter };
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static constexpr char VolumeDelimiter{ DosVolumeDelimiter };
#endif

public:
  static bool IsDosDriveLetter(int ch)
  {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
  }

public:
  static bool IsDosVolumeDelimiter(int ch)
  {
    return ch == DosVolumeDelimiter;
  }

public:
  /// Tests if a character is a directory delimiter.
  /// @param ch The character to test.
  /// @return Returns true if the character is a directory delimiter.
  static bool IsDirectoryDelimiter(int ch)
  {
    if (ch == DirectoryDelimiter)
    {
      return true;
    }
#if defined(MIKTEX_WINDOWS)
    return ch == AltDirectoryDelimiter;
#else
    return false;
#endif
  }

public:
  static bool IsExplicitlyRelative(const std::string& path)
  {
    if (path.empty())
    {
      return false;
    }
    else if (path[0] == '.')
    {
      return (path.length() > 1 && IsDirectoryDelimiter(path[1])) || (path.length() > 2 && path[1] == '.' && IsDirectoryDelimiter(path[2]));
    }
    else
    {
      return false;
    }
  }

  static bool IsAbsolutePath(const std::string& path)
  {
    if (path.empty())
    {
      return false;
    }
    else if (IsDirectoryDelimiter(path[0]))
    {
      // "/xyz/foo.txt", also "//server/xyz/foo.txt"
      return true;
    }
#if defined(MIKTEX_WINDOWS)
    else if (path.length() > 2 && PathNameUtil::IsDosDriveLetter(path[0]) // "C:\xyz\foo.txt"
      && IsDosVolumeDelimiter(path[1])
      && IsDirectoryDelimiter(path[2]))
    {
      return true;
    }
#endif
    else
    {
      return false;
    }
  }

  static bool IsFullyQualifiedPath(const std::string& path)
  {
#if defined(MIKTEX_WINDOWS)
    if (path.length() < 3)
    {
      return false;
    }
    else if (IsDirectoryDelimiter(path[0]) && IsDirectoryDelimiter(path[1]))
    {
      // \\server\xyz\foo.txt
      return true;
    }
    else if (PathNameUtil::IsDosDriveLetter(path[0])
      && IsDosVolumeDelimiter(path[1])
      && IsDirectoryDelimiter(path[2]))
    {
      // C:\xyz\foo.txt
      return true;
    }
    else
    {
      return false;
    }
#else
    return IsAbsolutePath(path);
#endif
  }

public:
  static char ToUnix(char ch)
  {
    return ch == DosDirectoryDelimiter ? UnixDirectoryDelimiter : ch;
  }

public:
  static char ToDos(char ch)
  {
    return ch == UnixDirectoryDelimiter ? DosDirectoryDelimiter : ch;
  }

public:
  static void ConvertToUnix(std::string& path)
  {
    for (char& ch : path)
    {
      ch = ToUnix(ch);
    }
  }

public:
  static void ConvertToDos(std::string& path)
  {
    for (char& ch : path)
    {
      ch = ToDos(ch);
    }
  }

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXUTILCEEAPI(std::wstring) ToLengthExtendedPathName(const std::string& path);
#endif
};

MIKTEX_UTIL_END_NAMESPACE;

#endif
