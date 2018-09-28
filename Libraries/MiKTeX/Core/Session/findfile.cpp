/* findfile.cpp: finding files

   Copyright (C) 1996-2017 Christian Schenk

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

#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"

#include "Fndb/FileNameDatabase.h"
#include "Session/SessionImpl.h"
#include "Utils/AutoTraceTime.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

void SessionImpl::SetFindFileCallback(IFindFileCallback* callback)
{
  findFileCallback = callback;
}

bool SessionImpl::CheckCandidate(PathName& path, const char* lpszFileInfo)
{
  bool found = false;
  if (IsMpmFile(path.GetData()))
  {
    PathName trigger(Utils::GetRelativizedPath(path.GetData(), MPM_ROOT_PATH));
    PathName installRoot;
    if (lpszFileInfo != nullptr && findFileCallback != nullptr && findFileCallback->InstallPackage(lpszFileInfo, trigger, installRoot))
    {
      PathName temp = installRoot;
      temp /= path.GetData() + MPM_ROOT_PATH_LEN;
      if (!File::Exists(temp))
      {
        MIKTEX_FATAL_ERROR_2(T_("The installed file does not exist."), "fileName", temp.ToString());
      }
      path = temp;
      found = true;
    }
  }
  else
  {
    found = File::Exists(path);
  }
  return found;
}

bool SessionImpl::SearchFileSystem(const string& fileName, const char* lpszDirectoryPattern, bool firstMatchOnly, vector<PathName>& result)
{
  MIKTEX_ASSERT(result.empty());

  if (IsMpmFile(lpszDirectoryPattern))
  {
    return false;
  }

  trace_filesearch->WriteFormattedLine("core", T_("file system search: filename=%s, directory=%s"), Q_(fileName), Q_(lpszDirectoryPattern));

  vector<PathName> directories;

  PathName comparableDirectoryPattern(lpszDirectoryPattern);
  comparableDirectoryPattern.TransformForComparison();

  SearchPathDictionary::const_iterator it = expandedPathPatterns.find(comparableDirectoryPattern.ToString());

  if (it == expandedPathPatterns.end())
  {
    ExpandPathPattern("", lpszDirectoryPattern, directories);
    expandedPathPatterns[comparableDirectoryPattern.ToString()] = directories;
  }
  else
  {
    directories = it->second;
  }

  bool found = false;

  for (vector<PathName>::const_iterator it = directories.begin(); !(found && firstMatchOnly) && it != directories.end(); ++it)
  {
    PathName path(*it, fileName);
    if (CheckCandidate(path, nullptr))
    {
      found = true;
      result.push_back(path);
    }
  }

  return found;
}

bool SessionImpl::FindFileInternal(const string& fileName, const vector<PathName>& directoryPatterns, bool firstMatchOnly, bool useFndb, bool searchFileSystem, vector<PathName>& result)

{
  AutoTraceTime att("find file", fileName.c_str());

  MIKTEX_ASSERT(useFndb || searchFileSystem);

  bool found = false;

  // if a fully qualified path name is given, then don't look out any
  // further
  if (Utils::IsAbsolutePath(fileName))
  {
    PathName path(fileName);
    found = CheckCandidate(path, nullptr);
    if (found)
    {
      result.push_back(path);
    }
    return found;
  }

  // if an explicitly relative path name is given, then don't look out
  // any further
  if (IsExplicitlyRelativePath(fileName.c_str()))
  {
    PathName pathWD;
    for (unsigned idx = 0; !(found && firstMatchOnly) && GetWorkingDirectory(idx, pathWD); ++idx)
    {
      PathName path(pathWD);
      path /= fileName;
      path.MakeAbsolute();
      if (CheckCandidate(path, nullptr))
      {
        found = true;
#if FIND_FILE_PREFER_RELATIVE_PATH_NAMES
        // 2015-01-15
        if (idx == 0)
        {
          MIKTEX_ASSERT(PathName::Compare(pathWD, PathName().SetToCurrentDirectory()) == 0);
          path = fileName;
        }
#endif
        result.push_back(path);
      }
    }
    return found;
  }

  // make use of the file name database
  if (useFndb)
  {
    for (vector<PathName>::const_iterator it = directoryPatterns.begin(); !(found && firstMatchOnly) && it != directoryPatterns.end(); ++it)
    {
      trace_filesearch->WriteFormattedLine("core", T_("going to search in fndb: filename=%s, directory=%s"), Q_(fileName), Q_(it->ToString()));
      if (found && !firstMatchOnly && IsMpmFile(it->GetData()))
      {
        // don't trigger the package installer
        continue;
      }
      shared_ptr<FileNameDatabase> fndb = GetFileNameDatabase(it->GetData());
      if (fndb != nullptr)
      {
        // search fndb
        vector<PathName> paths;
        vector<string> fileNameInfo;
        bool foundInFndb = fndb->Search(fileName, it->GetData(), firstMatchOnly, paths, fileNameInfo);
        // we must release the FNDB handle since CheckCandidate() might request an unload of the FNDB
        fndb = nullptr;
        if (foundInFndb)
        {
          for (int idx = 0; idx < paths.size(); ++idx)
          {
            if (CheckCandidate(paths[idx], fileNameInfo[idx].c_str()))
            {
              found = true;
              result.push_back(paths[idx]);
            }
          }
        }
      }
      else
      {
        trace_filesearch->WriteFormattedLine("core", T_("no fndb found, so going to continue on disk: filename=%s, directory=%s"), Q_(fileName), Q_(it->ToString()));
        // search the file system because the file name database does not exist
        vector<PathName> paths;
        if (SearchFileSystem(fileName, it->GetData(), firstMatchOnly, paths))
        {
          found = true;
          result.insert(result.end(), paths.begin(), paths.end());
        }
      }
    }
  }

  if (found || !searchFileSystem)
  {
    return found;
  }

  // search the file system
  for (vector<PathName>::const_iterator it = directoryPatterns.begin(); !(found && firstMatchOnly) && it != directoryPatterns.end(); ++it)
  {
    if (found && !firstMatchOnly && IsMpmFile(it->GetData()))
    {
      // don't search the virtual MPM directory tree
      continue;
    }
    unsigned rootIdx = TryDeriveTEXMFRoot(*it);
    if (rootIdx != INVALID_ROOT_INDEX && IsManagedRoot(rootIdx))
    {
      // don't search managed root file system
      continue;
    }
    shared_ptr<FileNameDatabase> fndb = GetFileNameDatabase(it->GetData());
    if (fndb == nullptr)
    {
      // fndb does not exist => we already searched the file system (see above)
      continue;
    }
    fndb = nullptr;
    vector<PathName> paths;
    if (SearchFileSystem(fileName.c_str(), it->GetData(), firstMatchOnly, paths))
    {
      found = true;
      result.insert(result.end(), paths.begin(), paths.end());
    }
  }

  return found;
}

inline bool IsNewer(const PathName& path1, const PathName& path2)
{
  return File::Exists(path1) && File::Exists(path2) && File::GetLastWriteTime(path1) > File::GetLastWriteTime(path2);
}

bool SessionImpl::FindFileInternal(const string& fileName, FileType fileType, bool firstMatchOnly, bool tryHard, bool create, bool renew, vector<PathName>& result)
{
  MIKTEX_ASSERT(result.empty());

  // try to derive the file type
  if (fileType == FileType::None)
  {
    fileType = DeriveFileType(fileName);
    if (fileType == FileType::None)
    {
      trace_filesearch->WriteFormattedLine("core", T_("cannot derive file type from %s"), Q_(fileName));
      return false;
    }
  }

  if (renew && (findFileCallback == nullptr || !findFileCallback->TryCreateFile(fileName, fileType)))
  {
    return false;
  }

  // construct the search vector
  vector<PathName> vec = ConstructSearchVector(fileType);

  // get the file type information
  const InternalFileTypeInfo* fti = GetInternalFileTypeInfo(fileType);
  MIKTEX_ASSERT(fileTypeInfo != nullptr);

  // check to see whether the file name has a registered file name extension
  PathName extension = PathName(fileName).GetExtension();
  bool hasRegisteredExtension = !extension.Empty()
    && (std::find_if(fti->fileNameExtensions.begin(), fti->fileNameExtensions.end(), [extension](const string& ext) { return extension == ext; }) != fti->fileNameExtensions.end()
      || std::find_if(fti->alternateExtensions.begin(), fti->alternateExtensions.end(), [extension](const string& ext) { return extension == ext; }) != fti->alternateExtensions.end());

  vector<PathName> fileNamesToTry;

  // try each registered file name extension, if none was specified
  if (!hasRegisteredExtension)
  {
    for (const string& ext : fti->fileNameExtensions)
    {
      fileNamesToTry.push_back(PathName(fileName).AppendExtension(ext));
    }
  }

  // try it with the given file name
  fileNamesToTry.push_back(fileName);

  // first round: use the fndb
  for (const PathName& fn : fileNamesToTry)
  {
    if (FindFileInternal(fn.GetData(), vec, firstMatchOnly, true, false, result) && firstMatchOnly)
    {
      return true;
    }
  }

  // second round: don't use the fndb
  if (tryHard)
  {
    for (const PathName& fn : fileNamesToTry)
    {
      if (FindFileInternal(fn.GetData(), vec, firstMatchOnly, false, true, result) && firstMatchOnly)
      {
        return true;
      }
    }
  }

  if (create)
  {
    if (result.empty())
    {
      if (findFileCallback != nullptr && findFileCallback->TryCreateFile(fileName, fileType))
      {
        FindFileInternal(fileName, vec, firstMatchOnly, true, false, result);
      }
    }
    else if ((fileType == FileType::BASE || fileType == FileType::FMT || fileType == FileType::MEM) && GetConfigValue(MIKTEX_REGKEY_TEXMF, MIKTEX_REGVAL_RENEW_FORMATS_ON_UPDATE, true).GetBool())
    {
      PathName pathPackagesIniC(GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGES_INI);
      PathName pathPackagesIniU(GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGES_INI);
      if (IsNewer(pathPackagesIniC, result[0]) || (pathPackagesIniU != pathPackagesIniC && IsNewer(pathPackagesIniU, result[0])))
      {
        if (findFileCallback != nullptr && findFileCallback->TryCreateFile(fileName, fileType))
        {
          result.clear();
          FindFileInternal(fileName, vec, firstMatchOnly, true, false, result);
        }
      }
    }
  }

  return !result.empty();
}

bool SessionImpl::FindFile(const string& fileName, const string& pathList, FindFileOptionSet options, vector<PathName>& result)
{
  bool found = FindFileInternal(fileName, SplitSearchPath(pathList), !options[FindFileOption::All], true, false, result);

  if (!found && options[FindFileOption::TryHard])
  {
    found = FindFileInternal(fileName, SplitSearchPath(pathList), !options[FindFileOption::All], false, true, result);
  }

  return found;
}

bool SessionImpl::FindFile(const string& fileName, const string& pathList, FindFileOptionSet options, PathName& result)
{
  MIKTEX_ASSERT(!options[FindFileOption::All]);

  vector<PathName> paths;

  bool found = FindFile(fileName, pathList, options, paths);

  if (found)
  {
    result = paths[0];
  }

  return found;
}

bool SessionImpl::FindFile(const string& fileName, FileType fileType, FindFileOptionSet options, vector<PathName>& result)
{
  return FindFileInternal(fileName, fileType, !options[FindFileOption::All], options[FindFileOption::TryHard], options[FindFileOption::Create], options[FindFileOption::Renew], result);
}

bool SessionImpl::FindFile(const string& fileName, FileType fileType, FindFileOptionSet options, PathName& result)
{
  MIKTEX_ASSERT(!options[FindFileOption::All]);
  vector<PathName> paths;
  bool found = FindFile(fileName, fileType, options, paths);
  if (found)
  {
    result = paths[0];
  }
  return found;
}

static const string DEFAULT_PK_NAME_TEMPLATE = "%f.pk";

bool SessionImpl::MakePkFileName(PathName& pkFileName, const char* lpszFontName, int dpi)
{
  string nameTemplate;

  if (!GetSessionValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_PK_FN_TEMPLATE, nameTemplate))
  {
    nameTemplate = DEFAULT_PK_NAME_TEMPLATE;
  }

  string str;

  str.reserve(BufferSizes::MaxPath);

  for (const char* p = nameTemplate.c_str(); *p != 0; ++p)
  {
    if (p[0] == '%')
    {
      ++p;
      if (*p == 0)
      {
        MIKTEX_UNEXPECTED();
      }
      switch (*p)
      {
      case '%':
        str += '%';
        break;
      case 'd':
        str += std::to_string(dpi);
        break;
      case 'f':
        str += lpszFontName;
        break;
      default:
        MIKTEX_UNEXPECTED();
      }
    }
    else
    {
      str += *p;
    }
  }

  pkFileName = str.c_str();

  return true;
}

#if defined(MIKTEX_WINDOWS)
static const string DEFAULT_PK_SEARCH_PATH = ".;%R\\fonts\\pk\\%m//dpi%d";
#else
static const string DEFAULT_PK_SEARCH_PATH = ".:%R/fonts/pk/%m//dpi%d";
#endif

bool SessionImpl::FindPkFile(const string& fontName, const string& mfMode, int dpi, PathName& result)
{
  PathName pkFileName;

  if (!MakePkFileName(pkFileName, fontName.c_str(), dpi))
  {
    return false;
  }

  string searchPathTemplate;

  if (!GetSessionValue(MIKTEX_REGKEY_CORE, "PKPath", searchPathTemplate))
  {
    searchPathTemplate = DEFAULT_PK_SEARCH_PATH;
  }

  string searchPath;

  for (const char* q = searchPathTemplate.c_str(); *q != 0; ++q)
  {
    if (*q == '%')
    {
      ++q;
      if (*q == 0)
      {
        MIKTEX_UNEXPECTED();
      }
      switch (*q)
      {
      case 'R':
        searchPath += '%';
        searchPath += 'R';
        break;
      case '%':
        searchPath += '%';
        break;
      case 'm':
        if (!mfMode.empty())
        {
          searchPath += mfMode;
        }
        else
        {
          // FIXME: hardcoded METAFONT mode
          searchPath += "ljfour";
        }
        break;
      case 'd':
        searchPath += std::to_string(dpi);
        break;
      default:
        MIKTEX_UNEXPECTED();
      }
    }
    else
    {
      searchPath += *q;
    }
  }

  bool found = FindFile(pkFileName.GetData(), searchPath, result);

  if (!found && (mfMode.empty() || StringCompare(mfMode.c_str(), "modeless", true) != 0))
  {
    // RECURSION
    found = FindPkFile(fontName, "modeless", dpi, result);
  }

  return found;
}
