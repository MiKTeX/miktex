/* LogFile.cpp:

   Copyright (C) 2000-2020 Christian Schenk

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

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

void LogFile::Load(const PathName& logFileName)
{
  setupService->ReportLine(fmt::format(T_("loading {0}..."), logFileName));
  files.clear();
#if defined(MIKTEX_WINDOWS)
  regValues.clear();
#endif

  if (!File::Exists(logFileName))
  {
    return;
  }

  // open the log file
  StreamReader stream(logFileName);

  // read lines
  string line;
  enum Section { None, Files, HKCU, HKLM };
  Section section = None;
  while (stream.ReadLine(line) && !setupService->IsCancelled())
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
    if (line == "[files]")
    {
      section = Files;
      continue;
    }
    else if (line == "[hkcu]")
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
      files.insert(PathName(line));
      break;
    case HKCU:
    case HKLM:
    {
#if defined(MIKTEX_WINDOWS)
      Tokenizer tok(line, ";");
      if (!tok)
      {
        continue;
      }
      RegValue regvalue;
      regvalue.hkey = (section == HKCU ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE);
      regvalue.strSubKey = *tok;
      ++tok;
      if (!tok)
      {
        continue;
      }
      regvalue.strValueName = *tok;
      regValues.push_back(regvalue);
#endif
      break;
    }
    }
  }

  stream.Close();
}

#if defined(MIKTEX_WINDOWS)
void LogFile::RemoveStartMenu()
{
  if (setupService->IsCancelled())
  {
    return;
  }
  shared_ptr<Session> session = Session::Get();
  int cidl = (session->IsAdminMode() ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS);
  PathName prefix = Utils::GetFolderPath(cidl, cidl, true);
  RemoveFiles(prefix);
}
#endif

#if defined(MIKTEX_WINDOWS)
void LogFile::RemoveRegistrySettings()
{
  setupService->ReportLine(fmt::format(T_("removing {0} registry values..."), regValues.size()));
  for (const RegValue& rv :regValues)
  {
    if (setupService->IsCancelled())
    {
      break;
    }
    HKEY hkeySub;
    if (RegOpenKeyExW(rv.hkey, UW_(rv.strSubKey), 0, KEY_ALL_ACCESS, &hkeySub) != ERROR_SUCCESS)
    {
      continue;
    }
    RegDeleteValueW(hkeySub, UW_(rv.strValueName));
    wchar_t szName[BufferSizes::MaxPath];
    DWORD sizeName = BufferSizes::MaxPath;
    FILETIME fileTime;
    bool del = RegEnumKeyExW(hkeySub, 0, szName, &sizeName, nullptr, nullptr, nullptr, &fileTime) == ERROR_NO_MORE_ITEMS;
    RegCloseKey(hkeySub);
    if (del)
    {
      RegDeleteKeyW(rv.hkey, UW_(rv.strSubKey));
    }
  }
}
#endif

void LogFile::RemoveFiles(const PathName& prefix)
{
  setupService->ReportLine(fmt::format(T_("removing MiKTeX files from {0}..."), prefix));
  set<PathName> directories;
  for (const PathName& file : files)
  {
    if (setupService->IsCancelled())
    {
      break;
    }
    if (!prefix.Empty() && !Utils::IsParentDirectoryOf(prefix, file))
    {
      continue;
    }
    if (!((PackageInstallerCallback*)setupService)->OnProgress(MiKTeX::Packages::Notification::RemoveFileStart))
    {
      return;
    }
    PathName pathCurDir = file;
    pathCurDir.RemoveFileSpec();
    directories.insert(pathCurDir);
    if (File::Exists(file))
    {
      File::Delete(file, { FileDeleteOption::TryHard });
    }
    if (!((PackageInstallerCallback*)setupService)->OnProgress(MiKTeX::Packages::Notification::RemoveFileEnd))
    {
      return;
    }
  }
  for (const PathName& dir : directories)
  {
    if (setupService->IsCancelled())
    {
      break;
    }
    if (Directory::Exists(dir))
    {
      RemoveEmptyDirectoryChain(dir);
    }
  }
}
