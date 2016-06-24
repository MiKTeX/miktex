/* util.cpp: generi utilities

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

#include "miktex/Core/CSVList.h"
#include "miktex/Core/PathName.h"
#include "miktex/Core/PathNameParser.h"

#include "Session/SessionImpl.h"
#include "inliners.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

MIKTEXINTERNALFUNC(void) CopyString2(char * lpszBuf, size_t bufSize, const char * lpszSource, size_t count)
{
  MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  MIKTEX_ASSERT_STRING(lpszSource);

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
    TraceError(T_("CopyString2() is going to throw an exception"));
    TraceError("  bufSize=%u", static_cast<unsigned>(bufSize));
    TraceError("  count=%u", static_cast<unsigned>(count));
    TraceError("  lpszSource=%.*s...", static_cast<int>(bufSize), lpszSource);
    BUF_TOO_SMALL();
  }

  if (count == 0)
  {
    *lpsz = 0;
  }
}

string Utils::Hexify(const void * pv, size_t nBytes, bool lowerCase)
{
  string ret;
#define TOHEX(x) ((x) < 10 ? '0' + (x) : (x) - 10 + 'A')
#define tohex(x) ((x) < 10 ? '0' + (x) : (x) - 10 + 'a')
  for (size_t i = 0; i < nBytes; ++i)
  {
    unsigned char XX = reinterpret_cast<const unsigned char *>(pv)[i];
    ret += (lowerCase ? tohex(XX >> 4) : TOHEX(XX >> 4));
    ret += (lowerCase ? tohex(XX & 15) : TOHEX(XX & 15));
  }
#undef tohex
#undef TOHEX
  return ret;
}

string Utils::Hexify(const void * pv, size_t nBytes)
{
  return Hexify(pv, nBytes, true);
}

bool Utils::IsUTF8(const char * lpsz, bool allowPureAscii)
{
  MIKTEX_ASSERT_STRING(lpsz);
  const unsigned char * lpsz2 = reinterpret_cast<const unsigned char *>(lpsz);
  for (; lpsz2[0] != 0; ++lpsz2)
  {
    if (((lpsz2[0] & 0xe0) == 0xc0) && ((lpsz2[1] & 0xc0) == 0x80))
    {
      return true;
    }
    else if (((lpsz2[0] & 0xf0) == 0xe0) && ((lpsz2[1] & 0xc0) == 0x80) && ((lpsz2[2] & 0xc0) == 0x80))

    {
      return true;
    }
    else if (((lpsz2[0] & 0xf8) == 0xf0) && ((lpsz2[1] & 0xc0) == 0x80) && ((lpsz2[2] & 0xc0) == 0x80) && ((lpsz2[3] & 0xc0) == 0x80))
    {
      return true;
    }
    else if (static_cast<unsigned>(lpsz2[0]) > 127)
    {
      return false;
    }
  }
  return allowPureAscii;
}

MIKTEXINTERNALFUNC(const char *) GetShortSourceFile(const char * lpszSourceFile)
{
  const char * lpszShortSourceFile = 0;
  if (Utils::IsAbsolutePath(lpszSourceFile))
  {
    lpszShortSourceFile = Utils::GetRelativizedPath(lpszSourceFile, MIKTEX_SOURCE_DIR);
    if (lpszShortSourceFile == nullptr)
    {
      lpszShortSourceFile = Utils::GetRelativizedPath(lpszSourceFile, MIKTEX_BINARY_DIR);
    }
  }
  if (lpszShortSourceFile == nullptr)
  {
    lpszShortSourceFile = lpszSourceFile;
  }
  return lpszShortSourceFile;
}

inline int StrNCmpI(const char * lpsz1, const char * lpsz2, size_t n)
{
#if defined(_MSC_VER)
  return _strnicmp(lpsz1, lpsz2, n);
#else
  return strncasecmp(lpsz1, lpsz2, n);
#endif
}

/* _________________________________________________________________________

   Utils::GetRelativizedPath

   Return the part of PATH that is relative to ROOT.

   lpszPath lpszRoot return
   --------                     --------                ------
   C:\ab\cd\ef.gh C:\ab\                  cd\ef.gh
   C:\ab\cd\ef.gh C:\ab cd\ef.gh
   C:\abcd\ef.gh C:\ab\                  <NULL>
   C:\abcd\ef.gh C:\ab                   <NULL>
   C:\ef.gh C:\                     ef.gh
   _________________________________________________________________________ */

