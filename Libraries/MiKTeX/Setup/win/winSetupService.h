/* winSetupService.h: internal definitions              -*- C++ -*-

   Copyright (C) 2014-2020 Christian Schenk

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

#pragma once

BEGIN_INTERNAL_NAMESPACE;

struct ShellLinkData
{
  bool isUrl;
  bool isObsolete;
  std::string folder;
  std::string name;
  std::string pathName;
  unsigned long flags;
  std::string description;
  std::string args;
  std::string iconPath;
  int iconIndex;
  std::string workingDir;
  int showCmd;
  WORD hotKey;
};

class winSetupServiceImpl :
  public SetupServiceImpl
{
public:
  winSetupServiceImpl();

public:
  virtual void Initialize();

private:
  void ULogAddRegValue(HKEY hkey, const std::string& valueName, const std::string& value) override;

private:
  void CreateProgramIcons() override;

private:
  void RegisterUninstaller() override;

private:
  void UnregisterShellFileTypes() override;

private:
  void RemoveRegistryKeys() override;

private:
  void UnregisterPath(bool shared) override;

private:
  void RemoveRegistryKey(HKEY hkeyRoot, const MiKTeX::Core::PathName& subKey);

private:
  bool Exists(HKEY hkeyRoot, const MiKTeX::Core::PathName& subKey);

private:
  bool IsEmpty(HKEY hkeyRoot, const MiKTeX::Core::PathName& subKey);

private:
  bool winSetupServiceImpl::RemoveBinDirectoriesFromPath(std::string& path);

private:
  void AddUninstallerRegValue(HKEY hkey, const std::string& valueName, const std::string& value);

private:
  void AddUninstallerRegValue(HKEY hkey, const std::string& valueName, DWORD value);

private:
  MiKTeX::Core::PathName CreateProgramFolder();

private:
  void CreateShellLink(const MiKTeX::Core::PathName& pathFolder, const ShellLinkData& ld);

private:
  void CreateInternetShortcut(const MiKTeX::Core::PathName& path, const std::string& url);
};

END_INTERNAL_NAMESPACE;
