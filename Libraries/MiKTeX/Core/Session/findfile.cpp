/* findfile.cpp: finding files

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

#include <miktex/Configuration/ConfigNames>
#include <miktex/Core/BufferSizes>
#include <miktex/Core/Paths>

#include "internal.h"

#include "Fndb/FileNameDatabase.h"
#include "Session/SessionImpl.h"
#include "Utils/CoreStopWatch.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Util;

void SessionImpl::SetFindFileCallback(IFindFileCallback* callback)
{
  findFileCallback = callback;
}

bool SessionImpl::CheckCandidate(PathName& path, const char* fileInfo, IFindFileCallback* callback)
{
  bool found = false;
  if (IsMpmFile(path.GetData()))
  {
    PathName trigger(Utils::GetRelativizedPath(path.GetData(), MPM_ROOT_PATH));
    PathName installRoot;
    if (fileInfo != nullptr && callback != nullptr && callback->InstallPackage(fileInfo, trigger, installRoot))
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

bool SessionImpl::SearchFileSystem(const string& fileName, const char* pathPattern, bool all, vector<PathName>& result, IFindFileCallback* callback)
{
  MIKTEX_ASSERT(result.empty());

  if (IsMpmFile(pathPattern))
  {
    return false;
  }

  trace_filesearch->WriteLine("core", fmt::format(T_("file system search: fileName={0}, pathPattern={1}"), Q_(fileName), Q_(pathPattern)));

  vector<PathName> directories;

  PathName comparablePathPattern(pathPattern);
  comparablePathPattern.TransformForComparison();

  SearchPathDictionary::const_iterator it = expandedPathPatterns.find(comparablePathPattern.ToString());

  if (it == expandedPathPatterns.end())
  {
    ExpandPathPattern(PathName(), PathName(pathPattern), directories);
    expandedPathPatterns[comparablePathPattern.ToString()] = directories;
  }
  else
  {
    directories = it->second;
  }

  bool found = false;

  for (vector<PathName>::const_iterator it = directories.begin(); (!found || all) && it != directories.end(); ++it)
  {
    PathName path(*it, PathName(fileName));
    if (CheckCandidate(path, nullptr, callback))
    {
      found = true;
      result.push_back(path);
    }
  }

  return found;
}

bool SessionImpl::FindFileInDirectories(const string& fileName, const vector<PathName>& pathPatterns, bool all, bool useFndb, bool searchFileSystem, vector<PathName>& result, IFindFileCallback* callback)
{
  CoreStopWatch stopWatch(fmt::format("find file {}", Q_(fileName)));

  MIKTEX_ASSERT(useFndb || searchFileSystem);

  bool found = false;

  if (callback == nullptr)
  {
    callback = findFileCallback;
  }

  // if a fully qualified path name is given, then don't look out any
  // further
  if (PathNameUtil::IsAbsolutePath(fileName))
  {
    PathName path(fileName);
    found = CheckCandidate(path, nullptr, callback);
    if (found)
    {
      result.push_back(path);
    }
    return found;
  }

  // if an explicitly relative path name is given, then don't look out
  // any further: search all working directories
  if (IsExplicitlyRelativePath(fileName.c_str()))
  {
    PathName pathWD;
    for (unsigned idx = 0; (!found || all) && GetWorkingDirectory(idx, pathWD); ++idx)
    {
      PathName path = pathWD / PathName(fileName);
      path.MakeFullyQualified();
      if (CheckCandidate(path, nullptr, callback))
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

  // if a HOME relative path name is given, then don't look out
  // any further: search the HOME directory
  if (!fileName.empty() && fileName[0] == '~' && (fileName.length() == 1 || PathNameUtil::IsDirectoryDelimiter(fileName[1])))
  {
    auto p = Utils::ExpandTilde(fileName);
    if (p.first)
    {
      found = CheckCandidate(p.second, nullptr, callback);
      if (found)
      {
        result.push_back(p.second);
      }
      return found;
    }
  }

  // make use of the file name database
  if (useFndb)
  {
    for (vector<PathName>::const_iterator it = pathPatterns.begin(); (!found || all) && it != pathPatterns.end(); ++it)
    {
      trace_filesearch->WriteLine("core", fmt::format(T_("going to search in FNDB: filename={0}, directory={1}"), Q_(fileName), Q_(it->ToString())));
#if FIND_FILE_DONT_TRIGGER_INSTALLER_IF_ALL
      if (found && all && IsMpmFile(it->GetData()))
      {
        // don't trigger the package installer if we have found a file and if all occurrences are requested
        continue;
      }
#endif
      shared_ptr<FileNameDatabase> fndb = GetFileNameDatabase(it->GetData());
      if (fndb != nullptr)
      {
        // search fndb
        vector<Fndb::Record> records;
        bool foundInFndb = fndb->Search(PathName(fileName), it->ToString(), all, records);
        // we must release the FNDB handle since CheckCandidate() might request an unload of the FNDB
        fndb = nullptr;
        if (foundInFndb)
        {
          for (int idx = 0; idx < records.size(); ++idx)
          {
            if (CheckCandidate(records[idx].path, records[idx].fileNameInfo.c_str(), callback))
            {
              found = true;
              result.push_back(records[idx].path);
            }
          }
        }
      }
      else
      {
        // search the file system because the FNDB does not exist
        trace_filesearch->WriteLine("core", fmt::format(T_("no FNDB found, so going to continue on disk: filename={0}, directory={1}"), Q_(fileName), Q_(*it)));
        vector<PathName> paths;
        if (SearchFileSystem(fileName, it->GetData(), all, paths, callback))
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
  for (vector<PathName>::const_iterator it = pathPatterns.begin(); (!found || all) && it != pathPatterns.end(); ++it)
  {
    // FIXME: why if found and all?
    if (found && all && IsMpmFile(it->GetData()))
    {
      // don't search the virtual MPM directory tree
      continue;
    }
    unsigned rootIdx = TryDeriveTEXMFRoot(*it);
    if (rootIdx != INVALID_ROOT_INDEX && IsManagedRoot(rootIdx))
    {
      // don't search managed root file system (because we insist that an FNDB exists)
      continue;
    }
    shared_ptr<FileNameDatabase> fndb = GetFileNameDatabase(it->GetData());
    if (fndb == nullptr)
    {
      // FNDB does not exist => we already searched the file system (see above)
      continue;
    }
    fndb = nullptr;
    vector<PathName> paths;
    if (SearchFileSystem(fileName, it->GetData(), all, paths, callback))
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

bool SessionImpl::FindFileByType(const string& fileName, FileType fileType, bool all, bool searchFileSystem, bool create, bool renew, vector<PathName>& result, IFindFileCallback* callback)
{
  MIKTEX_ASSERT(result.empty());

  if (callback == nullptr)
  {
    callback = findFileCallback;
  }

  // try to derive the file type
  if (fileType == FileType::None)
  {
    fileType = DeriveFileType(PathName(fileName));
    if (fileType == FileType::None)
    {
      trace_filesearch->WriteLine("core", fmt::format(T_("cannot derive file type from {0}"), Q_(fileName)));
      return false;
    }
  }

  if (renew && (callback == nullptr || !callback->TryCreateFile(PathName(fileName), fileType)))
  {
    return false;
  }

  // construct the search vector
  vector<PathName> pathPatterns = GetDirectoryPatterns(fileType);

  // get the file type information
  const InternalFileTypeInfo* fti = GetInternalFileTypeInfo(fileType);
  MIKTEX_ASSERT(fti != nullptr);

  // check to see whether the file name has a registered file name extension
  PathName extension(PathName(fileName).GetExtension());
  bool hasRegisteredExtension = !extension.Empty()
    && (std::find_if(fti->fileNameExtensions.begin(), fti->fileNameExtensions.end(), [extension](const string& ext) { return extension == PathName(ext); }) != fti->fileNameExtensions.end()
      || std::find_if(fti->alternateExtensions.begin(), fti->alternateExtensions.end(), [extension](const string& ext) { return extension == PathName(ext); }) != fti->alternateExtensions.end());

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
  fileNamesToTry.push_back(PathName(fileName));

  // first round: use the fndb
  for (const PathName& fn : fileNamesToTry)
  {
    if (FindFileInDirectories(fn.ToString(), pathPatterns, all, true, false, result, callback) && !all)
    {
      return true;
    }
  }

  // second round: don't use the FNDB
  if (searchFileSystem)
  {
    for (const PathName& fn : fileNamesToTry)
    {
      if (FindFileInDirectories(fn.ToString(), pathPatterns, all, false, true, result, callback) && !all)
      {
        return true;
      }
    }
  }

  if (create)
  {
    if (result.empty())
    {
      if (callback != nullptr && callback->TryCreateFile(PathName(fileName), fileType))
      {
        FindFileInDirectories(fileName, pathPatterns, all, true, false, result, callback);
      }
    }
    else if ((fileType == FileType::BASE || fileType == FileType::FMT || fileType == FileType::MEM) && callback != nullptr && GetConfigValue(MIKTEX_CONFIG_SECTION_TEXANDFRIENDS, MIKTEX_CONFIG_VALUE_RENEW_FORMATS_ON_UPDATE).GetBool())
    {
      bool renew = false;
      if (IsSharedSetup())
      {
        PathName pathPackagesIni(GetSpecialPath(SpecialPath::CommonInstallRoot), PathName(MIKTEX_PATH_PACKAGES_INI));
        renew = IsNewer(pathPackagesIni, result[0]);
      }
      if (!renew && !IsAdminMode() && (!IsSharedSetup() || GetUserConfigRoot() != GetCommonConfigRoot()))
      {
        PathName pathPackagesIni(GetSpecialPath(SpecialPath::UserInstallRoot), PathName(MIKTEX_PATH_PACKAGES_INI));
        renew = IsNewer(pathPackagesIni, result[0]);
      }
      if (renew)
      {
        trace_filesearch->WriteLine("core", fmt::format(T_("going to renew {0} after update"), result[0]));
        if (callback->TryCreateFile(PathName(fileName), fileType))
        {
          result.clear();
          FindFileInDirectories(fileName, pathPatterns, all, true, false, result, callback);
        }
      }
    }
  }

  return !result.empty();
}

LocateResult MIKTEXTHISCALL SessionImpl::Locate(const string& fileName, const LocateOptions& options)
{
  bool found = false;
  vector<PathName> pathNames;
  if (options.fileType == FileType::None)
  {
    auto pathPatterns = SplitSearchPath(options.searchPath.empty() ? MIKTEX_PATH_TEXMF_PLACEHOLDER : options.searchPath);
    found = FindFileInDirectories(fileName, pathPatterns, options.all, true, false, pathNames, options.callback);
    if (!found && options.searchFileSystem)
    {
      found = FindFileInDirectories(fileName, pathPatterns, options.all, false, true, pathNames, options.callback);
    }
  }
  else
  {
    found = FindFileByType(fileName, options.fileType, options.all, options.searchFileSystem, options.create, options.renew, pathNames, options.callback);
  }
  if (!found)
  {
    return {};
  }
  // Eliminate duplicates.
  vector<PathName> result;
  set<PathName> resultSet;
  for (auto& p : pathNames)
  {
    if (resultSet.find(p) != resultSet.end())
    {
      continue;
    }
    result.push_back(p);
    resultSet.insert(p);
  }
  return { result };
}

bool SessionImpl::FindFile(const string& fileName, const string& searchPath, FindFileOptionSet options, vector<PathName>& result)
{
  LocateOptions locateOptions;
  locateOptions.all = options[FindFileOption::All];
  locateOptions.create = options[FindFileOption::Create];
  locateOptions.renew = options[FindFileOption::Renew];
  locateOptions.searchFileSystem = options[FindFileOption::SearchFileSystem];
  locateOptions.searchPath = searchPath;
  result = Locate(fileName, locateOptions).pathNames;
  return !result.empty();
}

bool SessionImpl::FindFile(const string& fileName, const string& searchPath, FindFileOptionSet options, PathName& result)
{
  MIKTEX_ASSERT(!options[FindFileOption::All]);
  LocateOptions locateOptions;
  locateOptions.create = options[FindFileOption::Create];
  locateOptions.renew = options[FindFileOption::Renew];
  locateOptions.searchFileSystem = options[FindFileOption::SearchFileSystem];
  locateOptions.searchPath = searchPath;
  if (auto locateResult = Locate(fileName, locateOptions); !locateResult.pathNames.empty())
  {
    result = locateResult.pathNames[0];
    return true;
  }
  return false;
}

bool SessionImpl::FindFile(const string& fileName, FileType fileType, FindFileOptionSet options, vector<PathName>& result)
{
  LocateOptions locateOptions;
  locateOptions.all = options[FindFileOption::All];
  locateOptions.create = options[FindFileOption::Create];
  locateOptions.fileType = fileType;
  locateOptions.renew = options[FindFileOption::Renew];
  locateOptions.searchFileSystem = options[FindFileOption::SearchFileSystem];
  result = Locate(fileName, locateOptions).pathNames;
  return !result.empty();
}

bool SessionImpl::FindFile(const string& fileName, FileType fileType, FindFileOptionSet options, PathName& result)
{
  MIKTEX_ASSERT(!options[FindFileOption::All]);
  MIKTEX_ASSERT(!options[FindFileOption::All]);
  LocateOptions locateOptions;
  locateOptions.create = options[FindFileOption::Create];
  locateOptions.fileType = fileType;
  locateOptions.renew = options[FindFileOption::Renew];
  locateOptions.searchFileSystem = options[FindFileOption::SearchFileSystem];
  if (auto locateResult = Locate(fileName, locateOptions); !locateResult.pathNames.empty())
  {
    result = locateResult.pathNames[0];
    return true;
  }
  return false;
}

static const string DEFAULT_PK_NAME_TEMPLATE = "%f.pk";

bool SessionImpl::MakePkFileName(PathName& pkFileName, const string& fontName, int dpi)
{
  string nameTemplate;

  if (!GetSessionValue(MIKTEX_CONFIG_SECTION_CORE, MIKTEX_CONFIG_VALUE_PK_FN_TEMPLATE, nameTemplate, nullptr))
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
        str += fontName;
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

  pkFileName = str;

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

  if (!MakePkFileName(pkFileName, fontName, dpi))
  {
    return false;
  }

  string searchPathTemplate;

  if (!GetSessionValue(MIKTEX_CONFIG_SECTION_CORE, "PKPath", searchPathTemplate, nullptr))
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

  LocateOptions locateOptions;
  locateOptions.searchPath = searchPath;

  if (auto locateResult = Locate(pkFileName.ToString(), locateOptions); !locateResult.pathNames.empty())
  {
    result = locateResult.pathNames[0];
    return true;
  }

  if (mfMode.empty() || StringCompare(mfMode.c_str(), "modeless", true) != 0)
  {
    // RECURSION
    return FindPkFile(fontName, "modeless", dpi, result);
  }

  return false;
}
