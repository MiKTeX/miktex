/* winSetupService.cpp:

   Copyright (C) 2014-2021 Christian Schenk

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

#include "config.h"

#include <miktex/Core/win/HResult>

#include "internal.h"

#include "winSetupService.h"

#include "setup-version.h"

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Trace;
using namespace MiKTeX::Util;

PathName SetupService::GetDefaultCommonInstallDir()
{
  PathName path = Utils::GetFolderPath(CSIDL_PROGRAM_FILES, CSIDL_PROGRAM_FILES, true);
  path /= MIKTEX_PRODUCTNAME_STR;
  return path;
}

PathName SetupService::GetDefaultUserInstallDir()
{
  PathName path;
  PWSTR pwstr;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_UserProgramFiles, KF_FLAG_CREATE, nullptr, &pwstr)))
  {
    MIKTEX_AUTO(CoTaskMemFree(pwstr));
    path = pwstr;
  }
  if (path.Empty())
  {
    path = Utils::GetFolderPath(CSIDL_LOCAL_APPDATA, CSIDL_LOCAL_APPDATA, true);
    path /= "PrograMS";
  }
  path /= MIKTEX_PRODUCTNAME_STR;
  return path;
}

PathName SetupService::GetDefaultPortableRoot()
{
  // TODO
  return PathName("C:/miktex-portable");
}

void winSetupServiceImpl::Initialize()
{
  SetupServiceImpl::Initialize();
}

void winSetupServiceImpl::ULogAddRegValue(HKEY hkey, const string& valueName, const string& value)
{
  if (!uninstStream.is_open())
  {
    return;
  }
  if (hkey == HKEY_LOCAL_MACHINE && section != HKLM)
  {
    uninstStream << "[hklm]" << "\n";
    section = HKLM;
  }
  else if (hkey == HKEY_CURRENT_USER && section != HKCU)
  {
    uninstStream << "[hkcu]" << "\n";
    section = HKCU;
  }
  uninstStream << fmt::format("{0};{1}", valueName, value) << endl;
}

PathName winSetupServiceImpl::CreateProgramFolder()
{
  int cidl = (options.IsCommonSetup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS);
  PathName programFolder = Utils::GetFolderPath(cidl, cidl, true);
  PathName folderName;
  if (options.Task == SetupTask::FinishUpdate)
  {
    for (const string& folderName : { string(MIKTEX_PRODUCTNAME_STR), string(MIKTEX_PRODUCTNAME_STR " " MIKTEX_LEGACY_MAJOR_MINOR_STR) })
    {
      PathName path = programFolder / PathName(folderName);
      if (Directory::Exists(path))
      {
        return path;
      }
    }
    folderName = MIKTEX_PRODUCTNAME_STR;
  }
  else
  {
    folderName = options.FolderName;
  }
  PathName path = programFolder / folderName;
  Directory::Create(path);
  return path;
}

constexpr auto LD_USEDESC = 0x00000001;
constexpr auto LD_USEARGS = 0x00000002;
constexpr auto LD_USEICON = 0x00000004;
constexpr auto LD_USEWORKDIR = 0x00000008;
constexpr auto LD_USEHOTKEY = 0x00000010;
constexpr auto LD_USESHOWCMD = 0x00000020;
constexpr auto LD_IFCOMMON = 0x00000040;

#define EXEPATH(name) fmt::format("%MIKTEX_INSTALL%\\{0}\\{1}", MIKTEX_PATH_BIN_DIR, name)
#define DOCPATH(name) fmt::format("%MIKTEX_INSTALL%\\{0}\\{1}", MIKTEX_PATH_MIKTEX_DOC_DIR, name)

vector<ShellLinkData> winSetupServiceImpl::GetShellLinks()
{
  return vector<ShellLinkData> {
    // "MiKTeX Console"
    {
      false,
        false,
        "",
        "MiKTeX Console",
        EXEPATH(MIKTEX_CONSOLE_EXE),
        LD_USESHOWCMD | LD_USEDESC,
        T_("MiKTeX Console helps you to manage your MiKTeX configuration."),
        "",
        "",
        0,
        "",
        SW_SHOWNORMAL,
        0,
    },

      // "TeXworks"
  {
    false,
    false,
    "",
    "TeXworks",
    EXEPATH(MIKTEX_TEXWORKS_EXE),
    LD_USESHOWCMD | LD_USEDESC,
    T_("TeXworks is a TeX front-end program."),
    "",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

  // "Previewer"
  {
    false,
    true,
    "",
    "DVI Previewer (Yap)",
    EXEPATH(MIKTEX_YAP_EXE),
    LD_USESHOWCMD,
    "",
    "",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance->Settings"
  {
    false,
    true,
    "Maintenance",
    "MiKTeX Settings",
    EXEPATH(MIKTEX_CONSOLE_EXE),
    LD_USESHOWCMD | LD_USEARGS,
    "",
    "--start-page settings",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance->Update"
  {
    false,
    true,
    "Maintenance",
    "MiKTeX Update",
    EXEPATH(MIKTEX_CONSOLE_EXE),
    LD_USESHOWCMD | LD_USEARGS,
    "",
    "--start-page updates",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance->Package Manager"
  {
    false,
    true,
    "Maintenance",
    "MiKTeX Package Manager",
    EXEPATH(MIKTEX_CONSOLE_EXE),
    LD_USESHOWCMD | LD_USEARGS,
    "",
    "--start-page packages",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance (Admin)->Settings (Admin)"
  {
    false,
    true,
    "Maintenance (Admin)",
    "MiKTeX Settings (Admin)",
    EXEPATH(MIKTEX_CONSOLE_ADMIN_EXE),
    LD_IFCOMMON | LD_USESHOWCMD | LD_USEARGS,
    "",
    "--admin --start-page settings",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance (Admin)->Update (Admin)"
  {
    false,
    true,
    "Maintenance (Admin)",
    "MiKTeX Update (Admin)",
    EXEPATH(MIKTEX_CONSOLE_ADMIN_EXE),
    LD_IFCOMMON | LD_USESHOWCMD | LD_USEARGS,
    "",
    "--admin --start-page updates",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance (Admin)->Package Manager (Admin)"
  {
    false,
    true,
    "Maintenance (Admin)",
    "MiKTeX Package Manager (Admin)",
    EXEPATH(MIKTEX_CONSOLE_ADMIN_EXE),
    LD_IFCOMMON | LD_USESHOWCMD | LD_USEARGS,
    "",
    "--admin --start-page packages",
    "",
    0,
    "",
    SW_SHOWNORMAL,
    0,
  },

#if 0
  // "Help->Manual"
      {
        false,
        true,
        "Help",
        "MiKTeX Manual",
        DOCPATH(MIKTEX_MAIN_HELP_FILE),
        LD_USESHOWCMD,
        "",
        "",
        "",
        0,
        "",
        SW_SHOWNORMAL,
        0,
      },
#endif

#if 0
      // "Help->FAQ"
      {
        false,
        true,
        "Help",
        "MiKTeX FAQ",
        DOCPATH(MIKTEX_FAQ_HELP_FILE),
        LD_USESHOWCMD,
        "",
        "",
        "",
        0,
        "",
        SW_SHOWNORMAL,
        0,
      },
#endif

#if 0
      // "MiKTeX on the Web->MiKTeX Project Page"
      {
        true,
        true,
        "MiKTeX on the Web",
        "MiKTeX Project Page",
        MIKTEX_URL_WWW,
        0,
        "",
        "",
        "",
        0,
        "",
        0,
        0,
      },
#endif

#if 0
      // "MiKTeX on the Web->Support"
      {
        true,
        true,
        "MiKTeX on the Web",
        "MiKTeX Support",
        MIKTEX_URL_WWW_SUPPORT,
        0,
        "",
        "",
        "",
        0,
        "",
        0,
        0,
      },
#endif

#if 0
      // "MiKTeX on the Web->Give back"
      {
        true,
        true,
        "MiKTeX on the Web",
        "Give back",
        MIKTEX_URL_WWW_GIVE_BACK,
        0,
        "",
        "",
        "",
        0,
        "",
        0,
        0,
      },
#endif

#if 0
      // "MiKTeX on the Web->Known Issues"
      {
        true,
        true,
        "MiKTeX on the Web",
        "Known Issues",
        MIKTEX_URL_WWW_KNOWN_ISSUES,
        0,
        "",
        "",
        "",
        0,
        "",
        0,
        0,
      },
#endif
  };
}

void winSetupServiceImpl::CreateProgramIcons()
{
  PathName path = CreateProgramFolder();
  for (const ShellLinkData& shlnk : GetShellLinks())
  {
    CreateShellLink(path, shlnk);
  }
}

void winSetupServiceImpl::CreateShellLink(const PathName& pathFolder, const ShellLinkData& ld)
{
  bool dontCreate = ld.pathName.empty();

  if ((ld.flags & LD_IFCOMMON) != 0 && !options.IsCommonSetup)
  {
    // ignore system-wide command if this is a per-user setup
    return;
  }

  PathName pathLink;

  if (!ld.folder.empty())
  {
    PathName pathSubFolder(pathFolder, PathName(ld.folder));
    if (!ld.isObsolete)
    {
      Directory::Create(pathSubFolder);
    }
    pathLink = pathSubFolder;
  }
  else
  {
    pathLink = pathFolder;
  }

  pathLink /= ld.name;
  pathLink.AppendExtension(ld.isUrl ? ".url" : ".lnk");

  if (File::Exists(pathLink))
  {
    LogInternal(TraceLevel::Info, fmt::format("removing {}...\n", Q_(pathLink)));
    if (!options.IsDryRun)
    {
      File::Delete(pathLink);
    }
  }
  else
  {
    dontCreate = ld.isObsolete;
  }

  if (dontCreate)
  {
    return;
  }

  if (ld.isUrl)
  {
    LogInternal(TraceLevel::Info, fmt::format("creating internet shortcut {}...\n", Q_(pathLink)));
  }
  else
  {
    LogInternal(TraceLevel::Info, fmt::format("creating shell link {}...\n", Q_(pathLink)));
  }

  if (options.IsDryRun)
  {
    return;
  }

  if (ld.isUrl)
  {
    CreateInternetShortcut(pathLink, ld.pathName);
  }
  else
  {
    _COM_SMARTPTR_TYPEDEF(IShellLinkW, IID_IShellLinkW);

    IShellLinkWPtr psl;

    HResult hr = CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void**>(&psl));

    if (hr.Failed())
    {
      LogInternal(TraceLevel::Error, fmt::format("IShellLinkW could not be created (hr={0})\n", hr));
      MIKTEX_UNEXPECTED();
    }

    wstring str;

    hr = psl->SetPath(Expand(ld.pathName, str).c_str());

    if (hr.Failed())
    {
      LogInternal(TraceLevel::Error, fmt::format("IShellLinkW::SetPath() failed (hr={0})\n", hr));
      MIKTEX_UNEXPECTED();
    }

    if ((ld.flags & LD_USEARGS) != 0)
    {
      hr = psl->SetArguments(Expand(ld.args, str).c_str());
      if (hr.Failed())
      {
        LogInternal(TraceLevel::Error, fmt::format("IShellLinkW::SetArguments() failed (hr={0})\n", hr));
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEDESC) != 0)
    {
      hr = psl->SetDescription(UW_(ld.description));
      if (hr.Failed())
      {
        LogInternal(TraceLevel::Error, fmt::format("IShellLinkW::SetDescription() failed (hr={0})\n", hr));
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEICON) != 0)
    {
      hr = psl->SetIconLocation(Expand(ld.iconPath, str).c_str(), ld.iconIndex);
      if (hr.Failed())
      {
        LogInternal(TraceLevel::Error, fmt::format("IShellLinkW::SetIconLocation() failed (hr={0})\n", hr));
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEWORKDIR) != 0)
    {
      hr = psl->SetWorkingDirectory(Expand(ld.workingDir, str).c_str());
      if (hr.Failed())
      {
        LogInternal(TraceLevel::Error, fmt::format("IShellLinkW::SetWorkingDirectory() failed (hr={0})\n", hr));
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USESHOWCMD) != 0)
    {
      hr = psl->SetShowCmd(ld.showCmd);
      if (hr.Failed())
      {
        LogInternal(TraceLevel::Error, fmt::format("IShellLinkW::SetShowCmd() failed (hr={0})\n", hr));
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEHOTKEY) != 0)
    {
      hr = psl->SetHotkey(ld.hotKey);
      if (hr.Failed())
      {
        LogInternal(TraceLevel::Error, fmt::format("IShellLinkW::SetHotkey() failed (hr={0})\n", hr));
        MIKTEX_UNEXPECTED();
      }
    }

    IPersistFilePtr ppf;

    hr = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));

    if (hr.Failed())
    {
      LogInternal(TraceLevel::Error, fmt::format("IPersistFile could not be created (hr={0})\n", hr));
      MIKTEX_UNEXPECTED();
    }

    hr = ppf->Save(pathLink.ToWideCharString().c_str(), TRUE);

    if (hr.Failed())
    {
      LogInternal(TraceLevel::Error, fmt::format("IPersistFile::Save() failed (hr={0})\n", hr));
      MIKTEX_UNEXPECTED();
    }
  }

  ULogAddFile(pathLink);
}

void winSetupServiceImpl::CreateInternetShortcut(const PathName& path, const string& url)
{
  _COM_SMARTPTR_TYPEDEF(IUniformResourceLocatorW, IID_IUniformResourceLocatorW);

  IUniformResourceLocatorWPtr pURL;

  HResult hr = CoCreateInstance(CLSID_InternetShortcut, 0, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocatorW, reinterpret_cast<void**>(&pURL));

  if (hr.Failed())
  {
    LogInternal(TraceLevel::Error, fmt::format("IUniformResourceLocator could not be created (hr={0})\n", hr));
    MIKTEX_UNEXPECTED();
  }

  hr = pURL->SetURL(UW_(url), 0);

  if (hr.Failed())
  {
    LogInternal(TraceLevel::Error, fmt::format("IUniformResourceLocatorW::SetURL() failed (hr={0})\n", hr));
    MIKTEX_UNEXPECTED();
  }

  IPersistFilePtr pPF;

  hr = pURL->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&pPF));

  if (hr.Failed())
  {
    LogInternal(TraceLevel::Error, fmt::format("IPersistFile could not be created (hr={0})\n", hr));
    MIKTEX_UNEXPECTED();
  }

  hr = pPF->Save(path.ToWideCharString().c_str(), TRUE);

  if (hr.Failed())
  {
    LogInternal(TraceLevel::Error, fmt::format("IPersistFile::Save() failed (hr={0})\n", hr));
    MIKTEX_UNEXPECTED();
  }
}

constexpr auto UNINST_HELP_LINK = "https://miktex.org/support";
constexpr auto UNINST_PUBLISHER = MIKTEX_COMP_COMPANY_STR;
constexpr auto UNINST_DISPLAY_VERSION = MIKTEX_DISPLAY_VERSION_STR;
#define UNINST_DISPLAY_STRING                       \
  (options.Task == SetupTask::PrepareMiKTeXDirect   \
   ? UNINST_DISPLAY_NAME_MIKTEXDIRECT               \
   : UNINST_DISPLAY_NAME)
constexpr auto UNINST_ABOUT_URL = "https://miktex.org/about";
constexpr auto UNINST_UPDATE_URL = "https://miktex.org";
constexpr auto UNINST_README = UNINST_HELP_LINK;

constexpr auto UNINST_DISPLAY_NAME = MIKTEX_PRODUCTNAME_STR;
constexpr auto UNINST_DISPLAY_NAME_MIKTEXDIRECT = "MiKTeXDirect" " " MIKTEX_DISPLAY_VERSION_STR;

constexpr auto REGSTR_PATH_UNINSTALL_U = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

#define UNINST_REG_PATH fmt::format("{0}\\{1}", REGSTR_PATH_UNINSTALL_U, options.Task == SetupTask::PrepareMiKTeXDirect ? UNINST_DISPLAY_NAME_MIKTEXDIRECT : UNINST_DISPLAY_NAME)

#define UNINST_HKEY_ROOT (options.IsCommonSetup ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER)

void winSetupServiceImpl::RegisterUninstaller()
{
  // make uninstall command line
  PathName miktexConsole = GetInstallRoot() / PathName(MIKTEX_PATH_BIN_DIR) / PathName(options.IsCommonSetup ? MIKTEX_CONSOLE_ADMIN_EXE : MIKTEX_CONSOLE_EXE);
  string commandLine = Q_(miktexConsole);
  if (options.IsCommonSetup)
  {
    commandLine += " --admin";
  }
  commandLine += " --start-page cleanup";

  // make icon path
  PathName iconPath = miktexConsole;
  iconPath.Append(",0", false);

  // create registry key
  HKEY hkey;
  DWORD disp;
  LONG result = RegCreateKeyExW(UNINST_HKEY_ROOT, UW_(UNINST_REG_PATH), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hkey, &disp);
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegCreateKeyExW", result);
  }
  AutoHKEY autoHKEY(hkey);

  // set values
  PathName installRoot(GetInstallRoot());
  AddUninstallerRegValue(hkey, "Comments", T_("MiKTeX is a scalable TeX distribution for Windows, Linux and macOS."));
  AddUninstallerRegValue(hkey, "DisplayIcon", iconPath.ToString());
  AddUninstallerRegValue(hkey, "DisplayName", UNINST_DISPLAY_STRING);
  AddUninstallerRegValue(hkey, "DisplayVersion", UNINST_DISPLAY_VERSION);
  AddUninstallerRegValue(hkey, "HelpLink", UNINST_HELP_LINK);
  AddUninstallerRegValue(hkey, "InstallLocation", installRoot.ToString());
  AddUninstallerRegValue(hkey, "NoModify", 1);
  AddUninstallerRegValue(hkey, "NoRepair", 1);
  AddUninstallerRegValue(hkey, "Publisher", UNINST_PUBLISHER);
  AddUninstallerRegValue(hkey, "Readme", UNINST_README);
  AddUninstallerRegValue(hkey, "UninstallString", commandLine);
  AddUninstallerRegValue(hkey, "UrlInfoAbout", UNINST_ABOUT_URL);
  AddUninstallerRegValue(hkey, "UrlUpdateInfo", UNINST_UPDATE_URL);
}

void winSetupServiceImpl::AddUninstallerRegValue(HKEY hkey, const string& valueName, const string& value)
{
  wstring wideValue = UW_(value);
  LONG result = RegSetValueExW(hkey, UW_(valueName), 0, REG_SZ, reinterpret_cast<const BYTE*>(wideValue.c_str()), static_cast<DWORD>((wideValue.length() + 1) * sizeof(wchar_t)));
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegSetValueExW", result);
  }
  ULogAddRegValue(UNINST_HKEY_ROOT, UNINST_REG_PATH, value);
}

void winSetupServiceImpl::AddUninstallerRegValue(HKEY hkey, const string& valueName, DWORD value)
{
  LONG result = RegSetValueExW(hkey, UW_(valueName), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), static_cast<DWORD>(sizeof(value)));
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegSetValueExW", result);
  }
  ULogAddRegValue(UNINST_HKEY_ROOT, UNINST_REG_PATH, valueName);
}

void winSetupServiceImpl::UnregisterShellFileTypes()
{
  shared_ptr<Session> session = Session::Get();
  PathName initexmfExe;
  if (!session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmfExe))
  {
    MIKTEX_UNEXPECTED();
  }
  vector<string> args{ initexmfExe.GetFileNameWithoutExtension().ToString(), "--unregister-shell-file-types" };
  if (session->IsAdminMode())
  {
    args.push_back("--admin");
  }
  Process::Run(initexmfExe, args);
}

void winSetupServiceImpl::UnregisterPath(bool shared)
{
  constexpr auto REGSTR_PATH_CURRENTCONTROLSET_U = "System\\CurrentControlSet";
  constexpr auto REGSTR_KEY_ENVIRONMENT_USER = "Environment";

#define REGSTR_KEY_ENVIRONMENT_COMMON fmt::format("{0}\\Control\\Session Manager\\Environment", REGSTR_PATH_CURRENTCONTROLSET_U)

  HKEY hkey;

  string subkey(shared
    ? REGSTR_KEY_ENVIRONMENT_COMMON
    : REGSTR_KEY_ENVIRONMENT_USER);

  LONG result = RegOpenKeyExW((shared ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER), UW_(subkey), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey);

  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegOpenKeyExW", result);
  }

  AutoHKEY autoHKEY(hkey);

  DWORD type;
  CharBuffer<wchar_t> value(32 * 1024);
  DWORD valueSize = static_cast<DWORD>(value.GetCapacity() * sizeof(wchar_t));

  result = RegQueryValueExW(hkey, L"Path", nullptr, &type, reinterpret_cast<LPBYTE>(value.GetData()), &valueSize);

  bool havePath = result == ERROR_SUCCESS;

  if (!havePath)
  {
    if (result != ERROR_FILE_NOT_FOUND)
    {
      MIKTEX_FATAL_WINDOWS_RESULT("RegQueryValueExW", result);
    }
  }
  else
  {
    string path = WU_(value.GetData());
    if (RemoveBinDirectoriesFromPath(path))
    {
      CharBuffer<wchar_t> wpath(UW_(path.c_str()));
      result = RegSetValueExW(hkey, L"Path", 0, type, reinterpret_cast<const BYTE*>(wpath.GetData()), static_cast<DWORD>((StrLen(wpath.GetData()) + 1) * sizeof(wpath.GetData()[0])));

      if (result != ERROR_SUCCESS)
      {
        MIKTEX_FATAL_WINDOWS_RESULT("RegSetValueExW", result);
      }

      DWORD_PTR sendMessageResult;

      if (SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>(L"Environment"), SMTO_ABORTIFHUNG, 5000, &sendMessageResult) == 0)
      {
        if (::GetLastError() != ERROR_SUCCESS)
        {
          MIKTEX_FATAL_WINDOWS_ERROR("SendMessageTimeoutW");
        }
      }
    }
  }
}

bool winSetupServiceImpl::RemoveBinDirectoriesFromPath(string& path)
{
  shared_ptr<Session> session = Session::Get();
  bool removed = false;
  vector<string> newPath;
  vector<PathName> binDirectories;
  if (!session->IsAdminMode())
  {
    PathName userBinDir = session->GetSpecialPath(SpecialPath::UserInstallRoot);
    userBinDir /= MIKTEX_PATH_BIN_DIR;
    userBinDir.AppendDirectoryDelimiter();
    binDirectories.push_back(userBinDir);
  }
  PathName commonBinDir = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
  commonBinDir /= MIKTEX_PATH_BIN_DIR;
  commonBinDir.AppendDirectoryDelimiter();
  binDirectories.push_back(commonBinDir);
  for (const string& entry : StringUtil::Split(path, PathNameUtil::PathNameDelimiter))
  {
    PathName dir(entry);
    dir.AppendDirectoryDelimiter();
    if (std::find(binDirectories.begin(), binDirectories.end(), dir) != binDirectories.end())
    {
      removed = true;
    }
    else
    {
      newPath.push_back(entry);
    }
  }
  if (removed)
  {
    path = StringUtil::Flatten(newPath, PathNameUtil::PathNameDelimiter);
  }
  return removed;
}

void winSetupServiceImpl::RemoveRegistryKeys()
{
  shared_ptr<Session> session = Session::Get();

  if (session->IsAdminMode() && Exists(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_SERIES)))
  {
    RemoveRegistryKey(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_SERIES));
  }

  if (!session->IsAdminMode() && Exists(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_SERIES)))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_SERIES));
  }

  if (session->IsAdminMode()
    && Exists(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_PRODUCT))
    && IsEmpty(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_PRODUCT)))
  {
    RemoveRegistryKey(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_PRODUCT));
  }

  if (session->IsAdminMode()
    && Exists(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_COMPANY))
    && IsEmpty(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_COMPANY)))
  {
    RemoveRegistryKey(HKEY_LOCAL_MACHINE, PathName(MIKTEX_REGPATH_COMPANY));
  }

  if (!session->IsAdminMode() && Exists(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_PRODUCT))
    && IsEmpty(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_PRODUCT)))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_PRODUCT));
  }

  if (!session->IsAdminMode() && Exists(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_COMPANY))
    && IsEmpty(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_COMPANY)))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, PathName(MIKTEX_REGPATH_COMPANY));
  }

  if (!session->IsAdminMode() && Exists(HKEY_CURRENT_USER, PathName(MIKTEX_GPL_GHOSTSCRIPT)))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, PathName(MIKTEX_GPL_GHOSTSCRIPT));
  }
}

void winSetupServiceImpl::RemoveRegistryKey(HKEY hkeyRoot, const PathName& subKey)
{
  AutoHKEY hkeySub;

  LONG result = RegOpenKeyExW(hkeyRoot, subKey.ToWideCharString().c_str(), 0, KEY_READ, &hkeySub);

  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegOpenKeyExW", result);
  }

  wchar_t szName[BufferSizes::MaxPath];
  DWORD size = BufferSizes::MaxPath;

  FILETIME fileTime;

  while ((result = RegEnumKeyExW(hkeySub.Get(), 0, szName, &size, nullptr, nullptr, nullptr, &fileTime)) == ERROR_SUCCESS)
  {
    RemoveRegistryKey(hkeyRoot, PathName(subKey, PathName(szName)));
    size = BufferSizes::MaxPath;
  }

  if (result != ERROR_NO_MORE_ITEMS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegEnumKeyExW", result);
  }

  hkeySub.Reset();

  result = RegDeleteKeyW(hkeyRoot, subKey.ToWideCharString().c_str());

  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegDeleteKeyW", result);
  }
}

bool winSetupServiceImpl::Exists(HKEY hkeyRoot, const PathName& subKey)
{
  AutoHKEY hkeySub;

  LONG result = RegOpenKeyExW(hkeyRoot, subKey.ToWideCharString().c_str(), 0, KEY_READ, &hkeySub);

  if (result != ERROR_SUCCESS)
  {
    if (result == ERROR_FILE_NOT_FOUND)
    {
      return false;
    }
    MIKTEX_FATAL_WINDOWS_RESULT("RegOpenKeyExW", result);
  }

  return true;
}

bool winSetupServiceImpl::IsEmpty(HKEY hkeyRoot, const PathName& subKey)
{
  AutoHKEY hkeySub;

  LONG result = RegOpenKeyExW(hkeyRoot, subKey.ToWideCharString().c_str(), 0, KEY_READ, &hkeySub);

  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegOpenKeyExW", result);
  }

  DWORD nSubKeys;
  DWORD nValues;

  result = RegQueryInfoKeyW(hkeySub.Get(), nullptr, nullptr, nullptr, &nSubKeys, nullptr, nullptr, &nValues, nullptr, nullptr, nullptr, nullptr);

  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegQueryInfoKeyW", result);
  }

  return nSubKeys + nValues == 0;
}
