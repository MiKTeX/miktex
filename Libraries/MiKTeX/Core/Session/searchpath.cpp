/* searchpath.cpp: managing search paths

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

#include "miktex/Core/Directory.h"
#include "miktex/Core/CSVList.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

void SessionImpl::ExpandRootDirectories(const char * lpszToBeExpanded, vector<PathName> & paths)
{
  if (lpszToBeExpanded[0] == '%' && (lpszToBeExpanded[1] == 'R' || lpszToBeExpanded[1] == 'r'))
  {
    const char * lpszSuffix = lpszToBeExpanded + 2;
    if (IsDirectoryDelimiter(*lpszSuffix))
    {
      ++lpszSuffix;
    }
    for (unsigned idx = 0; idx < GetNumberOfTEXMFRoots(); ++idx)
    {
      PathName path2 = GetRootDirectory(idx);
      path2.AppendDirectoryDelimiter();
      path2.Append(lpszSuffix, false);
      paths.push_back(path2);
    }
    if (lpszToBeExpanded[1] == 'R')
    {
      PathName path2 = MPM_ROOT_PATH;
      path2.AppendDirectoryDelimiter();
      path2.Append(lpszSuffix, false);
      paths.push_back(path2);
    }
  }
  else
  {
    paths.push_back(lpszToBeExpanded);
  }
}

vector<PathName> SessionImpl::ExpandRootDirectories(const char * lpszToBeExpanded)
{
  vector<PathName> result;
  for (CSVList path(lpszToBeExpanded, PATH_DELIMITER); path.GetCurrent() != nullptr; ++path)
  {
    ExpandRootDirectories(path.GetCurrent(), result);
  }
  return result;
}

void SessionImpl::PushBackPath(vector<PathName> & vec, const PathName & path)
{
  vector<PathName> paths = ExpandBraces(path.Get());

  for (const PathName & path : paths)
  {
    // expand '~'
    if (path[0] == '~' && (path[1] == 0 || IsDirectoryDelimiter(path[1])))
    {
      PathName pathFQ = GetHomeDirectory();
      if (!Utils::IsAbsolutePath(pathFQ.Get()))
      {
	TraceError(T_("cannot expand ~: %s is not fully qualified"), Q_(pathFQ));
	continue;
      }
      if (path[1] != 0 && IsDirectoryDelimiter(path[1]) && path[2] != 0)
      {
	pathFQ /= &path[2];
      }
      if (find(vec.begin(), vec.end(), pathFQ) == vec.end())
      {
	vec.push_back(pathFQ);
      }
      continue;
    }

    // fully qualified path?
    if (Utils::IsAbsolutePath(path.Get()))
    {
      if (find(vec.begin(), vec.end(), path) == vec.end())
      {
	vec.push_back(path);
      }
      continue;
    }

    // it is a relative path
    PathName pathFQ;
    for (unsigned idx = 0; GetWorkingDirectory(idx, pathFQ); ++idx)
    {
      if (!Utils::IsAbsolutePath(pathFQ.Get()))
      {
	TraceError(T_("%s is not fully qualified"), Q_(pathFQ));
	continue;
      }
      if (PathName::Compare(path, CURRENT_DIRECTORY) != 0)
      {
	pathFQ /= path.Get();
      }
      else
      {
#if FIND_FILE_PREFER_RELATIVE_PATH_NAMES
	// 2015-01-15
	if (idx == 0)
	{
	  MIKTEX_ASSERT(PathName::Compare(pathFQ, PathName().SetToCurrentDirectory()) == 0);
	  pathFQ = CURRENT_DIRECTORY;
	}
#endif
      }
      if (find(vec.begin(), vec.end(), pathFQ) == vec.end())
      {
	vec.push_back(pathFQ);
      }
    }
  }
}

void SessionImpl::SplitSearchPath(vector<PathName> & vec, const char * lpszSearchPath)
{
  for (CSVList path(lpszSearchPath, PATH_DELIMITER); path.GetCurrent() != nullptr; ++path)
  {
    PushBackPath(vec, path.GetCurrent());
  }
}

vector<PathName> SessionImpl::SplitSearchPath(const char * lpszSearchPath)
{
  vector<PathName> vec;
  SplitSearchPath(vec, lpszSearchPath);
  return vec;
}

MIKTEXINTERNALFUNC(string) MakeSearchPath(const vector<PathName> & vec)
{
  string searchPath;
  for (const PathName & path : vec)
  {
    if (!searchPath.empty())
    {
      searchPath += PATH_DELIMITER;
    }
    searchPath += path.Get();
  }
  return searchPath;
}

void SessionImpl::AppendToSearchPath(string & searchPath, const string & searchPath2)
{
  vector<PathName> vec = SplitSearchPath(searchPath.c_str());
  for (CSVList path(searchPath2, PATH_DELIMITER); path.GetCurrent() != nullptr; ++path)
  {
    PushBackPath(vec, path.GetCurrent());
  }
  searchPath = MakeSearchPath(vec);
}

void SessionImpl::TraceSearchVector(const char * lpszKey, const vector<PathName> & vec)
{
  if (!trace_filesearch->IsEnabled())
  {
    return;
  }
  trace_filesearch->WriteFormattedLine("core", T_("search vector %s:"), lpszKey);
  unsigned nr = 0;
  for (vector<PathName>::const_iterator it = vec.begin(); it != vec.end(); ++it, ++nr)
  {
    trace_filesearch->WriteFormattedLine("core", T_("  %2u: %s"), nr, it->Get());
  }
}

vector<PathName> SessionImpl::ConstructSearchVector(FileType fileType)
{
  InternalFileTypeInfo * pfti = GetInternalFileTypeInfo(fileType);
  if (pfti->searchVec.empty())
  {
    for (CSVList env(pfti->envVarNames, PATH_DELIMITER); env.GetCurrent() != nullptr; ++env)
    {
      string searchPath;
      if (Utils::GetEnvironmentString(env.GetCurrent(), searchPath))
      {
	SplitSearchPath(pfti->searchVec, searchPath.c_str());
      }
    }
    SplitSearchPath(pfti->searchVec, pfti->searchPath.c_str());
    TraceSearchVector(pfti->fileTypeString.c_str(), pfti->searchVec);
  }
  return pfti->searchVec;
}

string SessionImpl::GetExpandedSearchPath(FileType fileType)
{
  MIKTEX_ASSERT(fileType != FileType::None);
  return MakeSearchPath(ConstructSearchVector(fileType));
}

void SessionImpl::DirectoryWalk(const PathName & directory, const PathName & pathPattern, vector<PathName> & paths)
{
  if (pathPattern.Empty())
  {
    paths.push_back(directory);
  }
  else
  {
    ExpandPathPattern(directory, pathPattern, paths);
  }
  unique_ptr<DirectoryLister> dirLister = DirectoryLister::Open(directory, nullptr, (int)DirectoryLister::Options::DirectoriesOnly);
  DirectoryEntry entry;
  vector<PathName> subdirs;
  while (dirLister->GetNext(entry))
  {
    MIKTEX_ASSERT(entry.isDirectory);
    PathName subdir(directory);
    subdir /= entry.name;
    subdirs.push_back(subdir);
  }
  dirLister->Close();
  // TODO: async?
  for (const PathName & subdir : subdirs)
  {
    if (!pathPattern.Empty())
    {
      ExpandPathPattern(subdir, pathPattern, paths);
    }
    // RECURSION
    DirectoryWalk(subdir, pathPattern, paths);
  }
}

void SessionImpl::ExpandPathPattern(const PathName & rootDirectory, const PathName & pathPattern, vector<PathName> & paths)
{
  MIKTEX_ASSERT(!pathPattern.Empty());
  const char * lpszRecursionIndicator = strstr(pathPattern.Get(), RECURSION_INDICATOR);
  if (lpszRecursionIndicator == nullptr)
  {
    // no recursion; check to see whether the path pattern specifies an
    // existing sub directory
    PathName directory(rootDirectory);
    directory /= pathPattern;
    if (Directory::Exists(directory))
    {
      paths.push_back(directory);
    }
  }
  else
  {
    // recursion; decompose the path pattern into two parts:
    // (1) sub directory (2) smaller (possibly empty) path pattern
    string subDir(pathPattern.Get(), lpszRecursionIndicator - pathPattern.Get());
    const char * lpszSmallerPathPattern = lpszRecursionIndicator + RECURSION_INDICATOR_LENGTH;
    for (; IsDirectoryDelimiter(*lpszSmallerPathPattern); ++lpszSmallerPathPattern)
    {
    };
    PathName directory(rootDirectory);
    directory /= subDir;
    // check to see whether the sub directory exists
    if (Directory::Exists(directory))
    {
      DirectoryWalk(directory, lpszSmallerPathPattern, paths);
    }
  }
}

vector<PathName> SessionImpl::ExpandPathPatterns(const char * lpszToBeExpanded)
{
  vector<PathName> pathPatterns = SplitSearchPath(lpszToBeExpanded);
  vector<PathName> paths;
  for (const PathName & pattern : pathPatterns)
  {
    PathName comparablePathPattern(pattern);
    comparablePathPattern.TransformForComparison();
    SearchPathDictionary::const_iterator it2 = expandedPathPatterns.find(comparablePathPattern.Get());
    if (it2 == expandedPathPatterns.end())
    {
      vector<PathName> paths2;
      ExpandPathPattern("", pattern, paths2);
      expandedPathPatterns[comparablePathPattern.Get()] = paths2;
      paths.insert(paths.end(), paths2.begin(), paths2.end());
    }
    else
    {
      paths.insert(paths.end(), it2->second.begin(), it2->second.end());
    }
  }
  return paths;
}

inline void Combine(vector<PathName> & paths, const vector<PathName> & toBeAppended)
{
  if (toBeAppended.empty())
  {
    return;
  }
  if (paths.empty())
  {
    paths = toBeAppended;
    return;
  }
  vector<PathName> result;
  result.reserve(paths.size() * toBeAppended.size());
  for (const PathName & p1 : paths)
  {
    for (const PathName & p2 : toBeAppended)
    {
      PathName path(p1);
      path.Append(p2.Get(), false);
      result.push_back(path);
    }
  }
  paths = result;
}

inline void Combine(vector<PathName> & paths, const string & path)
{
  vector<PathName> toBeAppended;
  toBeAppended.push_back(path);
  Combine(paths, toBeAppended);
}

inline vector<PathName> ExpandBracesHelper(const char * & lpszToBeExpanded)
{
  MIKTEX_ASSERT(*lpszToBeExpanded == '{');
  ++lpszToBeExpanded;
  string str;
  vector<PathName> result;
  vector<PathName> subtotal;
  for (; *lpszToBeExpanded != 0 && *lpszToBeExpanded != '}'; ++lpszToBeExpanded)
  {
    if (*lpszToBeExpanded == '{')
    {
      Combine(subtotal, str);
      str = "";
      Combine(subtotal, ExpandBracesHelper(lpszToBeExpanded));
      if (*lpszToBeExpanded != '}')
      {
	// todo
      }
    }
    else if (*lpszToBeExpanded == ',')
    {
      Combine(subtotal, str);
      str = "";
      result.insert(result.end(), subtotal.begin(), subtotal.end());
      subtotal.clear();
    }
    else
    {
      str += *lpszToBeExpanded;
    }
  }
  Combine(subtotal, str);
  result.insert(result.end(), subtotal.begin(), subtotal.end());
  return result;
}

// ab{cd,ef}gh{ij,kl}mn =>
// abcdghijmn
// abcdghklmn
// abefghijmn
// abefghklmn
void SessionImpl::ExpandBraces(const char * lpszToBeExpanded, vector<PathName> & paths)
{
  string str;
  vector<PathName> result;
  for (; *lpszToBeExpanded != 0; ++lpszToBeExpanded)
  {
    if (*lpszToBeExpanded == '{')
    {
      Combine(result, str);
      str = "";
      Combine(result, ExpandBracesHelper(lpszToBeExpanded));
      if (*lpszToBeExpanded != '}')
      {
	// todo
      }
    }
    else
    {
      str += *lpszToBeExpanded;
    }
  }
  Combine(result, str);
  paths.insert(paths.end(), result.begin(), result.end());
}

vector<PathName> SessionImpl::ExpandBraces(const char * lpszToBeExpanded)
{
  vector<PathName> paths = ExpandRootDirectories(lpszToBeExpanded);
  vector<PathName> result;
  for (const PathName & path : paths)
  {
    ExpandBraces(path.Get(), result);
  }
  return result;
}
