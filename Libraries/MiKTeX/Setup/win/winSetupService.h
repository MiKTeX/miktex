/* winSetupService.h: internal definitions              -*- C++ -*-

   Copyright (C) 2014-2016 Christian Schenk

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

BEGIN_INTERNAL_NAMESPACE;

struct ShellLinkData
{
  bool isUrl;
  const char * lpszFolder;
  const char * lpszName;
  const char * lpszPathName;
  unsigned long flags;
  const char * lpszDescription;
  const char * lpszArgs;
  const char * lpszIconPath;
  int iconIndex;
  const char * lpszWorkingDir;
  int showCmd;
  WORD hotKey;
};

class winSetupServiceImpl : public SetupServiceImpl
{
public:
  winSetupServiceImpl();

public:
  virtual void Initialize();

private:
  virtual void ULogAddRegValue(HKEY hkey, const std::string & valueName, const std::string & value);

private:
  virtual void CreateProgramIcons();

private:
  virtual void RegisterUninstaller();

private:
  virtual void UnregisterShellFileTypes();

private:
  virtual void RemoveRegistryKeys();

private:
  virtual void UnregisterPath(bool shared);

private:
  void RemoveRegistryKey(HKEY hkeyRoot, const MiKTeX::Core::PathName & subKey);

private:
  bool Exists(HKEY hkeyRoot, const MiKTeX::Core::PathName & subKey);

private:
  bool IsEmpty(HKEY hkeyRoot, const MiKTeX::Core::PathName & subKey);

private:
  bool winSetupServiceImpl::RemoveBinDirFromPath(std::string & path);

private:
  void AddUninstallerRegValue(HKEY hkey, const char * lpszValueName, const char * lpszValue);

private:
  void AddUninstallerRegValue(HKEY hkey, const char * lpszValueName, DWORD value);

private:
  MiKTeX::Core::PathName CreateProgramFolder();

private:
  void CreateShellLink(const MiKTeX::Core::PathName & pathFolder, const ShellLinkData & ld);

private:
  void CreateInternetShortcut(const MiKTeX::Core::PathName & path, const char * lpszUrl);
};

END_INTERNAL_NAMESPACE;
