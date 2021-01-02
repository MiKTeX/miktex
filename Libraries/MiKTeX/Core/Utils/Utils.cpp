/* util.cpp: generi utilities

   Copyright (C) 1996-2021 Christian Schenk

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

#include <iostream>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#if defined(MIKTEX_WINDOWS)
#include <direct.h>
#endif

#include <miktex/GitInfo>

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/AutoResource>
#include <miktex/Core/CsvList>
#include <miktex/Core/Directory>
#include <miktex/Core/PathName>
#include <miktex/Core/PathNameParser>
#include <miktex/Core/Paths>
#include <miktex/Core/Utils>
#include <miktex/Util/Tokenizer>

#include "internal.h"

#include "Session/SessionImpl.h"
#include "inliners.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

string Utils::Hexify(const void* bytes, size_t nBytes, bool lowerCase)
{
  string ret;
#define TOHEX(x) ((x) < 10 ? '0' + (x) : (x) - 10 + 'A')
#define tohex(x) ((x) < 10 ? '0' + (x) : (x) - 10 + 'a')
  for (size_t i = 0; i < nBytes; ++i)
  {
    unsigned char XX = reinterpret_cast<const unsigned char*>(bytes)[i];
    ret += (lowerCase ? tohex(XX >> 4) : TOHEX(XX >> 4));
    ret += (lowerCase ? tohex(XX & 15) : TOHEX(XX & 15));
  }
#undef tohex
#undef TOHEX
  return ret;
}

string Utils::Hexify(const void* bytes, size_t nBytes)
{
  return Hexify(bytes, nBytes, true);
}

bool Utils::IsUTF8(const char* lpsz, bool allowPureAscii)
{
  MIKTEX_ASSERT_STRING(lpsz);
  const unsigned char* lpsz2 = reinterpret_cast<const unsigned char*>(lpsz);
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

MIKTEXINTERNALFUNC(const char*) GetShortSourceFile(const char* lpszSourceFile)
{
  const char* lpszShortSourceFile = 0;
  if (PathNameUtil::IsAbsolutePath(lpszSourceFile))
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

inline int StrNCmpI(const char* lpsz1, const char* lpsz2, size_t n)
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

   lpszPath                     lpszRoot                return
   --------                     --------                ------
   C:\ab\cd\ef.gh               C:\ab\                  cd\ef.gh
   C:\ab\cd\ef.gh               C:\ab                   cd\ef.gh
   C:\abcd\ef.gh                C:\ab\                  <NULL>
   C:\abcd\ef.gh                C:\ab                   <NULL>
   C:\ef.gh                     C:\                     ef.gh
   _________________________________________________________________________ */

const char* Utils::GetRelativizedPath(const char* lpszPath, const char* lpszRoot)
{
  MIKTEX_ASSERT_STRING(lpszPath);
  MIKTEX_ASSERT_STRING(lpszRoot);

  MIKTEX_ASSERT(PathNameUtil::IsAbsolutePath(lpszPath));
  MIKTEX_ASSERT(PathNameUtil::IsAbsolutePath(lpszRoot));

  PathName pathNorm(lpszPath);
  pathNorm.TransformForComparison();

  PathName pathRootNorm(lpszRoot);
  pathRootNorm.TransformForComparison();

  size_t rootLen = strlen(lpszRoot);

  MIKTEX_ASSERT(rootLen > 0);

#if defined(MIKTEX_WINDOWS)
  int cmp = StrNCmpI(pathNorm.GetData(), pathRootNorm.GetData(), rootLen);
#else
  int cmp = strncmp(pathNorm.GetData(), pathRootNorm.GetData(), rootLen);
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

  if (PathNameUtil::IsDirectoryDelimiter(ch))
  {
    return lpszPath + rootLen;
  }

  if (!PathNameUtil::IsDirectoryDelimiter(lpszPath[rootLen]))
  {
    return nullptr;
  }

  return lpszPath + rootLen + 1;
}

static vector<string> forbiddenFileNames = {
#if defined(MIKTEX_WINDOWS)
  "desktop.ini",
  "folder.htt",
#endif
};