const char * Utils::GetRelativizedPath(const char * lpszPath, const char * lpszRoot)
{
  MIKTEX_ASSERT_STRING(lpszPath);
  MIKTEX_ASSERT_STRING(lpszRoot);

  MIKTEX_ASSERT(Utils::IsAbsolutePath(lpszPath));
  MIKTEX_ASSERT(Utils::IsAbsolutePath(lpszRoot));

  PathName pathNorm(lpszPath);
  pathNorm.TransformForComparison();

  PathName pathRootNorm(lpszRoot);
  pathRootNorm.TransformForComparison();

  size_t rootLen = strlen(lpszRoot);

  MIKTEX_ASSERT(rootLen > 0);

#if defined(MIKTEX_WINDOWS)
  int cmp = StrNCmpI(pathNorm.Get(), pathRootNorm.Get(), rootLen);
#else
  int cmp = strncmp(pathNorm.Get(), pathRootNorm.Get(), rootLen);
#endif
  if (cmp != 0)
  {
    return nullptr;
  }

  // special case: both paths are equal
  if (rootLen == strlen(lpszPath))
  {
#if 1
    return lpszPath + rootLen;
#else
    return ".";
#endif
  }

  int ch = lpszRoot[rootLen - 1];

  if (IsDirectoryDelimiter(ch))
  {
    return lpszPath + rootLen;
  }

  if (!IsDirectoryDelimiter(lpszPath[rootLen]))
  {
    return nullptr;
  }

  return lpszPath + rootLen + 1;
}

bool Utils::IsAbsolutePath(const char * lpszPath)
{
  MIKTEX_ASSERT_STRING(lpszPath);

  // "\xyz\foo.txt", "\\server\xyz\foo.txt"
  if (IsDirectoryDelimiter(lpszPath[0]))
  {
    return true;
  }
#if defined(MIKTEX_WINDOWS)
  else if (IsDriveLetter(lpszPath[0]) // "C:\xyz\foo.txt"
    && lpszPath[1] == ':'
    && IsDirectoryDelimiter(lpszPath[2]))
  {
    return true;
  }
#endif
  else
  {
    return false;
  }
}

static const char * const forbiddenFileNames[] = {
#if defined(MIKTEX_WINDOWS)
  "desktop.ini", "folder.htt",
#endif
  nullptr,
};

static const char * const forbiddenExtensions[] = {
#if defined(MIKTEX_WINDOWS)
  ".bat", ".cmd", ".com", ".exe", ".js", ".jse", ".lnk", ".msc", ".pif", ".reg", ".scr", ".pl"
  ".py", ".pyw", ".tcl", ".url", ".vbe", ".vbs", ".ws", ".wsf", ".wsh",
#endif
  nullptr
};

bool Utils::IsSafeFileName(const char * lpszPath, bool forInput)
{
  if (forInput)
  {
    return true;
  }
  if (IsAbsolutePath(lpszPath))
  {
    return false;
  }
  PathName fileName;
  for (PathNameParser parser(lpszPath); parser.GetCurrent() != nullptr; ++parser)
  {
    if (PathName::Compare(parser.GetCurrent(), PARENT_DIRECTORY) == 0)
    {
      return false;
    }
    fileName = parser.GetCurrent();
  }
  MIKTEX_ASSERT(!fileName.Empty());
  for (int idx = 0; forbiddenFileNames[idx] != nullptr; ++idx)
  {
    if (PathName::Compare(forbiddenFileNames[idx], fileName) == 0)
    {
      return false;
    }
  }
#if defined(MIKTEX_WINDOWS)
  const char * lpszExtension = fileName.GetExtension();
  string forbiddenExtensions;
  if (lpszExtension != nullptr && ::GetEnvironmentString("PATHEXT", forbiddenExtensions))
  {
    for (CSVList ext(forbiddenExtensions, PATH_DELIMITER); ext.GetCurrent() != nullptr; ++ext)
    {
      if (PathName::Compare(ext.GetCurrent(), lpszExtension) == 0)
      {
	return false;
      }
    }
  }
#endif
  return true;
}

