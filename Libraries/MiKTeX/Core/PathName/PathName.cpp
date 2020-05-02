/* PathName.cpp: path name utilities

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

#include "config.h"

#include <miktex/Core/PathName>
#include <miktex/Core/PathNameParser>

#include "internal.h"

#include "Utils/inliners.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

int PathName::Compare(const char* lpszPath1, const char* lpszPath2)
{
  MIKTEX_ASSERT_STRING(lpszPath1);
  MIKTEX_ASSERT_STRING(lpszPath2);

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
    MIKTEX_ASSERT(!(*lpszPath1 == 0 && *lpszPath2 == 0));
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
  MIKTEX_ASSERT_STRING(lpszPath1);
  MIKTEX_ASSERT_STRING(lpszPath2);

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

// TODO: code review
// TODO: performance
PathName& PathName::Convert(ConvertPathNameOptions options)
{
  bool toUnix = options[ConvertPathNameOption::ToUnix];
  bool toDos = options[ConvertPathNameOption::ToDos];
  MIKTEX_ASSERT(!(toUnix && toDos));

  bool toUpper = options[ConvertPathNameOption::MakeUpper];
  bool toLower = options[ConvertPathNameOption::MakeLower];
  MIKTEX_ASSERT(!(toUpper && toLower));

  MIKTEX_ASSERT(!options[ConvertPathNameOption::MakeRelative]);
  bool makeFQ = options[ConvertPathNameOption::MakeAbsolute];

#if defined(MIKTEX_WINDOWS)
  bool toLongPathName = options[ConvertPathNameOption::ToLongPathName];
  bool toExtendedLengthPathName = options[ConvertPathNameOption::ToExtendedLengthPathName];
#endif

  if (makeFQ)
  {
    PathName temp = GetFullPath(GetData());
    *this = temp;
  }

#if defined(MIKTEX_WINDOWS)
  if (toLongPathName)
  {
    CharBuffer<wchar_t, BufferSizes::MaxPath> longPathName;
    bool done = false;
    int rounds = 0;
    do
    {
      DWORD n = GetLongPathNameW(this->ToExtendedLengthPathName().ToWideCharString().c_str(), longPathName.GetData(), longPathName.GetCapacity());
      if (n == 0)
      {
        MIKTEX_FATAL_WINDOWS_ERROR_2("GetLongPathNameW", "path", this->ToString());
      }
      done = n < longPathName.GetCapacity();
      if (!done)
      {
        if (rounds > 0)
        {
          BUF_TOO_SMALL();
        }
        longPathName.Reserve(n);
      }
      rounds++;
    } while (!done);
    *this = longPathName.GetData();
  }

  if (toExtendedLengthPathName)
  {
    *this = PathNameUtil::ToLengthExtendedPathName(ToString());
  }
#endif

  if (options[ConvertPathNameOption::Canonicalize])
  {
    Utils::CanonicalizePathName(*this);
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
    if (Utils::IsPureAscii(GetData()))
    {
      for (char* lpsz = GetData(); *lpsz != 0; ++lpsz)
      {
        *lpsz = toUpper ? ToUpper(*lpsz) : ToLower(*lpsz);
      }
    }
    else
    {
      CharBuffer<wchar_t> wideCharBuffer(GetData());
      for (wchar_t* lpsz = wideCharBuffer.GetData(); *lpsz != 0; ++lpsz)
      {
        *lpsz = toUpper ? ToUpper(*lpsz) : ToLower(*lpsz);
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
  MIKTEX_ASSERT_STRING(lpszPath);
  MIKTEX_ASSERT_STRING(lpszPattern);
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
  const char* e = GetFileNameExtension(GetData());
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
          BUF_TOO_SMALL();
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
  RemoveDirectoryDelimiter(GetData());
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
  MIKTEX_ASSERT(sizeof(size_t) == 8);
  const size_t FNV_prime = 1099511628211;
  const size_t offset_basis = 14695981039346656037ull;
#else
  MIKTEX_ASSERT(sizeof(size_t) == 4);
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
  *this = GetHomeDirectory();
  return *this;
}

PathName& PathName::SetToLockDirectory()
{
  *this = GetHomeDirectory();
  return *this;
}

PathName& PathName::SetToTempFile()
{
  shared_ptr<SessionImpl> session = SessionImpl::TryGetSession();
  PathName tmpDir;
  if (session != nullptr)
  {
    tmpDir = session->GetTempDirectory();
  }
  else
  {
    tmpDir.SetToTempDirectory();
  }
  return SetToTempFile(tmpDir);
}
