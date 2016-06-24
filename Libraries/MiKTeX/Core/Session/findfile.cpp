/* findfile.cpp: finding files

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
#include "miktex/Core/Paths.h"
#include "miktex/Core/Registry.h"

#include "Fndb/FileNameDatabase.h"
#include "Session/SessionImpl.h"
#include "Utils/AutoTraceTime.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Util;
using namespace std;

void SessionImpl::SetFindFileCallback(IFindFileCallback * callback)
{
  findFileCallback = callback;
}

bool SessionImpl::CheckCandidate(PathName & path, const char * lpszFileInfo)
{
  bool found = false;
  if (IsMpmFile(path.Get()))
  {
    PathName trigger(Utils::GetRelativizedPath(path.Get(), MPM_ROOT_PATH));
    PathName installRoot;
    if (lpszFileInfo != nullptr && findFileCallback != nullptr && findFileCallback->InstallPackage(lpszFileInfo, trigger, installRoot))
    {
      PathName temp = installRoot;
      temp /= path.Get() + MPM_ROOT_PATH_LEN;
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

bool SessionImpl::SearchFileSystem(const char * lpszFileName, const char * lpszDirectoryPattern, bool firstMatchOnly, vector<PathName> & result)
{
  MIKTEX_ASSERT(result.empty());

  if (IsMpmFile(lpszDirectoryPattern))
  {
    return false;
  }

  trace_filesearch->WriteFormattedLine("core", T_("file system search: filename=%s, directory=%s"), Q_(lpszFileName), Q_(lpszDirectoryPattern));

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
    PathName path(*it, lpszFileName);
    if (CheckCandidate(path, nullptr))
    {
      found = true;
      result.push_back(path);
    }
  }

  return found;
}

bool SessionImpl::FindFileInternal(const char * lpszFileName, const vector<PathName> & directoryPatterns, bool firstMatchOnly, bool useFndb, bool searchFileSystem, vector<PathName> & result)

{
  AutoTraceTime att("find file", lpszFileName);

  MIKTEX_ASSERT(useFndb || searchFileSystem);

  bool found = false;

  // if a fully qualified path name is given, then don't look out any
  // further
  if (Utils::IsAbsolutePath(lpszFileName))
  {
    PathName path(lpszFileName);
    found = CheckCandidate(path, nullptr);
    if (found)
    {
      result.push_back(path);
    }
    return found;
  }

  // if an explicitly relative path name is given, then don't look out
  // any further
  if (IsExplicitlyRelativePath(lpszFileName))
  {
    PathName pathWD;
    for (unsigned idx = 0; !(found && firstMatchOnly) && GetWorkingDirectory(idx, pathWD); ++idx)
    {
      PathName path(pathWD);
      path /= lpszFileName;
      path.MakeAbsolute();
      if (CheckCandidate(path, nullptr))
      {
	found = true;
#if FIND_FILE_PREFER_RELATIVE_PATH_NAMES
	// 2015-01-15
	if (idx == 0)
	{
	  MIKTEX_ASSERT(PathName::Compare(pathWD, PathName().SetToCurrentDirectory()) == 0);
	  path = lpszFileName;
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
      trace_filesearch->WriteFormattedLine("core", T_("going to search in fndb: filename=%s, directory=%s"), Q_(lpszFileName), Q_(it->ToString()));
      if (found && !firstMatchOnly && IsMpmFile(it->Get()))
      {
	// don't trigger the package installer
	continue;
      }
      shared_ptr<FileNameDatabase> fndb = GetFileNameDatabase(it->Get());
      if (fndb != nullptr)
      {
	// search fndb
	vector<PathName> paths;
	vector<string> fileNameInfo;
	bool foundInFndb = fndb->Search(lpszFileName, it->Get(), firstMatchOnly, paths, fileNameInfo);
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
	trace_filesearch->WriteFormattedLine("core", T_("no fndb found, so going to continue on disk: filename=%s, directory=%s"), Q_(lpszFileName), Q_(it->ToString()));
	// search the file system because the file name database does not exist
	vector<PathName> paths;
	if (SearchFileSystem(lpszFileName, it->Get(), firstMatchOnly, paths))
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
    if (found && !firstMatchOnly && IsMpmFile(it->Get()))
    {
      // don't search the virtual MPM directory tree
      continue;
    }
    shared_ptr<FileNameDatabase> fndb = GetFileNameDatabase(it->Get());
    if (fndb == nullptr)
    {
      // fndb does not exist => we already searched the file system (see above)
      continue;
    }
    fndb = nullptr;
    vector<PathName> paths;
    if (SearchFileSystem(lpszFileName, it->Get(), firstMatchOnly, paths))
    {
      found = true;
      result.insert(result.end(), paths.begin(), paths.end());
    }
  }

  return found;
}

inline bool IsNewer(const PathName & path1, const PathName & path2)
{
  return File::Exists(path1) && File::Exists(path2) && File::GetLastWriteTime(path1) > File::GetLastWriteTime(path2);
}

bool SessionImpl::FindFileInternal(const char * lpszFileName, FileType fileType, bool firstMatchOnly, bool tryHard, bool create, bool renew, vector<PathName> & result)
{
  MIKTEX_ASSERT(result.empty());

  // try to derive the file type
  if (fileType == FileType::None)
  {
    fileType = DeriveFileType(lpszFileName);
    if (fileType == FileType::None)
    {
      trace_filesearch->WriteFormattedLine("core", T_("cannot derive file type from %s"), Q_(lpszFileName));
      return false;
    }
  }

  if (renew && (findFileCallback == nullptr || !findFileCallback->TryCreateFile(lpszFileName, fileType)))
  {
    return false;
  }

  // construct the search vector
  vector<PathName> vec = ConstructSearchVector(fileType);

  // get the file type information
  const InternalFileTypeInfo * fileTypeInfo = GetInternalFileTypeInfo(fileType);
  MIKTEX_ASSERT(fileTypeInfo != nullptr);

  // check to see whether the file name has a registered file name extension
  const char * lpszExtension = GetFileNameExtension(lpszFileName);
  bool hasRegisteredExtension = false;
  if (lpszExtension != nullptr)
  {
    for (CSVList ext(fileTypeInfo->fileNameExtensions, PATH_DELIMITER); ext.GetCurrent() != nullptr && !hasRegisteredExtension; ++ext)
    {
      if (PathName::Compare(lpszExtension, ext.GetCurrent()) == 0)
      {
	hasRegisteredExtension = true;
      }
    }
    for (CSVList ext(fileTypeInfo->alternateExtensions, PATH_DELIMITER); ext.GetCurrent() != nullptr && !hasRegisteredExtension; ++ext)
    {
      if (PathName::Compare(lpszExtension, ext.GetCurrent()) == 0)
      {
	hasRegisteredExtension = true;
      }
    }
  }

  vector<PathName> fileNamesToTry;

  // try each registered file name extension, if none was specified
  if (!hasRegisteredExtension)
  {
    for (CSVList ext(fileTypeInfo->fileNameExtensions, PATH_DELIMITER); ext.GetCurrent() != nullptr; ++ext)
    {
      PathName fileName(lpszFileName);
      fileName.AppendExtension(ext.GetCurrent());
      fileNamesToTry.push_back(fileName);
    }
  }

  // try it with the given file name
  fileNamesToTry.push_back(lpszFileName);

  // first round: use the fndb
  for (vector<PathName>::const_iterator it = fileNamesToTry.begin(); it != fileNamesToTry.end(); ++it)
  {
    if (FindFileInternal(it->Get(), vec, firstMatchOnly, true, false, result) && firstMatchOnly)
    {
      return true;
    }
  }

  // second round: don't use the fndb
  if (tryHard)
  {
    for (vector<PathName>::const_iterator it = fileNamesToTry.begin(); it != fileNamesToTry.end(); ++it)
    {
      if (FindFileInternal(it->Get(), vec, firstMatchOnly, false, true, result) && firstMatchOnly)
      {
	return true;
      }
    }
  }

  if (create)
  {
    if (result.empty())
    {
      if (findFileCallback != nullptr && findFileCallback->TryCreateFile(lpszFileName, fileType))
      {
	FindFileInternal(lpszFileName, vec, firstMatchOnly, true, false, result);
      }
    }
    else if ((fileType == FileType::BASE || fileType == FileType::FMT || fileType == FileType::MEM) && GetConfigValue(MIKTEX_REGKEY_TEXMF, MIKTEX_REGVAL_RENEW_FORMATS_ON_UPDATE, true))
    {
      PathName pathPackagesIniC(GetSpecialPath(SpecialPath::CommonInstallRoot), MIKTEX_PATH_PACKAGES_INI, PathName());
      PathName pathPackagesIniU(GetSpecialPath(SpecialPath::UserInstallRoot), MIKTEX_PATH_PACKAGES_INI, PathName());
      if (IsNewer(pathPackagesIniC, result[0]) || (pathPackagesIniU != pathPackagesIniC && IsNewer(pathPackagesIniU, result[0])))
      {
	if (findFileCallback != nullptr && findFileCallback->TryCreateFile(lpszFileName, fileType))
	{
	  result.clear();
	  FindFileInternal(lpszFileName, vec, firstMatchOnly, true, false, result);
	}
      }
    }
  }

  return !result.empty();
}

bool SessionImpl::FindFile(const char * lpszFileName, const char * lpszPathList, FindFileOptionSet options, vector<PathName> & result)
{
  MIKTEX_ASSERT_STRING(lpszFileName);
  MIKTEX_ASSERT_STRING(lpszPathList);

  bool found = FindFileInternal(lpszFileName, SplitSearchPath(lpszPathList), !options[FindFileOption::All], true, false, result);

  if (!found && options[FindFileOption::TryHard])
  {
    found = FindFileInternal(lpszFileName, SplitSearchPath(lpszPathList), !options[FindFileOption::All], false, true, result);
  }

  return found;
}

bool SessionImpl::FindFile(const char * lpszFileName, const char * lpszPathList, FindFileOptionSet options, PathName & result)
{
  MIKTEX_ASSERT_STRING(lpszFileName);
  MIKTEX_ASSERT_STRING(lpszPathList);
  MIKTEX_ASSERT(!options[(size_t)FindFileOption::All]);

  vector<PathName> paths;

  bool found = FindFile(lpszFileName, lpszPathList, options, paths);

  if (found)
  {
    result = paths[0];
  }

  return found;
}

bool SessionImpl::FindFile(const char * lpszFileName, FileType fileType, FindFileOptionSet options, vector<PathName> & result)
{
  return FindFileInternal(lpszFileName, fileType, !options[FindFileOption::All], options[FindFileOption::TryHard], options[FindFileOption::Create], options[FindFileOption::Renew], result);
}

bool SessionImpl::FindFile(const char * lpszFileName, FileType fileType, FindFileOptionSet options, PathName & result)
{
  MIKTEX_ASSERT(!options[(size_t)FindFileOption::All]);
  vector<PathName> paths;
  bool found = FindFile(lpszFileName, fileType, options, paths);
  if (found)
  {
    result = paths[0];
  }
  return found;
}

#define DEFAULT_PK_NAME_TEMPLATE "%f.pk"

bool SessionImpl::MakePkFileName(PathName & pkFileName, const char * lpszFontName, int dpi)
{
  string nameTemplate;

  if (!GetSessionValue(MIKTEX_REGKEY_CORE, MIKTEX_REGVAL_PK_FN_TEMPLATE, nameTemplate, DEFAULT_PK_NAME_TEMPLATE))
  {
    MIKTEX_UNEXPECTED();
  }

  string str;

  str.reserve(BufferSizes::MaxPath);

  for (const char * p = nameTemplate.c_str(); *p != 0; ++p)
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
#  define DEFAULT_PK_SEARCH_PATH ".;%R\\fonts\\pk\\%m//dpi%d"
#else
#  define DEFAULT_PK_SEARCH_PATH ".:%R/fonts/pk/%m//dpi%d"
#endif

bool SessionImpl::FindPkFile(const char * lpszFontName, const char * lpszMode, int dpi, PathName & result)
{
  MIKTEX_ASSERT_STRING(lpszFontName);
  MIKTEX_ASSERT_STRING_OR_NIL(lpszMode);

  PathName pkFileName;

  if (!MakePkFileName(pkFileName, lpszFontName, dpi))
  {
    return false;
  }

  string searchPathTemplate;

  if (!GetSessionValue(MIKTEX_REGKEY_CORE, "PKPath", searchPathTemplate, DEFAULT_PK_SEARCH_PATH))
  {
    MIKTEX_UNEXPECTED();
  }

  string searchPath;

  for (const char * q = searchPathTemplate.c_str(); *q != 0; ++q)
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
	if (lpszMode != nullptr)
	{
	  searchPath += lpszMode;
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

  bool found = FindFile(pkFileName.Get(), searchPath.c_str(), result);

  if (!found && (lpszMode == nullptr || StringCompare(lpszMode, "modeless", true) != 0))
  {
    // RECURSION
    found = FindPkFile(lpszFontName, "modeless", dpi, result);
  }

  return found;
}