void Utils::MakeTeXPathName(PathName & path)
{
#if defined(MIKTEX_WINDOWS)
  path.Convert({ ConvertPathNameOption::RemoveBlanks, ConvertPathNameOption::ToUnix });
#else
  #  warning Unimplemented : Utils::MakeTeXPathName()
    if (StrChr(path.Get(), ' ') != 0)
    {
      MIKTEX_FATAL_ERROR_2(T_("Path name contains the space character."), "path", path.ToString());
    }
#endif
}

bool Utils::IsParentDirectoryOf(const char * lpszParentDir, const char * lpszFileName)
{
  size_t len1 = strlen(lpszParentDir);
  if (PathName::Compare(lpszParentDir, lpszFileName, len1) != 0)
  {
    return false;
  }
  size_t len2 = strlen(lpszFileName);
  if (len1 >= len2)
  {
    return false;
  }
#if defined(MIKTEX_WINDOWS)
  if (len1 == 3)                // C:/
  {
    return true;
  }
#endif
  return IsDirectoryDelimiter(lpszFileName[len1]);
}

bool Utils::GetUncRootFromPath(const char * lpszPath, PathName & uncRoot)
{
  // must start with "\\"
  if (!(IsDirectoryDelimiter(lpszPath[0]) && IsDirectoryDelimiter(lpszPath[1])))
  {
    return false;
  }

  uncRoot = lpszPath;

  char * lpsz = uncRoot.GetData() + 2;

  if (lpsz[0] == 0 || lpsz[1] == 0)
  {
    return false;
  }

  // skip server name
  while (*++lpsz != 0)
  {
    if (IsDirectoryDelimiter(*lpsz))
    {
      break;
    }
  }

  if (lpsz[0] == 0 || lpsz[1] == 0)
  {
    return false;
  }

  // skip share name
  while (*++lpsz != 0)
  {
    if (IsDirectoryDelimiter(*lpsz))
    {
      break;
    }
  }

  if (!(*lpsz == 0 || IsDirectoryDelimiter(*lpsz)))
  {
    return false;
  }

  *lpsz++ = PathName::DirectoryDelimiter;
  *lpsz = 0;

#if defined(MIKTEX_WINDOWS)
  uncRoot.ToDos();
#endif

  return true;
}

bool Utils::GetPathNamePrefix(const PathName & path, const PathName & suffix, PathName & prefix)
{
  MIKTEX_ASSERT(!Utils::IsAbsolutePath(suffix.Get()));

  PathName path_(path);
  PathName suffix_(suffix);

  while (!suffix_.Empty())
  {
    char p[BufferSizes::MaxPath];
    char s[BufferSizes::MaxPath];
    if (PathName::Compare(path_.GetFileName(p), suffix_.GetFileName(s)) != 0)
    {
      return false;
    }
    path_.CutOffLastComponent(true);
    suffix_.CutOffLastComponent(true);
  }

  prefix = path_;

  return true;
}

MIKTEXINTERNALFUNC(void) AppendDirectoryDelimiter(string & path)
{
  size_t l = path.length();
  if (l > 0 && !IsDirectoryDelimiter(path[l - 1]))
  {
    path += PathName::DirectoryDelimiter;
  }
}

MIKTEXINTERNALFUNC(void) AppendDirectoryDelimiter(char * lpszPath, size_t size)
{
  MIKTEX_ASSERT(size > 0);
  MIKTEX_ASSERT_STRING(lpszPath);
  MIKTEX_ASSERT_CHAR_BUFFER(lpszPath, size);
  size_t l = strlen(lpszPath);
  MIKTEX_ASSERT(l < size);
  if (l > 0 && !IsDirectoryDelimiter(lpszPath[l - 1]))
  {
    if (l + 1 >= size)
    {
      INVALID_ARGUMENT("path", lpszPath);
    }
    lpszPath[l] = PathName::DirectoryDelimiter;
    lpszPath[l + 1] = 0;
  }
}

