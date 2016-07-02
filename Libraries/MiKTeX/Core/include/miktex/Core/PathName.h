/* miktex/Core/PathName.h:                              -*- C++ -*-

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

#if !defined(CF4C28E918A44B66B3B0BEF7E88AB721)
#define CF4C28E918A44B66B3B0BEF7E88AB721

#include <miktex/Core/config.h>

#include <string>
#include <ostream>

#include <miktex/Util/CharBuffer>

#include "BufferSizes.h"
#include "Debug.h"
#include "OptionSet.h"
#include "Utils.h"

MIKTEX_CORE_BEGIN_NAMESPACE;

/// Path name conversion option enum class.
enum class ConvertPathNameOption
{
  /// Replaces backslashes with slashes.
  ToUnix,
  /// Replaces slashes with backslashes.
  ToDos,
  /// Replaces capital letters with their small letter counterpart.
  MakeLower,
  /// Replaces small letters with their capital letter counterpart.
  MakeUpper,
#if defined(MIKTEX_WINDOWS)
  /// Removes blanks from the path name.
  RemoveBlanks,
  ToLongPathName,
#endif
  /// Makes the path name relative to the current directory.
  MakeRelative,
  /// Makes the path name absolute.
  MakeAbsolute,
  /// Resolve symbolic links.
  Canonicalize,
};

/// Path name conversion options.
typedef OptionSet<ConvertPathNameOption> ConvertPathNameOptions;

/// Instances of this class can be used to store path names.
class PathName :
  public MiKTeX::Util::CharBuffer<char, BufferSizes::MaxPath>
{
protected:
  typedef CharBuffer<char, BufferSizes::MaxPath> Base;

public:
  enum {
    DosDirectoryDelimiter = '\\',
    DosPathNameDelimiter = ';',
    UnixDirectoryDelimiter = '/',
    UnixPathNameDelimiter = ':',
#if defined(MIKTEX_WINDOWS)
    AltDirectoryDelimiter = UnixDirectoryDelimiter,
    DirectoryDelimiter = DosDirectoryDelimiter,
    PathNameDelimiter = DosPathNameDelimiter,
    VolumeDelimiter = ':',
#elif defined(MIKTEX_UNIX)
    DirectoryDelimiter = UnixDirectoryDelimiter,
    PathNameDelimiter = UnixPathNameDelimiter,
#else
#  error Unimplemented: PathName::DirectoryDelimiter
#endif
  };

#if defined(MIKTEX_WINDOWS)
public:
  static bool IsVolumeDelimiter(int ch)
  {
    return ch == VolumeDelimiter;
  }
#endif

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

  /// Initializes a new PathName object.
public:
  PathName()
  {
  }

  /// Copies another PathName object into a new PathName object.
  /// @param rhs The other PathName object.
public:
  PathName(const PathName & rhs) :
    CharBuffer<char, BufferSizes::MaxPath>(rhs)
  {
  }

public:
  PathName(int zero) :
    CharBuffer<char, BufferSizes::MaxPath>(zero)
  {
  }

  /// Copies a character string into a new PathName object.
  /// @param rhs Null-terminated character string.
public:
  PathName(const char * rhs) :
    CharBuffer<char, BufferSizes::MaxPath>(rhs)
  {
  }

  /// Copies a wide character string into a new PathName object.
  /// @param rhs Null-terminated character string.
public:
  PathName(const wchar_t * rhs) :
    CharBuffer<char, BufferSizes::MaxPath>(rhs)
  {
  }

  /// Copies a string object into a new PathName object.
  /// @param rhs String object.
public:
  PathName(const std::basic_string<char> & rhs) :
    CharBuffer<char, BufferSizes::MaxPath>(rhs)
  {
  }

  /// Copies a string object into a new PathName object.
  /// @param rhs String object.
public:
  PathName(const std::basic_string<wchar_t> & rhs) :
    CharBuffer<char, BufferSizes::MaxPath>(rhs)
  {
  }

#if defined(MIKTEX_WINDOWS)
public:
  PathName(const char * lpszDrive, const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension)
  {
    Set(lpszDrive, lpszAbsPath, lpszRelPath, lpszExtension);
  }
#endif

  /// Combines path name components into a new PathName object.
  /// @param lpszAbsPath The first component (absolute directory path).
  /// @param lpszRelPath The second component (relative file name path).
public:
  PathName(const char * lpszAbsPath, const char * lpszRelPath)
  {
    Set(lpszAbsPath, lpszRelPath);
  }

  /// Combines path name components into a new PathName object.
  /// @param lpszAbsPath The first component (absolute directory path).
  /// @param lpszRelPath The second component (relative file name path).
  /// @param lpszExtension The third component (file name extension).
public:
  PathName(const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension)
  {
    Set(lpszAbsPath, lpszRelPath, lpszExtension);
  }

  /// Combines path name components into a new PathName object.
  /// @param absPath The first component (absolute directory path).
  /// @param relPath The second component (relative file name path).
public:
  PathName(const PathName & absPath, const PathName & relPath)
  {
    Set(absPath, relPath);
  }

  /// Combines path name components into a new PathName object.
  /// @param absPath The first component (absolute directory path).
  /// @param relPath The second component (relative file name path).
  /// @param extension File name extension.
public:
  PathName(const PathName & absPath, const PathName & relPath, const PathName & extension)
  {
    Set(absPath, relPath, extension);
  }

public:
  PathName & operator= (const char * rhs)
  {
    Base::operator= (rhs);
    return *this;
  }

public:
  PathName & operator= (const std::basic_string<char> & rhs)
  {
    Base::operator= (rhs);
    return *this;
  }

  /// Calculates the hash value of this PathName object.
  /// @return Returns the hash value.
public:
  MIKTEXCORETHISAPI(std::size_t) GetHash() const;

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) Combine(char * lpszPath, std::size_t sizePath, const char * lpszDrive, const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension);
#endif

#if defined(MIKTEX_WINDOWS)
public:
  static MIKTEXCORECEEAPI(void) Split(const char * lpszPath, char * lpszDrive, std::size_t sizeDrive, char * lpszDirectory, std::size_t sizeDirectory, char * lpszName, std::size_t sizeName, char * lpszExtension, std::size_t sizeExtension);
#endif

public:
  static MIKTEXCORECEEAPI(void) Split(const char * lpszPath, char * lpszDirectory, std::size_t sizeDirectory, char * lpszName, std::size_t sizeName, char * lpszExtension, std::size_t sizeExtension);

public:
  char * GetFileName(char * lpszFileName) const
  {
    MIKTEX_ASSERT_CHAR_BUFFER(lpszFileName, BufferSizes::MaxPath);
    char szExtension[BufferSizes::MaxPath];
    Split(Get(), nullptr, 0, lpszFileName, BufferSizes::MaxPath, szExtension, BufferSizes::MaxPath);
    MiKTeX::Util::StringUtil::AppendString(lpszFileName, BufferSizes::MaxPath, szExtension);
    return lpszFileName;
  }

public:
  PathName GetFileName() const
  {
    char buf[BufferSizes::MaxPath];
    return GetFileName(buf);
  }

public:
  char * GetFileNameWithoutExtension(char * lpszFileName) const
  {
    MIKTEX_ASSERT_CHAR_BUFFER(lpszFileName, BufferSizes::MaxPath);
    Split(Get(), nullptr, 0, lpszFileName, BufferSizes::MaxPath, nullptr, 0);
    return lpszFileName;
  }

public:
  PathName & GetFileNameWithoutExtension(PathName & fileName) const
  {
    Split(Get(), nullptr, 0, fileName.GetData(), fileName.GetCapacity(), nullptr, 0);
    return fileName;
  }

public:
  PathName GetFileNameWithoutExtension() const
  {
    PathName result;
    return GetFileNameWithoutExtension(result);
    return result;
  }

public:
  static MIKTEXCORECEEAPI(void) Combine(char * lpszPath, std::size_t sizePath, const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension);

#if defined(MIKTEX_WINDOWS)
public:
  PathName & Set(const char * lpszDrive, const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension)
  {
    Combine(GetData(), GetCapacity(), lpszDrive, lpszAbsPath, lpszRelPath, lpszExtension);
    return *this;
  }
#endif

  /// Sets this PathName object equal to the combination three path name
  /// components.
  /// @param lpszAbsPath The first component (absolute directory path).
  /// @param lpszRelPath The second component (relative file name path).
  /// @return Returns a reference to this object.
public:
  PathName & Set(const char * lpszAbsPath, const char * lpszRelPath)
  {
    Combine(GetData(), GetCapacity(), lpszAbsPath, lpszRelPath, nullptr);
    return *this;
  }

  /// Sets this PathName object equal to the combination three path name
  /// components.
  /// @param lpszAbsPath The first component (absolute directory path).
  /// @param lpszRelPath The second component (relative file name path).
  /// @param lpszExtension The third component (file name extension).
  /// @return Returns a reference to this object.
public:
  PathName & Set(const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension)
  {
    Combine(GetData(), GetCapacity(), lpszAbsPath, lpszRelPath, lpszExtension);
    return *this;
  }

  /// Sets this PathName object equal to the combination three path name
  /// components.
  /// @param absPath The first component (absolute directory path).
  /// @param relPath The second component (relative file name path).
  /// @param extension The third component (file name extension).
  /// @return Returns a reference to this object.
public:
  PathName & Set(const PathName & absPath, const PathName & relPath)
  {
    Combine(GetData(), GetCapacity(), absPath.Get(), relPath.Get(), nullptr);
    return *this;
  }

  /// Sets this PathName object equal to the combination three path name
  /// components.
  /// @param absPath The first component (absolute directory path).
  /// @param relPath The second component (relative file name path).
  /// @param extension The third component (file name extension).
  /// @return Returns a reference to this object.
public:
  PathName & Set(const PathName & absPath, const PathName & relPath, const PathName & extension)
  {
    Combine(GetData(), GetCapacity(), absPath.Get(), relPath.Get(), extension.Get());
    return *this;
  }

  /// Removes the file name component from this path name.
  /// @return Returns a reference to this object.
public:
  PathName & RemoveFileSpec()
  {
    return CutOffLastComponent();
  }

  /// Removes the directory component from this path name.
  /// @return Returns a reference to this object.
public:
  PathName & RemoveDirectorySpec()
  {
    char fileName[BufferSizes::MaxPath];
    GetFileName(fileName);
    Base::Set(fileName);
    return *this;
  }

  /// Sets this PathName object equal to the current (working) directory.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName &) SetToCurrentDirectory();

  /// Sets this PathName object equal to the temporary directory.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName &) SetToTempDirectory();

  /// Sets this PathName object equal to the name of a temporary file.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName &) SetToTempFile();

  /// Sets this PathName object equal to the name of a temporary file in the
  /// specified directory.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName &) SetToTempFile(const PathName & directory);

  /// Get the mount point of this PathName.
  /// @return Returns the mount point as a new PathName.
public:
  MIKTEXCORETHISAPI(PathName) GetMountPoint() const;

public:
  MIKTEXCORETHISAPI(PathName &) Convert(ConvertPathNameOptions options);

  /// Replaces backslashes with normal slashes.
  /// @return Returns a reference to this object.
public:
  PathName & ToUnix()
  {
    return Convert({ ConvertPathNameOption::ToUnix });
  }

  /// Replaces normal slashes with backslashes.
  /// @return Returns a reference to this object.
public:
  PathName & ToDos()
  {
    return Convert({ ConvertPathNameOption::ToDos });
  }

#if defined(MIKTEX_WINDOWS)
  PathName & ToLongPathName()
  {
    return Convert({ ConvertPathNameOption::ToLongPathName });
  }
#endif

public:
  std::basic_string<wchar_t> ToWideCharString() const
  {
    return MiKTeX::Util::StringUtil::UTF8ToWideChar(Get());
  }

  /// Transform this path for comparison purposes.
  /// @return Returns a reference to this object.
public:
  PathName & TransformForComparison()
  {
#if defined(MIKTEX_WINDOWS)
    return Convert({ ConvertPathNameOption::ToUnix, ConvertPathNameOption::MakeLower });
#else
    return *this;
#endif
  }

public:
  bool IsComparable() const
  {
#if defined(MIKTEX_WINDOWS)
    for (const char * lpsz = Get(); *lpsz != 0; ++lpsz)
    {
      if (*lpsz == DosDirectoryDelimiter || *lpsz >= 'A' && *lpsz <= 'Z')
      {
        return false;
      }
    }
    return true;
#else
    return true;
#endif
  }

public:
  PathName & Canonicalize()
  {
    return Convert({ ConvertPathNameOption::Canonicalize });
  }

  /// Converts this path name into an absolute path name.
  /// @return Returns a reference to this object.
public:
  PathName & MakeAbsolute()
  {
    return Convert({ ConvertPathNameOption::MakeAbsolute });
  }

  /// Checks to see whether this path name has the specified extension.
  /// @param lpszExtension File name extension.
  /// @return Returns true, if this path name has the specified extension.
public:
  bool HasExtension(const char * lpszExtension) const
  {
    const char * lpszExt = GetExtension();
    return lpszExt != nullptr && PathName::Compare(lpszExt, lpszExtension) == 0;
  }

  /// Gets the file name extension.
  /// @param[out] lpszExtension The string buffer to be filled with the
  /// file name extension.
  /// @return Returns the string buffer.
public:
  char * GetExtension(char * lpszExtension) const
  {
    MIKTEX_ASSERT_CHAR_BUFFER(lpszExtension, BufferSizes::MaxPath);
    Split(Get(), nullptr, 0, nullptr, 0, lpszExtension, BufferSizes::MaxPath);
    return lpszExtension;
  }

  /// Gets the file name extension.
  /// @return Returns the file name extension. Returns 0, if the path name
  /// has no file name extension.
public:
  MIKTEXCORETHISAPI(const char *) GetExtension() const;

  /// Sets the file name extension.
  /// @param lpszExtension The file name extension to set.
  /// @param override Indicates whether an existing file name extension
  /// shall be overridden.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName &) SetExtension(const char * lpszExtension, bool override);

  /// Sets the file name extension.
  /// @param lpszExtension The file name extension to set. Can be 0, /// if the extension is to be removed.
  /// @return Returns a reference to this object.
public:
  PathName & SetExtension(const char * lpszExtension)
  {
    return SetExtension(lpszExtension, true);
  }

public:
  PathName & AppendExtension(const std::string & extension)
  {
    if (extension.empty() || extension[0] != '.')
    {
      Base::Append('.');
    }
    Base::Append(extension);
    return *this;
  }

public:
  PathName & AppendExtension(const char * lpszExtension)
  {
    MIKTEX_ASSERT_STRING(lpszExtension);
    if (*lpszExtension != '.')
    {
      Base::Append('.');
    }
    Base::Append(lpszExtension);
    return *this;
  }

  /// Checks to see whether this path name ends with a directory delimiter.
  /// @return Returns true if the last character is a directory delimiter.
public:
  bool EndsWithDirectoryDelimiter() const
  {
    std::size_t l = GetLength();
    return l > 0 && (IsDirectoryDelimiter(Base::operator[](l - 1)));
  }

  /// Appends a character string to this path name.
  /// @param lpsz The null-terminated character string to add.
  /// @param appendDirectoryDelimiter Indicates whether a directory delimiter
  /// shall be appended before the string.
  /// @return Returns a reference to this object.
public:
  PathName & Append(const char * lpsz, bool appendDirectoryDelimiter)
  {
    if (appendDirectoryDelimiter && !Empty())
    {
      AppendDirectoryDelimiter();
    }
    Base::Append(lpsz);
    return *this;
  }

public:
  PathName & Append(const std::string & str, bool appendDirectoryDelimiter)
  {
    return Append(str.c_str(), appendDirectoryDelimiter);
  }

  /// Appends a path name component to this path name.
  /// @param lpszComponent The null-terminated component to add.
  /// @return Returns a reference to this object.
public:
  PathName & AppendComponent(const char * lpszComponent)
  {
    return Append(lpszComponent, true);
  }

  /// Appends a path name component to this path name.
  /// @param lpszComponent The null-terminated component to add.
  /// @return Returns a reference to this object.
public:
  PathName & operator/= (const char * lpszComponent)
  {
    return AppendComponent(lpszComponent);
  }

  /// Appends a path name component to this path name.
  /// @param component The component to be appended.
  /// @return Returns a reference to this object.
public:
  PathName & operator/= (const PathName & component)
  {
    return AppendComponent(component.Get());
  }

  /// Appends a path name component to this path name.
  /// @param component The component to be appended.
  /// @return Returns a reference to this object.
public:
  PathName & operator/= (const std::basic_string<char> & component)
  {
    return AppendComponent(component.c_str());
  }

  /// Cuts off the last component from the path name.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName &) CutOffLastComponent(bool allowSelfCutting);

  /// Cuts off the last component from the path name.
  /// @return Returns a reference to this object.
public:
  PathName & CutOffLastComponent()
  {
    return CutOffLastComponent(false);
  }

  /// Makes sure that this path name ends with a directory delimiter.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName &) AppendDirectoryDelimiter();

#if defined(MIKTEX_WINDOWS)
public:
  MIKTEXCORETHISAPI(PathName &) AppendAltDirectoryDelimiter();
#endif

public:
  bool IsExplicitlyRelative() const
  {
    if ((*this)[0] == '.')
    {
      return IsDirectoryDelimiter((*this)[1] || ((*this)[1] == '.' && IsDirectoryDelimiter((*this)[2])));
    }
    else
    {
      return false;
    }
  }

public:
  /// Compares the prefixes of two path names.
  /// @param lpszPath1 The first path name.
  /// @param lpszPath1 The second path name.
  /// @param count Number of characters to compare.
  /// @return Returns -1, if the first prefix compares lesser than the second.
  /// Returns 0, if both prefixes compare equal.
  /// Returns 1, if the first prefix compares greater than the second.
  static MIKTEXCORECEEAPI(int) Compare(const char * lpszPath1, const char * lpszPath2, std::size_t count);

public:
  static int Compare(const PathName & path1, const PathName & path2, std::size_t count)
  {
    return Compare(path1.Get(), path2.Get(), count);
  }

  /// Compares two path names.
  /// @param lpszPath1 The first path name.
  /// @param lpszPath1 The second path name.
  /// @return Returns -1, if the first path name compares
  /// lesser than the second.
  /// Returns 0, if both path names compare equal.
  /// Returns 1, if the first path name compares greater than the second.
public:
  static MIKTEXCORECEEAPI(int) Compare(const char * lpszPath1, const char * lpszPath2);

  /// Compares two path names.
  /// @param path1 The first path name.
  /// @param path1 The second path name.
  /// @return Returns -1, if the first path name compares
  /// lesser than the second.
  /// Returns 0, if both path names compare equal.
  /// Returns 1, if the first path name compares greater than the second.
public:
  static int Compare(const PathName & path1, const PathName & path2)
  {
    return Compare(path1.Get(), path2.Get());
  }

public:
  static bool Equals(const PathName & path1, const PathName & path2)
  {
    return Compare(path1, path2) == 0;
  }

  /// Matches a path name against a wildcard pattern.
  /// @param lpszPattern The wildcard pattern.
  /// @param lpszPath The path name to test.
  /// @return Returns true, if the pattern matches.
public:
  static MIKTEXCORECEEAPI(bool) Match(const char * lpszPattern, const char * lpszPath);

public:
  static bool Match(const char * lpszPattern, const PathName & path)
  {
    return Match(lpszPattern, path.Get());
  }
};

inline bool operator< (const PathName & lhs, const PathName & rhs)
{
  return PathName::Compare(lhs, rhs) < 0;
}

/// Compares two path names.
/// @param lhs The first path name.
/// @param rhs The second path name.
/// @return Returns true, if both path names compare equal.
inline bool operator== (const PathName & lhs, const PathName & rhs)
{
  return PathName::Compare(lhs, rhs) == 0;
}

/// Compares two path names.
/// @param lhs The first path name.
/// @param rhs The second path name.
/// @return Returns true, if both path names compare unequal.
inline bool operator!= (const PathName & lhs, const PathName & rhs)
{
  return PathName::Compare(lhs, rhs) != 0;
}

inline PathName operator/ (const PathName & lhs, const PathName & rhs)
{
  PathName result = lhs;
  result /= rhs;
  return result;
}

inline std::ostream & operator<<(std::ostream & os, const PathName & path)
{
  return os << path.ToString();
}

inline bool IsDirectoryDelimiter(int ch)
{
  return PathName::IsDirectoryDelimiter(ch);
}


MIKTEX_CORE_END_NAMESPACE;

namespace std
{
  template <> struct hash<MiKTeX::Core::PathName>
  {
    std::size_t operator ()(const MiKTeX::Core::PathName & path) const
    {
      return path.GetHash();
    }
  };
}

#endif
