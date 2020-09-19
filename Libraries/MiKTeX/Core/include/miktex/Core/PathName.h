/* miktex/Core/PathName.h:                              -*- C++ -*-

   Copyright (C) 1996-2020 Christian Schenk

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

#pragma once

#if !defined(CF4C28E918A44B66B3B0BEF7E88AB721)
#define CF4C28E918A44B66B3B0BEF7E88AB721

#include <miktex/Core/config.h>

#include <string>
#include <ostream>
#include <vector>

#include <miktex/Util/CharBuffer>
#include <miktex/Util/PathNameUtil>

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
  /// Prefix with \\?\ to create an extended-length path name.
  ToExtendedLengthPathName,
#endif
  /// Makes the path name fully qualified.
  MakeFullyQualified,
  /// Resolve symbolic links.
  Canonicalize,
};

/// Path name conversion options.
typedef OptionSet<ConvertPathNameOption> ConvertPathNameOptions;

enum class DisplayPathNameOption
{

};

typedef OptionSet<DisplayPathNameOption> DisplayPathNameOptions;

/// Instances of this class can be used to store path names.
class PathName :
  public MiKTeX::Util::CharBuffer<char, BufferSizes::MaxPath>
{
protected:
  typedef CharBuffer<char, BufferSizes::MaxPath> Base;

public:
  PathName() = default;

public:
  PathName(const PathName& other) = default;

public:
  PathName& operator=(const PathName& other) = default;

public:
  PathName(PathName&& other) noexcept = default;

public:
  PathName& operator=(PathName&& other) noexcept = default;

public:
  virtual ~PathName() noexcept = default;

  /// Copies a character string into a new PathName object.
  /// @param rhs Null-terminated character string.
public:
  explicit PathName(const char* path) :
    Base(path)
  {
  }

  /// Copies a wide character string into a new PathName object.
  /// @param rhs Null-terminated character string.
public:
  explicit PathName(const wchar_t* path) :
    Base(path)
  {
  }

  /// Copies a string object into a new PathName object.
  /// @param rhs String object.
public:
  explicit PathName(const std::string& path) :
    Base(path)
  {
  }

  /// Copies a string object into a new PathName object.
  /// @param rhs String object.
public:
  explicit PathName(const std::wstring& path) :
    Base(path)
  {
  }

public:
  PathName(size_t n) = delete;

  /// Combines path name components into a new PathName object.
  /// @param component1 The first component (fully qualified directory path).
  /// @param component2 The second component (relative file name path).
public:
  PathName(const char* component1, const char* component2) :
    Base(component1)
  {
    if (component2 != nullptr)
    {
      AppendComponent(component2);
    }
  }

  /// Combines path name components into a new PathName object.
  /// @param component1 The first component (fully qualified directory path).
  /// @param component2 The second component (relative file name path).
public:
  PathName(const PathName& component1, const PathName& component2) :
    PathName(component1.GetData(), component2.GetData())
  {
  }

public:
  PathName& operator=(const char* path)
  {
    Base::operator= (path);
    return *this;
  }

public:
  PathName& operator=(const wchar_t* path)
  {
    Base::operator= (path);
    return *this;
  }

public:
  PathName& operator=(const std::string& path)
  {
    Base::operator= (path);
    return *this;
  }

public:
  PathName& operator=(const std::wstring& path)
  {
    Base::operator= (path);
    return *this;
  }

  /// Calculates the hash value of this PathName object.
  /// @return Returns the hash value.
public:
  MIKTEXCORETHISAPI(std::size_t) GetHash() const;

public:
  static MIKTEXCORECEEAPI(std::vector<std::string>) Split(const PathName& path);

private:
  static MIKTEXCORECEEAPI(void) Split(const PathName& path, std::string& directoryName, std::string& fileNameWithoutExtension, std::string& extension);

public:
  PathName GetDirectoryName() const
  {
    std::string directoryName;
    std::string fileNameWithoutExtension;
    std::string extension;
    Split(*this, directoryName, fileNameWithoutExtension, extension);
    return PathName(directoryName);
  }

public:
  PathName GetFileName() const
  {
    std::string directoryName;
    std::string fileNameWithoutExtension;
    std::string extension;
    Split(*this, directoryName, fileNameWithoutExtension, extension);
    return PathName(fileNameWithoutExtension + extension);
  }

public:
  PathName GetFileNameWithoutExtension() const
  {
    std::string directoryName;
    std::string fileNameWithoutExtension;
    std::string extension;
    Split(*this, directoryName, fileNameWithoutExtension, extension);
    return PathName(fileNameWithoutExtension);
  }

  /// Removes the file name component from this path name.
  /// @return Returns a reference to this object.
public:
  PathName& RemoveFileSpec()
  {
    return CutOffLastComponent();
  }

  /// Removes the directory component from this path name.
  /// @return Returns a reference to this object.
public:
  PathName& RemoveDirectorySpec()
  {
    Base::Set(GetFileName());
    return *this;
  }
  
public:
  MIKTEXCORETHISAPI(PathName&) SetToHomeDirectory();
  
public:
  MIKTEXCORETHISAPI(PathName&) SetToLockDirectory();

  /// Sets this PathName object equal to the current (working) directory.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName&) SetToCurrentDirectory();

  /// Sets this PathName object equal to the temporary directory.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName&) SetToTempDirectory();

  /// Sets this PathName object equal to the name of a temporary file.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName&) SetToTempFile();

  /// Sets this PathName object equal to the name of a temporary file in the
  /// specified directory.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName&) SetToTempFile(const PathName& directory);

  /// Get the mount point of this PathName.
  /// @return Returns the mount point as a new PathName.
public:
  MIKTEXCORETHISAPI(PathName) GetMountPoint() const;

public:
  MIKTEXCORETHISAPI(PathName&) Convert(ConvertPathNameOptions options);

  /// Replaces backslashes with normal slashes.
  /// @return Returns a reference to this object.
public:
  PathName& ConvertToUnix()
  {
    return Convert({ ConvertPathNameOption::ToUnix });
  }

public:
  PathName ToUnix() const
  {
    PathName result = *this;
    result.Convert({ ConvertPathNameOption::ToUnix });
    return result;
  }

  /// Replaces normal slashes with backslashes.
  /// @return Returns a reference to this object.
public:
  PathName& ConvertToDos()
  {
    return Convert({ ConvertPathNameOption::ToDos });
  }

public:
  PathName ToDos() const
  {
    PathName result = *this;
    result.Convert({ ConvertPathNameOption::ToDos });
    return result;
  }

#if defined(MIKTEX_WINDOWS)
  PathName ToExtendedLengthPathName() const
  {
    PathName result = *this;
    result.Convert({ ConvertPathNameOption::ToExtendedLengthPathName });
    return result;
  }
#endif

public:
  std::wstring ToWideCharString() const
  {
    return MiKTeX::Util::StringUtil::UTF8ToWideChar(GetData());
  }

#if defined(MIKTEX_WINDOWS)
public:
  std::wstring ToNativeString() const
  {
    return ToExtendedLengthPathName().ToWideCharString();
  }
#else
public:
  std::string ToNativeString() const
  {
    return ToString();
  }
#endif

public:
  MIKTEXCORETHISAPI(std::string) ToDisplayString(DisplayPathNameOptions options = {}) const;

  /// Transform this path for comparison purposes.
  /// @return Returns a reference to this object.
public:
  PathName& TransformForComparison()
  {
#if defined(MIKTEX_WINDOWS)
    return Convert({ ConvertPathNameOption::ToUnix, ConvertPathNameOption::MakeLower });
#else
    return *this;
#endif
  }

public:
  bool IsFullyQUalified() const
  {
    return MiKTeX::Util::PathNameUtil::IsFullyQualifiedPath(ToString());
  }

public:
  bool IsAbsolute() const
  {
    return MiKTeX::Util::PathNameUtil::IsAbsolutePath(ToString());
  }

public:
  bool IsComparable() const
  {
#if defined(MIKTEX_WINDOWS)
    for (const char* lpsz = GetData(); *lpsz != 0; ++lpsz)
    {
      if (*lpsz == MiKTeX::Util::PathNameUtil::DosDirectoryDelimiter || (*lpsz >= 'A' && *lpsz <= 'Z'))
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
  PathName& Canonicalize()
  {
    return Convert({ ConvertPathNameOption::Canonicalize });
  }

  /// Converts this path name into a fully qualified path name.
  /// @return Returns a reference to this object.
public:
  PathName& MakeFullyQualified()
  {
    return Convert({ ConvertPathNameOption::MakeFullyQualified });
  }

public:
  bool HasExtension() const
  {
    return !GetExtension().empty();
  }

  /// Checks to see whether this path name has the specified extension.
  /// @param lpszExtension File name extension.
  /// @return Returns true, if this path name has the specified extension.
public:
  bool HasExtension(const char* extension) const
  {
    MIKTEX_ASSERT_STRING(extension);
    std::string currentExtension = GetExtension();
    if (currentExtension.empty())
    {
      return false;
    }
    if (extension[0] == '.')
    {
      extension += 1;
    }
    MIKTEX_ASSERT(currentExtension[0] == '.');
    return PathName::Compare(currentExtension.substr(1), extension) == 0;
  }

  /// Gets the file name extension.
  /// @return Returns the file name extension. Returns 0, if the path name
  /// has no file name extension.
public:
  MIKTEXCORETHISAPI(std::string) GetExtension() const;

  /// Sets the file name extension.
  /// @param extension The file name extension to set.
  /// @param override Indicates whether an existing file name extension
  /// shall be overridden.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName&) SetExtension(const char* extension, bool override);

  /// Sets the file name extension.
  /// @param lpszExtension The file name extension to set. Can be 0,
  /// if the extension is to be removed.
  /// @return Returns a reference to this object.
public:
  PathName& SetExtension(const char* extension)
  {
    return SetExtension(extension, true);
  }

  /// Sets the file name extension.
  /// @param lpszExtension The file name extension to set. Can be 0,
  /// if the extension is to be removed.
  /// @return Returns a reference to this object.
public:
  PathName& SetExtension(const std::string& extension)
  {
    return SetExtension(extension.c_str(), true);
  }

public:
  PathName& AppendExtension(const char* extension)
  {
    MIKTEX_ASSERT_STRING(extension);
    if (!HasExtension(extension))
    {
      if (*extension != '.')
      {
        Base::Append('.');
      }
      Base::Append(extension);
    }
    return *this;
  }

public:
  PathName& AppendExtension(const std::string& extension)
  {
    return AppendExtension(extension.c_str());
  }

  /// Checks to see whether this path name ends with a directory delimiter.
  /// @return Returns true if the last character is a directory delimiter.
public:
  bool EndsWithDirectoryDelimiter() const
  {
    std::size_t l = GetLength();
    return l > 0 && (MiKTeX::Util::PathNameUtil::IsDirectoryDelimiter(Base::operator[](l - 1)));
  }

  /// Appends a character string to this path name.
  /// @param lpsz The null-terminated character string to add.
  /// @param appendDirectoryDelimiter Indicates whether a directory delimiter
  /// shall be appended before the string.
  /// @return Returns a reference to this object.
public:
  PathName& Append(const char* lpsz, bool appendDirectoryDelimiter)
  {
    if (appendDirectoryDelimiter && !Empty() && !MiKTeX::Util::PathNameUtil::IsDirectoryDelimiter(lpsz[0]))
    {
      AppendDirectoryDelimiter();
    }
    Base::Append(lpsz);
    return *this;
  }

public:
  PathName& Append(const std::string& str, bool appendDirectoryDelimiter)
  {
    return Append(str.c_str(), appendDirectoryDelimiter);
  }

  /// Appends a path name component to this path name.
  /// @param lpszComponent The null-terminated component to add.
  /// @return Returns a reference to this object.
public:
  PathName& AppendComponent(const char* component)
  {
    return Append(component, true);
  }

  /// Appends a path name component to this path name.
  /// @param lpszComponent The null-terminated component to add.
  /// @return Returns a reference to this object.
public:
  PathName& operator/=(const char* component)
  {
    return AppendComponent(component);
  }

  /// Appends a path name component to this path name.
  /// @param component The component to be appended.
  /// @return Returns a reference to this object.
public:
  PathName& operator/=(const PathName& component)
  {
    return AppendComponent(component.GetData());
  }

  /// Appends a path name component to this path name.
  /// @param component The component to be appended.
  /// @return Returns a reference to this object.
public:
  PathName& operator/=(const std::string& component)
  {
    return AppendComponent(component.c_str());
  }

  /// Cuts off the last component from the path name.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName&) CutOffLastComponent(bool allowSelfCutting);

  /// Cuts off the last component from the path name.
  /// @return Returns a reference to this object.
public:
  PathName& CutOffLastComponent()
  {
    return CutOffLastComponent(false);
  }

  /// Makes sure that this path name ends with a directory delimiter.
  /// @return Returns a reference to this object.
public:
  MIKTEXCORETHISAPI(PathName&) AppendDirectoryDelimiter();

#if defined(MIKTEX_WINDOWS)
public:
  MIKTEXCORETHISAPI(PathName&) AppendAltDirectoryDelimiter();
#endif

public:
  bool IsExplicitlyRelative() const
  {
    return MiKTeX::Util::PathNameUtil::IsExplicitlyRelative(ToString());
  }

public:
  /// Compares the prefixes of two path names.
  /// @param lpszPath1 The first path name.
  /// @param lpszPath1 The second path name.
  /// @param count Number of characters to compare.
  /// @return Returns -1, if the first prefix compares lesser than the second.
  /// Returns 0, if both prefixes compare equal.
  /// Returns 1, if the first prefix compares greater than the second.
  static MIKTEXCORECEEAPI(int) Compare(const char* lpszPath1, const char* lpszPath2, std::size_t count);

public:
  static int Compare(const PathName& path1, const PathName& path2, std::size_t count)
  {
    return Compare(path1.GetData(), path2.GetData(), count);
  }

  /// Compares two path names.
  /// @param lpszPath1 The first path name.
  /// @param lpszPath1 The second path name.
  /// @return Returns -1, if the first path name compares
  /// lesser than the second.
  /// Returns 0, if both path names compare equal.
  /// Returns 1, if the first path name compares greater than the second.
public:
  static MIKTEXCORECEEAPI(int) Compare(const char* lpszPath1, const char* lpszPath2);

  /// Compares two path names.
  /// @param path1 The first path name.
  /// @param path1 The second path name.
  /// @return Returns -1, if the first path name compares
  /// lesser than the second.
  /// Returns 0, if both path names compare equal.
  /// Returns 1, if the first path name compares greater than the second.
public:
  static int Compare(const PathName& path1, const PathName& path2)
  {
    return Compare(path1.GetData(), path2.GetData());
  }

  /// Compares two path names.
  /// @param path1 The first path name.
  /// @param path1 The second path name.
  /// @return Returns -1, if the first path name compares
  /// lesser than the second.
  /// Returns 0, if both path names compare equal.
  /// Returns 1, if the first path name compares greater than the second.
public:
  static int Compare(const std::string& path1, const std::string& path2)
  {
    return Compare(path1.c_str(), path2.c_str());
  }
  
public:
  static bool Equals(const PathName& path1, const PathName& path2)
  {
    return Compare(path1, path2) == 0;
  }

  /// Matches a path name against a wildcard pattern.
  /// @param lpszPattern The wildcard pattern.
  /// @param lpszPath The path name to test.
  /// @return Returns true, if the pattern matches.
public:
  static MIKTEXCORECEEAPI(bool) Match(const char* lpszPattern, const char* lpszPath);

public:
  static bool Match(const char* lpszPattern, const PathName& path)
  {
    return Match(lpszPattern, path.GetData());
  }
};

inline bool operator<(const PathName& lhs, const PathName& rhs)
{
  return PathName::Compare(lhs, rhs) < 0;
}

/// Compares two path names.
/// @param lhs The first path name.
/// @param rhs The second path name.
/// @return Returns true, if both path names compare equal.
inline bool operator==(const PathName& lhs, const PathName& rhs)
{
  return PathName::Compare(lhs, rhs) == 0;
}

/// Compares two path names.
/// @param lhs The first path name.
/// @param rhs The second path name.
/// @return Returns true, if both path names compare unequal.
inline bool operator!=(const PathName& lhs, const PathName& rhs)
{
  return PathName::Compare(lhs, rhs) != 0;
}

inline PathName operator/(const PathName& lhs, const PathName& rhs)
{
  PathName result = lhs;
  result /= rhs;
  return result;
}

inline std::ostream& operator<<(std::ostream& os, const PathName& path)
{
  return os << path.ToDisplayString();
}

MIKTEX_CORE_END_NAMESPACE;

namespace std
{
  template<> struct hash<MiKTeX::Core::PathName>
  {
    std::size_t operator()(const MiKTeX::Core::PathName& path) const
    {
      return path.GetHash();
    }
  };
}

#endif