MIKTEXINTERNALFUNC(void) RemoveDirectoryDelimiter(char * lpszPath)
{
  size_t l = strlen(lpszPath);
  if (l > 1 && IsDirectoryDelimiter(lpszPath[l - 1]))
  {
#if defined(MIKTEX_WINDOWS)
    if (lpszPath[l - 2] == PathName::VolumeDelimiter)
    {
      return;
    }
#endif
    lpszPath[l - 1] = 0;
  }
}

MIKTEXINTERNALFUNC(const char *) GetFileNameExtension(const char * lpszPath)
{
  const char * lpszExtension = nullptr;
  for (const char * lpsz = lpszPath; *lpsz != 0; ++lpsz)
  {
    if (IsDirectoryDelimiter(*lpsz))
    {
      lpszExtension = nullptr;
    }
    else if (*lpsz == '.')
    {
      lpszExtension = lpsz;
    }
  }
  return lpszExtension;
}

MIKTEXINTERNALFUNC(bool) IsExplicitlyRelativePath(const char * lpszPath)
{
  if (lpszPath[0] == '.')
  {
    return IsDirectoryDelimiter(lpszPath[1]) || (lpszPath[1] == '.' && IsDirectoryDelimiter(lpszPath[2]));
  }
  else
  {
    return false;
  }
}

MIKTEXINTERNALFUNC(PathName) GetFullPath(const char * lpszPath)
{
  PathName path;

  if (!Utils::IsAbsolutePath(lpszPath))
  {
#if defined(MIKTEX_WINDOWS)
    if (IsDriveLetter(lpszPath[0]) && lpszPath[1] == ':' && lpszPath[2] == 0)
    {
      path = lpszPath;
      path /= PathName::DirectoryDelimiter;
      return path;
    }
#endif
    path.SetToCurrentDirectory();
  }

  for (PathNameParser parser(lpszPath); parser.GetCurrent() != 0; ++parser)
  {
    if (PathName::Compare(parser.GetCurrent(), PARENT_DIRECTORY) == 0)
    {
      path.CutOffLastComponent();
    }
    else if (PathName::Compare(parser.GetCurrent(), CURRENT_DIRECTORY) != 0)
    {
      path.AppendComponent(parser.GetCurrent());
    }
  }

  return path;
}

void Utils::PrintException(const exception & e)
{
  if (cerr.fail())
  {
    return;
  }
  try
  {
    cerr << "*** " << e.what() << endl;
  }
  catch (const exception &)
  {
  }
}

void Utils::PrintException(const MiKTeXException & e)
{
  if (cerr.fail())
  {
    return;
  }
  try
  {
    string programInvocationName(e.GetProgramInvocationName());
    char szName[BufferSizes::MaxPath];
    bool haveName = (programInvocationName.length() > 0);
    if (haveName)
    {
      PathName path(programInvocationName);
      path.GetFileName(szName);
    }
    int last = '\n';
    for (const char * lpsz = e.what(); *lpsz != 0; ++lpsz)
    {
      if (haveName && last == '\n')
      {
	cerr << szName << ": ";
      }
      cerr << *lpsz;
      last = *lpsz;
    }
    if (last != '\n')
    {
      cerr << endl;
      last = '\n';
    }
    string info = e.GetInfo();
    for (const char * lpsz = info.c_str(); *lpsz != 0; ++lpsz)
    {
      if (haveName && last == '\n')
      {
	cerr << szName << ": ";
      }
      if (last == '\n')
      {
	cerr << "Data: ";
      }
      cerr << *lpsz;
      last = *lpsz;
    }
    if (last != '\n')
    {
      cerr << endl;
      last = '\n';
    }
  }
  catch (const exception &)
  {
  }
}

string Utils::GetExeName()
{
  char szName[BufferSizes::MaxPath];
  SessionImpl::GetSession()->GetMyProgramFile(false).GetFileNameWithoutExtension(szName);
  return szName;
}

