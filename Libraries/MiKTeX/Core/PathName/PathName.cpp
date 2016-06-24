/* PathName.cpp: path name utilities

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

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/PathName.h"

#include "Utils/inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

int PathName::Compare(const char * lpszPath1, const char * lpszPath2)
{
  MIKTEX_ASSERT_STRING(lpszPath1);
  MIKTEX_ASSERT_STRING(lpszPath2);

#if defined(MIKTEX_WINDOWS)
  PathName path1(lpszPath1);
  path1.TransformForComparison();
  lpszPath1 = path1.Get();
  PathName path2(lpszPath2);
  path2.TransformForComparison();
  lpszPath2 = path2.Get();
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
	(*lpszPath1 == 0 && IsDirectoryDelimiter(*lpszPath2) && *(lpszPath2 + 1) == 0)
	|| (*lpszPath2 == 0 && IsDirectoryDelimiter(*lpszPath1) && *(lpszPath1 + 1) == 0))
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

int PathName::Compare(const char * lpszPath1, const char * lpszPath2, size_t count)

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
  lpszPath1 = path1.Get();
  PathName path2(lpszPath2);
  path2.TransformForComparison();
  lpszPath2 = path2.Get();
#endif

  for (size_t i = 0; i < count; ++i, ++lpszPath1, ++lpszPath2)
  {
    if (*lpszPath1 == 0 || *lpszPath1 != *lpszPath2)
    {
      return *reinterpret_cast<const unsigned char *>(lpszPath1) - *reinterpret_cast<const unsigned char *>(lpszPath2);
    }
  }

  return 0;
}

PathName & PathName::Convert(ConvertPathNameOptions options)
{
  bool toUnix = options[ConvertPathNameOption::ToUnix];
  bool toDos = options[ConvertPathNameOption::ToDos];
  MIKTEX_ASSERT(!(toUnix && toDos));

  bool toUpper = options[ConvertPathNameOption::MakeUpper];
  bool toLower = options[ConvertPathNameOption::MakeLower];
  MIKTEX_ASSERT(!(toUpper && toLower));

  bool relativize = options[ConvertPathNameOption::MakeRelative];
  UNUSED(relativize);
  bool makeFQ = options[ConvertPathNameOption::MakeAbsolute];
  MIKTEX_ASSERT(!(relativize && makeFQ));

#if defined(MIKTEX_WINDOWS)
  bool removeBlanks = options[ConvertPathNameOption::RemoveBlanks];
  bool toLongPathName = options[ConvertPathNameOption::ToLongPathName];
  MIKTEX_ASSERT(!(removeBlanks && toLongPathName));
#endif

  if (makeFQ)
  {
    PathName temp = GetFullPath(Get());
    *this = temp;
  }

#if defined(MIKTEX_WINDOWS)
  if (options[ConvertPathNameOption::RemoveBlanks])
  {
    Utils::RemoveBlanksFromPathName(*this);
  }
  if (options[ConvertPathNameOption::ToLongPathName])
  {
    wchar_t longPathName[_MAX_PATH];
    DWORD len = GetLongPathNameW(this->ToWideCharString().c_str(), longPathName, _MAX_PATH);
    if (len >= _MAX_PATH)
    {
      BUF_TOO_SMALL();
    }
    if (len == 0)
    {
      MIKTEX_FATAL_WINDOWS_ERROR_2("GetLongPathNameW", "path", this->ToString());
    }
    *this = longPathName;
  }
#endif

  if (options[ConvertPathNameOption::Canonicalize])
  {
    Utils::CanonicalizePathName(*this);
  }

  if (toUnix || toDos)
  {
    for (char * lpsz = GetData(); *lpsz != 0; ++lpsz)
    {
      if (toUnix && *lpsz == PathName::DosDirectoryDelimiter)
      {
        *lpsz = PathName::UnixDirectoryDelimiter;
      }
      else if (toDos && *lpsz == PathName::UnixDirectoryDelimiter)
      {
        *lpsz = PathName::DosDirectoryDelimiter;
      }
    }
  }

  if (toUpper || toLower)
  {
    if (Utils::IsPureAscii(Get()))
    {
      for (char * lpsz = GetData(); *lpsz != 0; ++lpsz)
      {
	*lpsz = toUpper ? ToUpper(*lpsz) : ToLower(*lpsz);
      }
    }
    else
    {
      CharBuffer<wchar_t> wideCharBuffer(Get());
      for (wchar_t * lpsz = wideCharBuffer.GetData(); *lpsz != 0; ++lpsz)
      {
	*lpsz = toUpper ? ToUpper(*lpsz) : ToLower(*lpsz);
      }
      *this = wideCharBuffer.Get();
    }
  }

  return *this;
}

static bool InternalMatch(const char * lpszPattern, const char * lpszPath)
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

bool PathName::Match(const char * lpszPattern, const char * lpszPath)
{
  MIKTEX_ASSERT_STRING(lpszPath);
  MIKTEX_ASSERT_STRING(lpszPattern);
  return InternalMatch(PathName(lpszPattern).TransformForComparison().Get(), PathName(lpszPath).TransformForComparison().Get());
}

void PathName::Combine(char * lpszPath, size_t sizePath, const char * lpszAbsPath, const char * lpszRelPath, const char * lpszExtension)
{
  MIKTEX_ASSERT_STRING_OR_NIL(lpszAbsPath);
  MIKTEX_ASSERT_STRING_OR_NIL(lpszRelPath);
  MIKTEX_ASSERT_STRING_OR_NIL(lpszExtension);

  size_t n = 0;

  if (lpszAbsPath != nullptr && *lpszAbsPath != 0)
  {
    n = StringUtil::CopyString(lpszPath, sizePath, lpszAbsPath);
  }

  if (((lpszRelPath != nullptr && *lpszRelPath != 0)
    || (lpszExtension != nullptr && *lpszExtension != 0)) && (n > 0 && !IsDirectoryDelimiter(lpszPath[n - 1])))
  {
    if (n + 1 >= sizePath)
    {
      BUF_TOO_SMALL();
    }
    lpszPath[n] = DirectoryDelimiter;
    ++n;
    lpszPath[n] = 0;
  }

  if (lpszRelPath != nullptr && *lpszRelPath != 0)
  {
    n += StringUtil::CopyString(&lpszPath[n], sizePath - n, lpszRelPath);
  }

  if (lpszExtension != nullptr && *lpszExtension != 0)
  {
    if (*lpszExtension != '.')
    {
      if (n + 1 >= sizePath)
      {
	BUF_TOO_SMALL();
      }
      lpszPath[n] = '.';
      ++n;
    }
    n += StringUtil::CopyString(&lpszPath[n], sizePath - n, lpszExtension);
  }

  MIKTEX_ASSERT(strlen(lpszPath) == n);
}

void PathName::Split(const char * lpszPath, char * lpszDir, size_t sizeDir, char * lpszName, size_t sizeName, char * lpszExtension, size_t sizeExtension)
{
  MIKTEX_ASSERT_STRING(lpszPath);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszDir, sizeDir);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszName, sizeName);
  MIKTEX_ASSERT_CHAR_BUFFER_OR_NIL(lpszExtension, sizeExtension);

  const char * lpsz;

  const char * lpszName_ = nullptr;

  // find the beginning of the name
  for (lpsz = lpszPath; *lpsz != 0; ++lpsz)
  {
    if (IsDirectoryDelimiter(*lpsz))
    {
      lpszName_ = lpsz + 1;
    }
  }
  if (lpszName_ == nullptr)
  {
    lpszName_ = lpszPath;
  }

  if (lpszDir != nullptr)
  {
    CopyString2(lpszDir, sizeDir, lpszPath, lpszName_ - lpszPath);
  }

  // find the extension
  const char * lpszExtension_ = nullptr;
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

  if (lpszName != nullptr)
  {
    CopyString2(lpszName, sizeName, lpszName_, lpszExtension_ - lpszName_);
  }

  if (lpszExtension != nullptr)
  {
    StringUtil::CopyString(lpszExtension, sizeExtension, lpszExtension_);
  }
}

const char * PathName::GetExtension() const
{
  return GetFileNameExtension(Get());
}

PathName & PathName::SetExtension(const char * lpszExtension, bool override)
{
  char szDir[BufferSizes::MaxPath];
  char szFileName[BufferSizes::MaxPath];
  char szExtOld[BufferSizes::MaxPath];

  PathName::Split(Get(), szDir, BufferSizes::MaxPath, szFileName, BufferSizes::MaxPath, szExtOld, BufferSizes::MaxPath);

  if (szExtOld[0] == 0 || override)
  {
    Set(szDir, szFileName, lpszExtension);
  }

  return *this;
}

PathName & PathName::AppendDirectoryDelimiter()
{
  size_t l = GetLength();
  if (l == 0 || !IsDirectoryDelimiter(Base::operator[](l - 1)))
  {
    Base::Append(DirectoryDelimiter);
  }
  return *this;
}

PathName & PathName::CutOffLastComponent(bool allowSelfCutting)
{
  RemoveDirectoryDelimiter(GetData());
  bool noCut = true;
  for (size_t end = GetLength(); noCut && end > 0; --end)
  {
    if (end > 0 && IsDirectoryDelimiter(Base::operator[](end - 1)))
    {
#if defined(MIKTEX_WINDOWS)
      if (end > 1 && Base::operator[](end - 2) == PathName::VolumeDelimiter)
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
	  Base::operator[](end - 1) = 0;
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
  for (const char * lpsz = GetData(); *lpsz != 0; ++lpsz)
  {
    char ch = *lpsz;
#if defined(MIKTEX_WINDOWS)
    if (ch == DirectoryDelimiter)
    {
      ch = AltDirectoryDelimiter;
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