static vector<string> allowedFileNames = {
  ".tex"
};

bool Utils::IsSafeFileName(const PathName& path)
{
  if (path.IsAbsolute())
  {
    return false;
  }
  PathName fileName;
  for (PathNameParser comp(path); comp; ++comp)
  {
    fileName = *comp;
    if (fileName.GetLength() > 1 && fileName[0] == '.' && std::find(allowedFileNames.begin(), allowedFileNames.end(), fileName.ToString()) == allowedFileNames.end())
    {
      return false;
    }    
  }
  MIKTEX_ASSERT(!fileName.Empty());
  for (const string& forbidden : forbiddenFileNames)
  {
    if (PathName::Compare(PathName(forbidden), fileName) == 0)
    {
      return false;
    }
  }
#if defined(MIKTEX_WINDOWS)
  string extension = fileName.GetExtension();
  string forbiddenExtensions;
  if (!extension.empty() && ::GetEnvironmentString("PATHEXT", forbiddenExtensions))
  {
    for (CsvList ext(forbiddenExtensions, PathNameUtil::PathNameDelimiter); ext; ++ext)
    {
      if (!(*ext).empty() && PathName::Compare(*ext, extension) == 0)
      {
        return false;
      }
    }
  }
#endif
  return true;
}

bool Utils::IsParentDirectoryOf(const PathName& parentDir, const PathName& fileName)
{
  size_t len1 = parentDir.GetLength();
  if (PathName::Compare(parentDir, fileName, len1) != 0)
  {
    return false;
  }
  size_t len2 = fileName.GetLength();
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
  return PathNameUtil::IsDirectoryDelimiter(fileName[len1]);
}

bool Utils::GetUncRootFromPath(const PathName& path, PathName& uncRoot)
{
  // must start with "\\" or "//"
  if (!(PathNameUtil::IsDirectoryDelimiter(path[0]) && PathNameUtil::IsDirectoryDelimiter(path[1])))
  {
    return false;
  }

  uncRoot = path;

  char* lpsz = uncRoot.GetData() + 2;

  if (lpsz[0] == 0 || lpsz[1] == 0)
  {
    return false;
  }

  // skip server name
  while (*++lpsz != 0)
  {
    if (PathNameUtil::IsDirectoryDelimiter(*lpsz))
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
    if (PathNameUtil::IsDirectoryDelimiter(*lpsz))
    {
      break;
    }
  }

  if (!(*lpsz == 0 || PathNameUtil::IsDirectoryDelimiter(*lpsz)))
  {
    return false;
  }

  *lpsz++ = PathNameUtil::DirectoryDelimiter;
  *lpsz = 0;

#if defined(MIKTEX_WINDOWS)
  uncRoot.ConvertToDos();
#endif

  return true;
}

bool Utils::GetPathNamePrefix(const PathName& path, const PathName& suffix, PathName& prefix)
{
  MIKTEX_ASSERT(!PathNameUtil::IsAbsolutePath(suffix));

  PathName path_(path);
  PathName suffix_(suffix);

  while (!suffix_.Empty())
  {
    if (PathName::Compare(path_.GetFileName(), suffix_.GetFileName()) != 0)
    {
      return false;
    }
    path_.CutOffLastComponent(true);
    suffix_.CutOffLastComponent(true);
  }

  prefix = path_;

  return true;
}

MIKTEXINTERNALFUNC(void) RemoveDirectoryDelimiter(char* lpszPath)
{
  size_t l = strlen(lpszPath);
  if (l > 1 && PathNameUtil::IsDirectoryDelimiter(lpszPath[l - 1]))
  {
#if defined(MIKTEX_WINDOWS)
    if (lpszPath[l - 2] == PathNameUtil::DosVolumeDelimiter)
    {
      return;
    }
#endif
    lpszPath[l - 1] = 0;
  }
}