#if !HAVE_MIKTEX_USER_INFO
bool Utils::IsRegisteredMiKTeXUser()
{
  return false;
}
#endif

#if !HAVE_MIKTEX_USER_INFO
void Utils::RegisterMiKTeXUser()
{
  ShowWebPage(MIKTEX_URL_WWW_GIVE_BACK);
}
#endif

string Utils::GetMiKTeXVersionString()
{
  return MIKTEX_VERSION_STR;
}

string Utils::GetMiKTeXBannerString()
{
  return MIKTEX_BANNER_STR;
}

string Utils::MakeProgramVersionString(const char * lpszProgramName, const VersionNumber & programVersionNumber)
{
  string str = lpszProgramName;
  if (string(MIKTEX_BANNER_STR).find(programVersionNumber.ToString()) == string::npos)
  {
    str += ' ';
    str += programVersionNumber.ToString();
  }
  str += " (" MIKTEX_BANNER_STR ")";
  return str;
}

bool Utils::GetEnvironmentString(const char * lpszName, string & str)
{
  bool haveValue = ::GetEnvironmentString(lpszName, str);
  if (SessionImpl::TryGetSession() != 0
    && SessionImpl::GetSession()->trace_env.get() != 0
    && SessionImpl::GetSession()->trace_env->IsEnabled())
  {
    SessionImpl::GetSession()->trace_env->WriteFormattedLine("core", "%s => %s", lpszName, (haveValue ? str.c_str() : "null"));
  }
  return haveValue;
}

bool Utils::GetEnvironmentString(const char * lpszName, PathName & path)
{
  return Utils::GetEnvironmentString(lpszName, path.GetData(), path.GetCapacity());
}

bool Utils::GetEnvironmentString(const char * lpszName, char * lpszOut, size_t sizeOut)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  size_t bufSize;
  if (_wgetenv_s(&bufSize, 0, 0, UW_(lpszName)) != 0)
  {
    return false;
  }
  if (bufSize == 0)
  {
    return false;
  }
  CharBuffer<wchar_t> buf(bufSize);
  if (_wgetenv_s(&bufSize, buf.GetData(), bufSize, UW_(lpszName)) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR_2("_wgetenv_s", "name", lpszName);
  }
  StringUtil::CopyString(lpszOut, sizeOut, buf.Get());
  return true;
#else
  const char * lpsz = getenv(lpszName);
  if (lpsz == 0)
  {
    return false;
  }
  StringUtil::CopyString(lpszOut, sizeOut, lpsz);
  return true;
#endif
}

bool Utils::ParseDvipsMapLine(const string & line, FontMapEntry & mapEntry)
{
  mapEntry.texName = "";
  mapEntry.psName = "";
  mapEntry.specialInstructions = "";
  mapEntry.encFile = "";
  mapEntry.fontFile = "";
  mapEntry.headerList = "";

  if (line.empty()
    || line[0] <= ' '
    || line[0] == '*'
    || line[0] == '#'
    || line[0] == ';'
    || line[0] == '%'
    )
  {
    return false;
  }

  for (string::const_iterator it = line.begin(); it != line.end(); ++it)
  {
    for (; it != line.end() && *it <= ' '; ++it)
    {
    }
    if (it == line.end())
    {
      break;
    }
    if (*it == '"')
    {
      string temp;
      ++it;
      for (; it != line.end() && *it != '"'; ++it)
      {
	temp += *it;
      }
      for (Tokenizer tok(temp.c_str(), " \t"); tok.GetCurrent() != 0; ++tok)
      {
	if (mapEntry.specialInstructions.length() > 0)
	{
	  mapEntry.specialInstructions += ' ';
	}
	mapEntry.specialInstructions += tok.GetCurrent();
      }
      if (it == line.end())
      {
	break;
      }
    }
    else if (*it == '<')
    {
      ++it;
      if (it == line.end())
      {
	break;
      }
      bool haveEncoding = false;
      bool noPartial = false;
      if (*it == '[')
      {
	haveEncoding = true;
	++it;
      }
      else if (*it == '<')
      {
	noPartial = true;
	++it;
      }
      if (it == line.end())
      {
	break;
      }
      for (; it != line.end() && *it <= ' '; ++it)
      {
      }
      if (it == line.end())
      {
	break;
      }
      string temp;
      for (; it != line.end() && *it > ' '; ++it)
      {
	temp += *it;
      }
      if (it == line.end())
      {
	--it;
      }
      PathName fileName(temp);
      if (mapEntry.headerList.length() > 0)
      {
	mapEntry.headerList += ';';
      }
      mapEntry.headerList += '<';
      if (haveEncoding || fileName.HasExtension(".enc"))
      {
	mapEntry.encFile = fileName.Get();
	if (haveEncoding)
	{
	  mapEntry.headerList += '[';
	}
      }
      else if (fileName.HasExtension(".pfa")
	|| fileName.HasExtension(".pfb"))
      {
	mapEntry.fontFile = fileName.Get();
      }
      if (noPartial)
      {
	mapEntry.headerList += '<';
      }
      mapEntry.headerList += fileName.Get();
    }
    else
    {
      string name;
      for (; it != line.end() && *it > ' '; ++it)
      {
	name += *it;
      }
      if (mapEntry.texName.length() == 0)
      {
	mapEntry.texName = name;
      }
      else
      {
	mapEntry.psName = name;
      }
      if (it == line.end())
      {
	break;
      }
    }
  }

  return true;
}

