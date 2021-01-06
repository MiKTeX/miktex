/* PathName.cpp: path name utilities

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

#if defined(MIKTEX_WINDOWS)
#include <direct.h>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#  define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#  define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/PathName.h"
#include "miktex/Util/PathNameParser.h"

#include "internal.h"

constexpr auto CURRENT_DIRECTORY = ".";
constexpr auto PARENT_DIRECTORY = "..";

using namespace std;

using namespace MiKTeX::Util;

int PathName::Compare(const char* lpszPath1, const char* lpszPath2)
{
#if defined(MIKTEX_WINDOWS)
  PathName path1(lpszPath1);
  path1.TransformForComparison();
  lpszPath1 = path1.GetData();
  PathName path2(lpszPath2);
  path2.TransformForComparison();
  lpszPath2 = path2.GetData();
#endif

  int ret;
  int cmp;

  while ((cmp = *lpszPath1 - *lpszPath2) == 0 && *lpszPath1 != 0)
  {
    ++lpszPath1;
    ++lpszPath2;
  }

  if (cmp != 0)
  {
    if (
        (*lpszPath1 == 0 && PathNameUtil::IsDirectoryDelimiter(*lpszPath2) && *(lpszPath2 + 1) == 0)
        || (*lpszPath2 == 0 && PathNameUtil::IsDirectoryDelimiter(*lpszPath1) && *(lpszPath1 + 1) == 0))
    {
      return 0;
    }
  }

  if (cmp < 0)
  {
    ret = -1;
  }
  else if (cmp > 0)
  {
    ret = 1;
  }
  else
  {
    ret = 0;
  }

  return ret;
}

int PathName::Compare(const char* lpszPath1, const char* lpszPath2, size_t count)
{
  if (count == 0)
  {
    return 0;
  }

#if defined(MIKTEX_WINDOWS)
  PathName path1(lpszPath1);
  path1.TransformForComparison();
  lpszPath1 = path1.GetData();
  PathName path2(lpszPath2);
  path2.TransformForComparison();
  lpszPath2 = path2.GetData();
#endif

  for (size_t i = 0; i < count; ++i, ++lpszPath1, ++lpszPath2)
  {
    if (*lpszPath1 == 0 || *lpszPath1 != *lpszPath2)
    {
      return *reinterpret_cast<const unsigned char*>(lpszPath1) - *reinterpret_cast<const unsigned char*>(lpszPath2);
    }
  }

  return 0;
}

PathName GetFullyQualifiedPath(const char* lpszPath)
{
  PathName path;

  if (!PathNameUtil::IsFullyQualifiedPath(lpszPath))
  {
#if defined(MIKTEX_WINDOWS)
    if (PathNameUtil::IsDosDriveLetter(lpszPath[0]) && PathNameUtil::IsDosVolumeDelimiter(lpszPath[1]) && lpszPath[2] == 0)
    {
      path = lpszPath;
      path += PathNameUtil::DirectoryDelimiter;
      return path;
    }
    if (PathNameUtil::IsDirectoryDelimiter(lpszPath[0]))
    {
      int currentDrive = _getdrive();
      if (currentDrive == 0)
      {
        // TODO
        throw Exception("unexpected");
      }
      // EXPECT: currentDrive >= 1 && currentDrive <= 26
      char currentDriveLetter = 'A' + currentDrive - 1;
      path = fmt::format("{0}{1}", currentDriveLetter, PathNameUtil::DosVolumeDelimiter);
    }
    else
    {
      path.SetToCurrentDirectory();
    }
#else
    path.SetToCurrentDirectory();
#endif
  }

  PathName fixme(lpszPath);
  for (PathNameParser parser(fixme); parser; ++parser)
  {
    if (PathName::Compare(PathName(*parser), PathName(PARENT_DIRECTORY)) == 0)
    {
      path.CutOffLastComponent();
    }
    else if (PathName::Compare(PathName(*parser), PathName(CURRENT_DIRECTORY)) != 0)
    {
      path /= *parser;
    }
  }

  return path;
}

PathName& PathName::Convert(ConvertPathNameOptions options)
{
  bool toUnix = options[ConvertPathNameOption::ToUnix];
  bool toDos = options[ConvertPathNameOption::ToDos];

  bool toUpper = options[ConvertPathNameOption::MakeUpper];
  bool toLower = options[ConvertPathNameOption::MakeLower];

  bool makeFQ = options[ConvertPathNameOption::MakeFullyQualified];

#if defined(MIKTEX_WINDOWS)
  bool toExtendedLengthPathName = options[ConvertPathNameOption::ToExtendedLengthPathName];
#endif

  if (makeFQ)
  {
    *this = GetFullyQualifiedPath(GetData());
  }

#if defined(MIKTEX_WINDOWS)
  if (toExtendedLengthPathName)
  {
    *this = PathNameUtil::ToLengthExtendedPathName(ToString());
  }
#endif

  if (options[ConvertPathNameOption::Canonicalize])
  {
    Helpers::CanonicalizePathName(*this);
  }

  if (toUnix)
  {
    string s = this->ToString();
    PathNameUtil::ConvertToUnix(s);
    *this = s;
  } else if (toDos)
  {
    string s = this->ToString();
    PathNameUtil::ConvertToDos(s);
    *this = s;
  }

  if (toUpper || toLower)
  {
    if (Helpers::IsPureAscii(GetData()))
    {
      for (char* lpsz = GetData(); *lpsz != 0; ++lpsz)
      {
        *lpsz = toUpper ? Helpers::ToUpperAscii(*lpsz) : Helpers::ToLowerAscii(*lpsz);
      }
    }
    else
    {
      CharBuffer<wchar_t> wideCharBuffer(GetData());
      locale defaultLocale;
      for (wchar_t* lpsz = wideCharBuffer.GetData(); *lpsz != 0; ++lpsz)
      {
        *lpsz = toUpper ? Helpers::ToUpper(*lpsz, defaultLocale) : Helpers::ToLower(*lpsz, defaultLocale);
      }
      *this = wideCharBuffer.GetData();
    }
  }

  return *this;
}

static bool InternalMatch(const char* lpszPattern, const char* lpszPath)
{
  switch (*lpszPattern)
  {
  case 0:
    return *lpszPath == 0;

  case '*':
    // RECURSION
    return InternalMatch(lpszPattern + 1, lpszPath) || (*lpszPath != 0 && InternalMatch(lpszPattern, lpszPath + 1));

  case '?':
    // RECURSION
    return *lpszPath != 0 && InternalMatch(lpszPattern + 1, lpszPath + 1);

  default:
    // RECURSION
    return *lpszPattern == *lpszPath && InternalMatch(lpszPattern + 1, lpszPath + 1);
  }
}

bool PathName::Match(const char* lpszPattern, const char* lpszPath)
{
  return InternalMatch(PathName(lpszPattern).TransformForComparison().GetData(), PathName(lpszPath).TransformForComparison().GetData());
}

vector<string> PathName::Split(const PathName& path)
{
  vector<string> result;
  for (PathNameParser parser(path); parser; ++parser)
  {
    result.push_back(*parser);
  }
  return result;
}

void PathName::Split(const PathName& path, string& directory, string& fileNameWithoutExtension, string& extension)
{
  const char* lpsz;

  const char* lpszName_ = nullptr;

  // find the beginning of the name
  for (lpsz = path.GetData(); *lpsz != 0; ++lpsz)
  {
    if (PathNameUtil::IsDirectoryDelimiter(*lpsz))
    {
      lpszName_ = lpsz + 1;
    }
  }
  if (lpszName_ == nullptr)
  {
    lpszName_ = path.GetData();
  }

  directory.assign(path.GetData(), lpszName_ - path.GetData());

  // find the extension
  const char* lpszExtension_ = nullptr;
  for (lpsz = lpszName_; *lpsz != 0; ++lpsz)
  {
    if (*lpsz == '.')
    {
      lpszExtension_ = lpsz;
    }
  }
  if (lpszExtension_ == nullptr)
  {
    lpszExtension_ = lpsz;
  }

  fileNameWithoutExtension.assign(lpszName_, lpszExtension_ - lpszName_);

  extension = lpszExtension_;
}

string PathName::GetExtension() const
{
  const char* e = Helpers::GetFileNameExtension(GetData());
  return e == nullptr ? string() : string(e);
}

PathName& PathName::SetExtension(const char* extension, bool override)
{
  string directory;
  string fileNameWithoutExtension;
  string oldExtension;

  Split(*this, directory, fileNameWithoutExtension, oldExtension);

  if (oldExtension.empty() || override)
  {
    *this = directory;
    AppendComponent(fileNameWithoutExtension.c_str());
    if (extension != nullptr && *extension != 0)
    {
      size_t n = GetLength();
      if (*extension != '.')
      {
        if (n + 1 >= GetCapacity())
        {
          throw Unexpected("buf too small");
        }
        (*this)[n] = '.';
        ++n;
      }
      n += StringUtil::CopyString(&(*this)[n], GetCapacity() - n, extension);
    }
  }

  return *this;
}

PathName& PathName::AppendDirectoryDelimiter()
{
  size_t l = GetLength();
  if (l == 0 || !PathNameUtil::IsDirectoryDelimiter(Base::operator[](l - 1)))
  {
    Base::Append(PathNameUtil::DirectoryDelimiter);
  }
  return *this;
}

// TODO: code review
PathName& PathName::CutOffLastComponent(bool allowSelfCutting)
{
  Helpers::RemoveDirectoryDelimiter(GetData());
  bool noCut = true;
  for (size_t end = GetLength(); noCut && end > 0; --end)
  {
    if (end > 0 && PathNameUtil::IsDirectoryDelimiter(Base::operator[](end - 1)))
    {
#if defined(MIKTEX_WINDOWS)
      if (end > 1 && Base::operator[](end - 2) == PathNameUtil::DosVolumeDelimiter)
      {
        Base::operator[](end) = 0;
      }
      else
#endif
        if (end == 1)
        {
          Base::operator[](1) = 0;
        }
        else
        {
          while (end > 0 && PathNameUtil::IsDirectoryDelimiter(Base::operator[](end - 1)))
          {
            --end;
            Base::operator[](end) = 0;
          }
        }
      noCut = false;
    }
  }
  if (noCut && allowSelfCutting)
  {
    Base::operator[](0) = 0;
  }
  return *this;
}

size_t PathName::GetHash() const
{
  // see http://www.isthe.com/chongo/tech/comp/fnv/index.html
#if defined(_M_AMD64) || defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  const size_t FNV_prime = 1099511628211;
  const size_t offset_basis = 14695981039346656037ull;
#else
  const size_t FNV_prime = 16777619;
  const size_t offset_basis = 2166136261;
#endif
  size_t hash = offset_basis;
  for (const char* lpsz = GetData(); *lpsz != 0; ++lpsz)
  {
    char ch = *lpsz;
#if defined(MIKTEX_WINDOWS)
    if (ch == PathNameUtil::DirectoryDelimiter)
    {
      ch = PathNameUtil::AltDirectoryDelimiter;
    }
    else if (static_cast<unsigned>(ch) >= 128)
    {
      // ignore UTF-8 chars
      continue;
    }
    else if (ch >= 'A' && ch <= 'Z')
    {
      ch = ch - 'A' + 'a';
    }
#endif
    hash ^= (size_t)ch;
    hash *= FNV_prime;
  }
  return hash;
}

string PathName::ToDisplayString(DisplayPathNameOptions options) const
{
#if defined(MIKTEX_WINDOWS)
  return ToDos().ToString();
#else
  return ToString();
#endif
}

PathName& PathName::SetToHomeDirectory()
{
  *this = Helpers::GetHomeDirectory();
  return *this;
}

PathName& PathName::SetToLockDirectory()
{
  *this = Helpers::GetHomeDirectory();
  return *this;
}

PathName& PathName::SetToTempFile()
{
  PathName tmpDir;
  tmpDir.SetToTempDirectory();
  return SetToTempFile(tmpDir);
}
