/* LogFile.cpp:

   Copyright (C) 2000-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include "internal.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;
using namespace std;

bool SkipPrefix(const string & str, const char * lpszPrefix, string & str2)
{
  size_t n = StrLen(lpszPrefix);
  if (str.compare(0, n, lpszPrefix) != 0)
  {
    return false;
  }
  str2 = str.c_str() + n;
  return true;
}


void LogFile::AddFile(const PathName & path)
{
  shared_ptr<Session> session = Session::Get();
  string fileName;
  if (SkipPrefix(path.Get(), T_("texmf/"), fileName)
    || SkipPrefix(path.Get(), T_("texmf\\"), fileName)
    || SkipPrefix(path.Get(), T_("./texmf/"), fileName)
    || SkipPrefix(path.Get(), T_(".\\texmf\\"), fileName))
  {
    PathName absPath(session->GetSpecialPath(SpecialPath::InstallRoot));
    absPath /= fileName;
    files.insert(absPath);
  }
}

void LogFile::AddPackages()
{
  // add packages.ini
  AddFile(PathName(T_("texmf"), MIKTEX_PATH_PACKAGES_INI));

  // iterate over all known packages
  PackageInfo pi;
  std::shared_ptr<MiKTeX::Packages::PackageManager> pManager(PackageManager::Create());
  unique_ptr<PackageIterator> pIter(pManager->CreateIterator());
  while (pIter->GetNext(pi))
  {
    // add .tpm file
    PathName tpmFile(T_("texmf"), MIKTEX_PATH_PACKAGE_DEFINITION_DIR);
    tpmFile /= pi.deploymentName;
    tpmFile.AppendExtension(MIKTEX_PACKAGE_DEFINITION_FILE_SUFFIX);
    AddFile(tpmFile);

    // check if the package is installed
    if (!pi.IsInstalled())
    {
      continue;
    }

    // add files to the file set
    vector<string>::const_iterator it;
    for (it = pi.runFiles.begin(); it != pi.runFiles.end(); ++it)
    {
      AddFile(*it);
    }
    for (it = pi.docFiles.begin(); it != pi.docFiles.end(); ++it)
    {
      AddFile(*it);
    }
    for (it = pi.sourceFiles.begin(); it != pi.sourceFiles.end(); ++it)
    {
      AddFile(*it);
    }
  }
  pIter->Dispose();
}

void LogFile::ReadLogFile()
{
  // open the log file
  StreamReader stream(logFileName);

  // read lines
  string line;
  enum Section { None, Files, HKCU, HKLM };
  Section section = None;
  while (stream.ReadLine(line) && !pService->IsCancelled())
  {
    size_t cchLine = line.length();
    if (cchLine == 0)
    {
      continue;
    }
    if (line[cchLine - 1] == '\n')
    {
      line[--cchLine] = 0;
    }
    if (line == T_("[files]"))
    {
      section = Files;
      continue;
    }
    else if (line == T_("[hkcu]"))
    {
      section = HKCU;
      continue;
    }
    else if (line == "[hklm]")
    {
      section = HKLM;
      continue;
    }
    switch (section)
    {
    case None:
      break;
    case Files:
      files.insert(line);
      break;
    case HKCU:
    case HKLM:
    {
#if defined(MIKTEX_WINDOWS)
      Tokenizer tok(line.c_str(), ";");
      if (tok.GetCurrent() == nullptr)
      {
        continue;
      }
      RegValue regvalue;
      regvalue.hkey = (section == HKCU ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE);
      regvalue.strSubKey = tok.GetCurrent();
      ++tok;
      if (tok.GetCurrent() == nullptr)
      {
        continue;
      }
      regvalue.strValueName = tok.GetCurrent();
      regValues.push_back(regvalue);
#endif
      break;
    }
    }
  }

  stream.Close();
}

void LogFile::Process()
{
  AddPackages();
  ReadLogFile();
  if (pService->IsCancelled())
  {
    return;
  }
  RemoveFiles();
  if (pService->IsCancelled())
  {
    return;
  }
#if defined(MIKTEX_WINDOWS)
  RemoveRegistrySettings();
#endif
  if (pService->IsCancelled())
  {
    return;
  }
}

void LogFile::RemoveFiles()
{
  set<PathName> directories;
  n = 0;
  for (set<PathName>::const_iterator it = files.begin(); it != files.end() && !pService->IsCancelled(); ++it, ++n)
  {
    currentFile = *it;
    if (!((PackageInstallerCallback*)pService)->OnProgress(MiKTeX::Packages::Notification::RemoveFileStart))
    {
      return;
    }
    PathName pathCurDir = *it;
    pathCurDir.RemoveFileSpec();
    directories.insert(pathCurDir);
    if (File::Exists(*it))
    {
      File::Delete(*it, { FileDeleteOption::TryHard });
    }
    if (!((PackageInstallerCallback*)pService)->OnProgress(MiKTeX::Packages::Notification::RemoveFileEnd))
    {
      return;
    }
  }
  for (set<PathName>::const_iterator it = directories.begin(); it != directories.end() && !pService->IsCancelled(); ++it)
  {
    if (Directory::Exists(*it))
    {
      RemoveEmptyDirectoryChain(*it);
    }
  }
}

#if defined(MIKTEX_WINDOWS)
void LogFile::RemoveRegistrySettings()
{
  // <code>sort (regValues.begin(), regValues.end());</code>
  for (vector<RegValue>::const_iterator it = regValues.begin(); it != regValues.end() && !pService->IsCancelled(); ++it)
  {
    HKEY hkeySub;
    if (RegOpenKeyExW(it->hkey, UW_(it->strSubKey), 0, KEY_ALL_ACCESS, &hkeySub) != ERROR_SUCCESS)
    {
      continue;
    }
    RegDeleteValueW(hkeySub, UW_(it->strValueName));
    wchar_t szName[BufferSizes::MaxPath];
    DWORD sizeName = BufferSizes::MaxPath;
    FILETIME fileTime;
    bool del = RegEnumKeyExW(hkeySub, 0, szName, &sizeName, nullptr, nullptr, nullptr, &fileTime) == ERROR_NO_MORE_ITEMS;
    RegCloseKey(hkeySub);
    if (del)
    {
      RegDeleteKeyW(it->hkey, UW_(it->strSubKey));
    }
  }
}
#endif