inline int GetC(FILE * stream)
{
  int ch = getc(stream);
  if (ferror(stream) != 0)
  {
    MIKTEX_FATAL_CRT_ERROR("getc");
  }
  return ch;
}

inline void UnGetC(int ch, FILE * stream)
{
  int ch2 = ungetc(ch, stream);
  if (ch2 == EOF)
  {
    MIKTEX_FATAL_CRT_ERROR("ungetc");
  }
}

bool Utils::ReadUntilDelim(string & str, int delim, FILE * stream)
{
  if (delim == '\n')
  {
    // special case
    return ReadLine(str, stream, true);
  }
  else
  {
    str = "";
    if (feof(stream) != 0)
    {
      return false;
    }
    int ch;
    while ((ch = GetC(stream)) != EOF)
    {
      str += static_cast<char>(ch);
      if (ch == delim)
      {
	return true;
      }
    }
    return ch == EOF ? !str.empty() : true;
  }
}

bool Utils::ReadLine(string & str, FILE * stream, bool keepLineEnding)
{
  str = "";
  if (feof(stream) != 0)
  {
    return false;
  }
  int ch;
  while ((ch = GetC(stream)) != EOF)
  {
    if (ch == '\r')
    {
      if (keepLineEnding)
      {
	str += '\r';
      }
      ch = GetC(stream);
      if (ch == '\n')
      {
	if (keepLineEnding)
	{
	  str += '\n';
	}
      }
      else if (ch != EOF)
      {
	UnGetC(ch, stream);
      }
      return true;
    }
    else if (ch == '\n')
    {
      if (keepLineEnding)
      {
	str += '\n';
      }
      return true;
    }
    else
    {
      str += static_cast<char>(ch);
    }
  }
  return ch == EOF ? !str.empty() : true;
}

#if defined(MIKTEX_WINDOWS)
static const char * const pathext[] = { ".com", ".exe", ".cmd", ".bat" };
#endif

bool Utils::FindProgram(const std::string & programName, PathName & path)
{
  string envPath;
  if (!Utils::GetEnvironmentString("PATH", envPath))
  {
    return false;
  }
  for (CSVList entry(envPath, PathName::PathNameDelimiter); entry.GetCurrent() != nullptr; ++entry)
  {
    PathName cand = entry.GetCurrent();
    cand /= programName;
#if defined(MIKTEX_WINDOWS)
    if (cand.GetExtension() == nullptr)
    {
      for (const char * ext : pathext)
      {
	cand.SetExtension(ext);
	if (File::Exists(cand))
	{
	  path = cand;
	  return true;
	}
      }
    }
#endif
    if (File::Exists(cand))
    {
      path = cand;
      return true;
    }
  }
  return false;
}