MIKTEXINTERNALFUNC(const char*) GetFileNameExtension(const char* lpszPath)
{
  const char* lpszExtension = nullptr;
  for (const char* lpsz = lpszPath; *lpsz != 0; ++lpsz)
  {
    if (PathNameUtil::IsDirectoryDelimiter(*lpsz))
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

MIKTEXINTERNALFUNC(bool) IsExplicitlyRelativePath(const char* lpszPath)
{
  if (lpszPath[0] == '.')
  {
    return PathNameUtil::IsDirectoryDelimiter(lpszPath[1]) || (lpszPath[1] == '.' && PathNameUtil::IsDirectoryDelimiter(lpszPath[2]));
  }
  else
  {
    return false;
  }
}

MIKTEXINTERNALFUNC(PathName) GetFullyQualifiedPath(const char* lpszPath)
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
      MIKTEX_ASSERT(!PathNameUtil::IsDirectoryDelimiter(lpszPath[1]));
      int currentDrive = _getdrive();
      if (currentDrive == 0)
      {
        // TODO
        MIKTEX_UNEXPECTED();
      }
      MIKTEX_EXPECT(currentDrive >= 1 && currentDrive <= 26);
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

void Utils::PrintException(const exception& e)
{
  if (cerr.fail())
  {
    return;
  }
  try
  {
    cerr << "*** " << e.what() << endl;
  }
  catch (const exception&)
  {
  }
}

void Utils::PrintException(const MiKTeXException& e)
{
  if (cerr.fail())
  {
    return;
  }
  try
  {
    string programInvocationName(e.GetProgramInvocationName());
    string name;
    bool haveName = (programInvocationName.length() > 0);
    if (haveName)
    {
      PathName path(programInvocationName);
      name = path.GetFileName().ToString();
    }
    int last = '\n';
    for (const char& ch : e.GetErrorMessage())
    {
      if (haveName && last == '\n')
      {
        cerr << name << ": ";
      }
      cerr << ch;
      last = ch;
    }
    if (last != '\n')
    {
      cerr << endl;
      last = '\n';
    }
    for (const char& ch : e.GetInfo().ToString())
    {
      if (haveName && last == '\n')
      {
        cerr << name << ": ";
      }
      if (last == '\n')
      {
        cerr << "Data: ";
      }
      cerr << ch;
      last = ch;
    }
    if (last != '\n')
    {
      cerr << endl;
      last = '\n';
    }
  }
  catch (const exception&)
  {
  }
}

string Utils::GetExeName()
{
  auto session = SessionImpl::TryGetSession();
  if (session != nullptr)
  {
    return SessionImpl::GetSession()->GetMyProgramFile(false).GetFileNameWithoutExtension().ToString();
  }
  // TODO
  return "miktex";
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
  return MIKTEX_DISPLAY_VERSION_STR;
}

string Utils::GetMiKTeXBannerString()
{
  string banner = fmt::format("{0} {1}", MIKTEX_PRODUCTNAME_STR, GetMiKTeXVersionString());
#if defined(MIKTEX_WINDOWS_32)
  banner += " 32-bit";
#endif
  auto session = SessionImpl::TryGetSession();
  if (session != nullptr && session->IsMiKTeXPortable())
  {
    banner += " Portable";
  }
  return banner;
}

GitInfo Utils::GetGitInfo()
{
#if MIKTEX_HAVE_GIT_INFO
  GitInfo result;
  result.commit = MIKTEX_GIT_COMMIT_STR;
  result.commitAbbrev = MIKTEX_GIT_COMMIT_ABBREV_STR;
  result.authorDate = chrono::system_clock::from_time_t(MIKTEX_GIT_AUTHOR_DATE);
  return result;
#else
  MIKTEX_UNEXPECTED();
#endif
}

bool Utils::HaveGetGitInfo()
{
  return MIKTEX_HAVE_GIT_INFO != 0;
}

string GitInfo::ToString() const
{
  return fmt::format("{} / {:%Y-%m-%d %H:%M:%S}", this->commitAbbrev, fmt::gmtime(chrono::system_clock::to_time_t(this->authorDate)));
}

string Utils::MakeProgramVersionString(const string& programName, const VersionNumber& programVersionNumber)
{
  return fmt::format("{0} {1} ({2})", programName, programVersionNumber, GetMiKTeXBannerString());
}

bool Utils::GetEnvironmentString(const string& name, string& str)
{
  bool haveValue = ::GetEnvironmentString(name, str);
  if (SessionImpl::TryGetSession() != nullptr
    && SessionImpl::GetSession()->trace_env.get() != nullptr
    && SessionImpl::GetSession()->trace_env->IsEnabled("core", TraceLevel::Trace))
  {
    SessionImpl::GetSession()->trace_env->WriteLine("core", TraceLevel::Trace, fmt::format("{0} => {1}", name, (haveValue ? str : "null")));
  }
  return haveValue;
}

bool Utils::GetEnvironmentString(const string& name, PathName& path)
{
  string s;
  bool result = GetEnvironmentString(name, s);
  if (result)
  {
    path = s;
  }
  return result;
}

bool Utils::ParseDvipsMapLine(const string& line, FontMapEntry& mapEntry)
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
      for (Tokenizer tok(temp, " \t"); tok; ++tok)
      {
        if (mapEntry.specialInstructions.length() > 0)
        {
          mapEntry.specialInstructions += ' ';
        }
        mapEntry.specialInstructions += *tok;
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
        mapEntry.encFile = fileName.GetData();
        if (haveEncoding)
        {
          mapEntry.headerList += '[';
        }
      }
      else if (fileName.HasExtension(".pfa")
        || fileName.HasExtension(".pfb"))
      {
        mapEntry.fontFile = fileName.GetData();
      }
      if (noPartial)
      {
        mapEntry.headerList += '<';
      }
      mapEntry.headerList += fileName.GetData();
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

inline int GetC(FILE* stream)
{
  int ch = getc(stream);
  if (ferror(stream) != 0)
  {
    throw IOException();
  }
  return ch;
}

inline void UnGetC(int ch, FILE* stream)
{
  int ch2 = ungetc(ch, stream);
  if (ch2 == EOF)
  {
    throw IOException();
  }
}

bool Utils::ReadLine(string& str, FILE* stream, bool keepLineEnding)
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
static const char* const pathext[] = { ".com", ".exe", ".cmd", ".bat" };
#endif

bool Utils::FindProgram(const std::string& programName, PathName& path)
{
  string envPath;
  if (!Utils::GetEnvironmentString("PATH", envPath))
  {
    return false;
  }
  for (CsvList entry(envPath, PathNameUtil::PathNameDelimiter); entry; ++entry)
  {
    if ((*entry).empty())
    {
      continue;
    }
    PathName cand(*entry);
    cand /= programName;
#if defined(MIKTEX_WINDOWS)
    if (!cand.HasExtension())
    {
      for (const char* ext : pathext)
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

MIKTEXINTERNALFUNC(bool) FixProgramSearchPath(const string& oldPath, const PathName& binDir_, bool checkCompetition, string& newPath, bool& competition)
{
  bool modified = false;
  bool found = false;
  competition = false;
  newPath = "";
  PathName binDir = binDir_;
  binDir.AppendDirectoryDelimiter();
#if defined(MIKTEX_WINDOWS)
  binDir.ConvertToDos();
#endif
  for (CsvList entry(oldPath, PathNameUtil::PathNameDelimiter); entry; ++entry)
  {
    if ((*entry).empty())
    {
      continue;
    }
    string str2;
    for (const char& ch : *entry)
    {
      // FIXME: only on Windows
      if (ch != '"' && ch != '<' && ch != '>' && ch != '|')
      {
        str2 += ch;
      }
    }
    PathName dir(str2);
    dir.AppendDirectoryDelimiter();
    if (binDir == dir)
    {
      if (found)
      {
        // prevent duplicates
        continue;
      }
      found = true;
    }
    else if (Directory::Exists(dir) && checkCompetition)
    {
      PathName otherPdfTeX(dir);
      otherPdfTeX /= "pdftex" MIKTEX_EXE_FILE_SUFFIX;
      if (!found && File::Exists(otherPdfTeX))
      {
        int exitCode;
        ProcessOutput<80> pdfTeXOutput;
        bool isOtherPdfTeX = true;
        vector<string> args{ "pdftex", "--miktex-disable-installer", "--miktex-disable-maintenance", "--miktex-disable-diagnose", "--version" };
        shared_ptr<Session> session = Session::Get();
        if (session->IsAdminMode())
        {
          args.push_back("--miktex-admin");
        }
        if (Process::Run(otherPdfTeX, args, &pdfTeXOutput, &exitCode, nullptr) && exitCode == 0)
        {
          if (pdfTeXOutput.StdoutToString().find("MiKTeX") != string::npos)
          {
            isOtherPdfTeX = false;
          }
        }
        if (isOtherPdfTeX)
        {
          // another TeX system is in our way; push it out
          // from this place
          if (!newPath.empty())
          {
            newPath += PathNameUtil::PathNameDelimiter;
          }
          newPath += binDir.GetData();
          found = true;
          modified = true;
          competition = true;
        }
      }
    }
    if (!newPath.empty())
    {
      newPath += PathNameUtil::PathNameDelimiter;
    }
    newPath += *entry;
  }
  if (!found)
  {
    // MiKTeX is not yet in the PATH
    if (!newPath.empty())
    {
      newPath += PathNameUtil::PathNameDelimiter;
    }
    newPath += binDir.GetData();
    modified = true;
  }
  return modified;
}

pair<bool, bool> Utils::CheckPath()
{
  shared_ptr<SessionImpl> session = SessionImpl::GetSession();
  string envPath;
  if (!Utils::GetEnvironmentString("PATH", envPath))
  {
    return make_pair(false, false);
  }
  PathName linkTargetDirectory = session->GetSpecialPath(SpecialPath::LinkTargetDirectory);
  string repairedPath;
  bool pathCompetition;
  bool pathOkay = !Directory::Exists(linkTargetDirectory) || !FixProgramSearchPath(envPath, linkTargetDirectory, true, repairedPath, pathCompetition);
  if (!pathOkay)
  {
    session->trace_error->WriteLine("core", TraceLevel::Error, T_("Something is wrong with the PATH:"));
    session->trace_error->WriteLine("core", TraceLevel::Error, envPath);
  }
  return make_pair(pathOkay, pathCompetition);
}

unsigned long long Utils::ToUnsignedLongLong(const string& s)
{
  unsigned long long val;
  if (s == "0")
  {
    return 0;
  }
  bool good = true;
  if (s.empty() || !(s[0] == '1' || s[0] == '2' || s[0] == '3' || s[0] == '4' || s[0] == '5' || s[0] == '6' || s[0] == '7' || s[0] == '8' || s[0] == '9'))
  {
    good = false;
  }
  else
  {
    try
    {
      size_t pos;
      val = std::stoull(s, &pos, 10);
      if (pos != s.length())
      {
        good = false;
      }
    }
    catch (const invalid_argument&)
    {
      good = false;
    }
    catch (const out_of_range&)
    {
      good = false;
    }
  }
  if (!good)
  {
    MIKTEX_FATAL_ERROR_2(T_("Invalid number."), "number", s);
  }
  return val;
}

size_t Utils::ToSizeT(const string& s)
{
  return ToUnsignedLongLong(s);
}

time_t Utils::ToTimeT(const string& s)
{
  return ToUnsignedLongLong(s);
}

pair<bool, PathName> Utils::ExpandTilde(const string& s)
{
  if (!s.empty() && s[0] == '~' && (s.length() == 1 || PathNameUtil::IsDirectoryDelimiter(s[1])))
  {
    PathName pathFQ = GetHomeDirectory();
    if (!pathFQ.IsAbsolute())
    {
      TraceError(fmt::format(T_("cannot expand ~: {0} is not fully qualified"), Q_(pathFQ)));
      return make_pair(false , PathName());
    }
    if (s[1] != 0 && PathNameUtil::IsDirectoryDelimiter(s[1]) && s[2] != 0)
    {
      pathFQ /= &s[2];
    }
    return make_pair(true, pathFQ);
  }
  else
  {
    return make_pair(false, PathName());
  }
}
