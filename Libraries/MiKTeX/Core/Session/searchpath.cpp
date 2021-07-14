/* searchpath.cpp: managing search paths

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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Directory>
#include <miktex/Trace/Trace>
#include <miktex/Trace/TraceStream>

#include "internal.h"

#include "Session/SessionImpl.h"

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

void SessionImpl::ExpandRootDirectories(const string& toBeExpanded, vector<PathName>& pathNames)
{
  if (toBeExpanded.length() >= 2 && toBeExpanded[0] == '%' && (toBeExpanded[1] == 'R' || toBeExpanded[1] == 'r'))
  {
    const char* suffix = toBeExpanded.c_str() + 2;
    if (PathNameUtil::IsDirectoryDelimiter(*suffix))
    {
      ++suffix;
    }
    for (unsigned idx = 0; idx < GetNumberOfTEXMFRoots(); ++idx)
    {
      const RootDirectoryInternals& root = rootDirectories[idx];
      PathName path = root.get_Path();
      path.AppendDirectoryDelimiter();
      path.Append(suffix, false);
      pathNames.push_back(path);
    }
    if (toBeExpanded[1] == 'R')
    {
      PathName path(MPM_ROOT_PATH);
      path.AppendDirectoryDelimiter();
      path.Append(suffix, false);
      pathNames.push_back(path);
    }
  }
  else
  {
    pathNames.push_back(PathName(toBeExpanded));
  }
}

vector<PathName> SessionImpl::ExpandRootDirectories(const string& toBeExpanded)
{
  vector<PathName> result;
  for (const string& s : StringUtil::Split(toBeExpanded, PathNameUtil::PathNameDelimiter))
  {
    ExpandRootDirectories(s, result);
  }
  return result;
}

void SessionImpl::PushBackPath(vector<PathName>& pathNames, const PathName& path)
{
  for (const PathName& path : ExpandBraces(path.ToString()))
  {
    // expand '~'
    if (path[0] == '~' && (path[1] == 0 || PathNameUtil::IsDirectoryDelimiter(path[1])))
    {
      auto p = Utils::ExpandTilde(path.ToString());
      if (p.first)
      {
        if (find(pathNames.begin(), pathNames.end(), p.second) == pathNames.end())
        {
          pathNames.push_back(p.second);
        }
      }
      continue;
    }

    // fully qualified path?
    if (path.IsAbsolute())
    {
      if (find(pathNames.begin(), pathNames.end(), path) == pathNames.end())
      {
        pathNames.push_back(path);
      }
      continue;
    }

    // it is a relative path
    PathName pathFQ;
    for (unsigned idx = 0; GetWorkingDirectory(idx, pathFQ); ++idx)
    {
      if (!pathFQ.IsAbsolute())
      {
        auto trace_error = TraceStream::Open(MIKTEX_TRACE_ERROR);
        trace_error->WriteLine("core", TraceLevel::Error, fmt::format(T_("{0} is not fully qualified"), Q_(pathFQ)));
        continue;
      }
      if (PathName::Compare(path, PathName(CURRENT_DIRECTORY)) != 0)
      {
        pathFQ /= path;
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
      if (find(pathNames.begin(), pathNames.end(), pathFQ) == pathNames.end())
      {
        pathNames.push_back(pathFQ);
      }
    }
  }
}

vector<PathName> SessionImpl::SplitSearchPath(const string& searchPath)
{
  vector<PathName> result;
  for (const string& s : StringUtil::Split(searchPath, PathNameUtil::PathNameDelimiter))
  {
    PushBackPath(result, PathName(s));
  }
  return result;
}

MIKTEXINTERNALFUNC(string) MakeSearchPath(const vector<PathName>& pathNames)
{
  string searchPath;
  for (const PathName& path : pathNames)
  {
    if (!searchPath.empty())
    {
      searchPath += PATH_DELIMITER;
    }
    searchPath += path.ToString();
  }
  return searchPath;
}

void SessionImpl::TraceDirectoryPatterns(const std::string& fileType, const vector<PathName>& vec)
{
  if (!trace_filesearch->IsEnabled("core", TraceLevel::Trace))
  {
    return;
  }
  trace_filesearch->WriteLine("core", TraceLevel::Trace, fmt::format(T_("directory patterns for {0}:"), fileType));
  unsigned ord = 0;
  for (vector<PathName>::const_iterator it = vec.begin(); it != vec.end(); ++it, ++ord)
  {
    trace_filesearch->WriteLine("core", TraceLevel::Trace, fmt::format("  {0}: {1}", ord, *it));
  }
}

vector<PathName> SessionImpl::GetDirectoryPatterns(FileType fileType)
{
  InternalFileTypeInfo* fti = GetInternalFileTypeInfo(fileType);
  if (fti->pathPatterns.empty())
  {
    for (const string& env : fti->envVarNames)
    {
      string searchPath;
      if (Utils::GetEnvironmentString(env, searchPath))
      {
        for (const string& s : StringUtil::Split(searchPath, PathNameUtil::PathNameDelimiter))
        {
          PushBackPath(fti->pathPatterns, PathName(s));
        }
      }
    }
    for (const string& s : fti->searchPath)
    {
      PushBackPath(fti->pathPatterns, PathName(s));
    }
    TraceDirectoryPatterns(fti->fileTypeString, fti->pathPatterns);
  }
  return fti->pathPatterns;
}

string SessionImpl::GetExpandedSearchPath(FileType fileType)
{
  MIKTEX_ASSERT(fileType != FileType::None);
  return MakeSearchPath(GetDirectoryPatterns(fileType));
}

void SessionImpl::DirectoryWalk(const PathName& directory, const PathName& pathPattern, vector<PathName>& paths)
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
  for (const PathName& subdir : subdirs)
  {
    if (!pathPattern.Empty())
    {
      ExpandPathPattern(subdir, pathPattern, paths);
    }
    // RECURSION
    DirectoryWalk(subdir, pathPattern, paths);
  }
}

void SessionImpl::ExpandPathPattern(const PathName& rootDirectory, const PathName& pathPattern, vector<PathName>& paths)
{
  MIKTEX_ASSERT(!pathPattern.Empty());
  const char* lpszRecursionIndicator = strstr(pathPattern.GetData(), RECURSION_INDICATOR);
  if (lpszRecursionIndicator == nullptr || (rootDirectory.Empty() && lpszRecursionIndicator == pathPattern.GetData()))
  {
    // no recursion; check to see whether the path pattern specifies an
    // existing sub directory
    PathName directory(rootDirectory);
    directory /= pathPattern;
    if (!IsMpmFile(directory.GetData()) && Directory::Exists(directory))
    {
      paths.push_back(directory);
    }
  }
  else
  {
    // recursion; decompose the path pattern into two parts:
    // (1) sub directory (2) smaller (possibly empty) path pattern
    string subDir(pathPattern.GetData(), lpszRecursionIndicator - pathPattern.GetData());
    const char* lpszSmallerPathPattern = lpszRecursionIndicator + RECURSION_INDICATOR_LENGTH;
    for (; PathNameUtil::IsDirectoryDelimiter(*lpszSmallerPathPattern); ++lpszSmallerPathPattern)
    {
    };
    PathName directory(rootDirectory);
    directory /= subDir;
    // check to see whether the sub directory exists
    if (!IsMpmFile(directory.GetData()) && Directory::Exists(directory))
    {
      DirectoryWalk(directory, PathName(lpszSmallerPathPattern), paths);
    }
  }
}

vector<PathName> SessionImpl::ExpandPathPatterns(const string& toBeExpanded)
{
  vector<PathName> pathNames;
  for (const PathName& pattern : SplitSearchPath(toBeExpanded))
  {
    PathName comparablePathPattern(pattern);
    comparablePathPattern.TransformForComparison();
    SearchPathDictionary::const_iterator it2 = expandedPathPatterns.find(comparablePathPattern.GetData());
    if (it2 == expandedPathPatterns.end())
    {
      vector<PathName> paths2;
      ExpandPathPattern(PathName(), pattern, paths2);
      expandedPathPatterns[comparablePathPattern.GetData()] = paths2;
      pathNames.insert(pathNames.end(), paths2.begin(), paths2.end());
    }
    else
    {
      pathNames.insert(pathNames.end(), it2->second.begin(), it2->second.end());
    }
  }
  return pathNames;
}

inline void Combine(vector<PathName>& pathNames, const vector<PathName>& toBeAppended)
{
  if (toBeAppended.empty())
  {
    return;
  }
  if (pathNames.empty())
  {
    pathNames = toBeAppended;
    return;
  }
  vector<PathName> result;
  result.reserve(pathNames.size() * toBeAppended.size());
  for (const PathName& p1 : pathNames)
  {
    for (const PathName& p2 : toBeAppended)
    {
      PathName path(p1);
      path.Append(p2.GetData(), false);
      result.push_back(path);
    }
  }
  pathNames = result;
}

inline void Combine(vector<PathName>& paths, const string& path)
{
  vector<PathName> toBeAppended;
  toBeAppended.push_back(PathName(path));
  Combine(paths, toBeAppended);
}

inline vector<PathName> ExpandBracesHelper(const char*& lpszToBeExpanded)
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
        // TODO
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
void SessionImpl::ExpandBraces(const string& toBeExpanded, vector<PathName>& pathNames)
{
  string str;
  vector<PathName> result;
  for (const char* lpszToBeExpanded = toBeExpanded.c_str(); *lpszToBeExpanded != 0; ++lpszToBeExpanded)
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
  pathNames.insert(pathNames.end(), result.begin(), result.end());
}

vector<PathName> SessionImpl::ExpandBraces(const string& toBeExpanded)
{
  vector<PathName> result;
  for (const PathName& path : ExpandRootDirectories(toBeExpanded))
  {
    ExpandBraces(path.ToString(), result);
  }
  return result;
}
