/**
 * @file  winSetupService.h
 * @author Christian Schenk
 * @brief Setup service implementation (Windows)
 *
 * @copyright Copyright © 2014-2023 Christian Schenk
 *
 * This file is part of the MiKTeX Setup Library.
 *
 * The MiKTeX Setup Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

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

class winSetupServiceImpl:
    public SetupServiceImpl
{

public:

    virtual void Initialize();

private:

    void ULogAddRegValue(HKEY hkey, const std::string& valueName, const std::string& value) override;
    void CreateProgramIcons() override;
    void RegisterUninstaller() override;
    void UnregisterShellFileTypes() override;
    void RemoveRegistryKeys() override;
    void UnregisterPath(bool shared) override;
    void RemoveRegistryKey(HKEY hkeyRoot, const MiKTeX::Util::PathName& subKey);
    bool Exists(HKEY hkeyRoot, const MiKTeX::Util::PathName& subKey);
    bool IsEmpty(HKEY hkeyRoot, const MiKTeX::Util::PathName& subKey);
    bool winSetupServiceImpl::RemoveBinDirectoriesFromPath(std::string& path);
    void AddUninstallerRegValue(HKEY hkey, const std::string& valueName, const std::string& value);
    void AddUninstallerRegValue(HKEY hkey, const std::string& valueName, DWORD value);
    MiKTeX::Util::PathName CreateProgramFolder();
    void CreateShellLink(const MiKTeX::Util::PathName& pathFolder, const ShellLinkData& ld);
    void CreateInternetShortcut(const MiKTeX::Util::PathName& path, const std::string& url);
    std::vector<ShellLinkData> GetShellLinks();
};

END_INTERNAL_NAMESPACE;
