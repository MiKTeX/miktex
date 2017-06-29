/* winSetupService.cpp:

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

#include "internal.h"

#include "winSetupService.h"

#include "setup-version.h"

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Util;
using namespace std;

PathName SetupService::GetDefaultCommonInstallDir()
{
  PathName path = Utils::GetFolderPath(CSIDL_PROGRAM_FILES, CSIDL_PROGRAM_FILES, true);
  path /= MIKTEX_PRODUCTNAME_STR " " MIKTEX_MAJOR_MINOR_STR;
  return path;
}

PathName SetupService::GetDefaultUserInstallDir()
{
  PathName path;
  if (WindowsVersion::IsWindowsVistaOrGreater())
  {
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
// borrowed from ShlObj.h
typedef enum
{
    KF_FLAG_CREATE          = 0x00008000,
} KNOWN_FOLDER_FLAG;
#endif
    DllProc4<HRESULT, REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*> SHGetKnownFolderPath_("Shell32.dll", "SHGetKnownFolderPath");
    PWSTR pwstr;
    if (SUCCEEDED(SHGetKnownFolderPath_(FOLDERID_UserProgramFiles, KF_FLAG_CREATE, nullptr, &pwstr)))
    {
      AutoCoTaskMem xxx(pwstr);
      path = pwstr;
    }
  }
  if (path.Empty())
  {
    path = Utils::GetFolderPath(CSIDL_LOCAL_APPDATA, CSIDL_LOCAL_APPDATA, true);
    path /= "PrograMS";
  }
  path /= MIKTEX_PRODUCTNAME_STR " " MIKTEX_MAJOR_MINOR_STR;
  return path;
}

PathName SetupService::GetDefaultPortableRoot()
{
  // TODO
  return "C:/miktex-portable";
}

winSetupServiceImpl::winSetupServiceImpl()
{
  if (FAILED(CoInitialize(0)))
  {
    MIKTEX_FATAL_ERROR(T_("COM could not be initialized"));
  }
  if (options.FolderName.Empty())
  {
    options.FolderName = MIKTEX_PRODUCTNAME_STR " " MIKTEX_MAJOR_MINOR_STR;
  }
}

void winSetupServiceImpl::Initialize()
{
  SetupServiceImpl::Initialize();
}

void winSetupServiceImpl::ULogAddRegValue(HKEY hkey, const string & valueName, const string & value)
{
  if (!uninstStream.IsOpen())
  {
    return;
  }
  if (hkey == HKEY_LOCAL_MACHINE && section != HKLM)
  {
    uninstStream.WriteLine("[hklm]");
    section = HKLM;
  }
  else if (hkey == HKEY_CURRENT_USER && section != HKCU)
  {
    uninstStream.WriteLine("[hkcu]");
    section = HKCU;
  }
  uninstStream.WriteFormattedLine("%s;%s", valueName.c_str(), value.c_str());
}

PathName winSetupServiceImpl::CreateProgramFolder()
{
  int cidl = (options.IsCommonSetup ? CSIDL_COMMON_PROGRAMS : CSIDL_PROGRAMS);
  PathName path = Utils::GetFolderPath(cidl, cidl, true);
  path /= options.FolderName;
  Directory::Create(path);
  return path;
}

#define LD_USEDESC 0x00000001
#define LD_USEARGS 0x00000002
#define LD_USEICON 0x00000004
#define LD_USEWORKDIR 0x00000008
#define LD_USEHOTKEY 0x00000010
#define LD_USESHOWCMD 0x00000020
#define LD_IFCOMMON 0x00000040

#define EXEPATH(name) \
 "%MIKTEX_INSTALL%\\" MIKTEX_PATH_BIN_DIR "\\" name

#define INTEXEPATH(name) \
 "%MIKTEX_INSTALL%\\" MIKTEX_PATH_INTERNAL_BIN_DIR "\\" name

#define DOCPATH(name) \
 "%MIKTEX_INSTALL%\\" MIKTEX_PATH_MIKTEX_DOC_DIR "\\" name

BEGIN_ANONYMOUS_NAMESPACE;

const ShellLinkData shellLinks[] = {

  // "TeXworks"
  {
    false,
    nullptr,
    "TeXworks",
    EXEPATH(MIKTEX_TEXWORKS_EXE),
    LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Previewer"
  {
    false,
    nullptr,
    "DVI Previewer (Yap)",
    EXEPATH(MIKTEX_YAP_EXE),
    LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance->Settings"
  {
    false,
    "Maintenance",
    "MiKTeX Settings",
    EXEPATH(MIKTEX_MO_EXE),
    LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance->Update"
  {
    false,
    "Maintenance",
    "MiKTeX Update",
    INTEXEPATH(MIKTEX_COPYSTART_EXE),
    LD_USESHOWCMD | LD_USEARGS | LD_USEICON,
    nullptr,
    "\"" INTEXEPATH(MIKTEX_UPDATE_EXE) "\"",
    INTEXEPATH(MIKTEX_UPDATE_EXE),
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance->Package Manager"
  {
    false,
    "Maintenance",
    "MiKTeX Package Manager",
    EXEPATH(MIKTEX_MPM_MFC_EXE),
    LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance (Admin)->Settings (Admin)"
  {
    false,
    "Maintenance (Admin)",
    "MiKTeX Settings (Admin)",
    EXEPATH(MIKTEX_MO_ADMIN_EXE),
    LD_IFCOMMON | LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance (Admin)->Update (Admin)"
  {
    false,
    "Maintenance (Admin)",
    "MiKTeX Update (Admin)",
    INTEXEPATH(MIKTEX_COPYSTART_ADMIN_EXE),
    LD_IFCOMMON | LD_USESHOWCMD | LD_USEARGS | LD_USEICON,
    nullptr,
    "\"" INTEXEPATH(MIKTEX_UPDATE_ADMIN_EXE) "\"",
    INTEXEPATH(MIKTEX_UPDATE_ADMIN_EXE),
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Maintenance (Admin)->Package Manager (Admin)"
  {
    false,
    "Maintenance (Admin)",
    "MiKTeX Package Manager (Admin)",
    EXEPATH(MIKTEX_MPM_MFC_ADMIN_EXE),
    LD_IFCOMMON | LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Help->Manual"
  {
    false,
    "Help",
    "MiKTeX Manual",
    DOCPATH(MIKTEX_MAIN_HELP_FILE),
    LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "Help->FAQ"
  {
    false,
    "Help",
    "MiKTeX FAQ",
    DOCPATH(MIKTEX_FAQ_HELP_FILE),
    LD_USESHOWCMD,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    SW_SHOWNORMAL,
    0,
  },

  // "MiKTeX on the Web->MiKTeX Project Page"
  {
    true,
    "MiKTeX on the Web",
    "MiKTeX Project Page",
    MIKTEX_URL_WWW,
    0,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    0,
    0,
  },

  // "MiKTeX on the Web->Support"
  {
    true,
    "MiKTeX on the Web",
    "MiKTeX Support",
    MIKTEX_URL_WWW_SUPPORT,
    0,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    0,
    0,
  },

  // "MiKTeX on the Web->Give back"
  {
    true,
    "MiKTeX on the Web",
    "Give back",
    MIKTEX_URL_WWW_GIVE_BACK,
    0,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    0,
    0,
  },

  // "MiKTeX on the Web->Known Issues"
  {
    true,
    "MiKTeX on the Web",
    "Known Issues",
    MIKTEX_URL_WWW_KNOWN_ISSUES,
    0,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    0,
    0,
  },

};

size_t nShellLinks = sizeof(shellLinks) / sizeof(shellLinks[0]);

END_ANONYMOUS_NAMESPACE;

void winSetupServiceImpl::CreateProgramIcons()
{
  PathName path = CreateProgramFolder();
  for (size_t i = 0; i < nShellLinks; ++i)
  {
    CreateShellLink(path, shellLinks[i]);
  }
}

void winSetupServiceImpl::CreateShellLink(const PathName & pathFolder, const ShellLinkData & ld)
{
  if ((ld.flags & LD_IFCOMMON) != 0 && !options.IsCommonSetup)
  {
    // ignore system-wide command if this is a per-user setup
    return;
  }

  PathName pathLink;

  if (ld.lpszFolder != nullptr)
  {
    PathName pathSubFolder(pathFolder, ld.lpszFolder);
    Directory::Create(pathSubFolder);
    pathLink = pathSubFolder;
  }
  else
  {
    pathLink = pathFolder;
  }

  pathLink /= ld.lpszName;
  pathLink.AppendExtension(ld.isUrl ? ".url" : ".lnk");

  if (File::Exists(pathLink))
  {
    Log(T_("removing %s...\n"), Q_(pathLink));
    if (!options.IsDryRun)
    {
      File::Delete(pathLink);
    }
  }

  if (ld.lpszPathName == nullptr)
  {
    return;
  }

  if (ld.isUrl)
  {
    Log(T_("creating internet shortcut %s...\n"), Q_(pathLink));
  }
  else
  {
    Log(T_("creating shell link %s...\n"), Q_(pathLink));
  }

  if (options.IsDryRun)
  {
    return;
  }

  if (ld.isUrl)
  {
    CreateInternetShortcut(pathLink, ld.lpszPathName);
  }
  else
  {
    _COM_SMARTPTR_TYPEDEF(IShellLinkW, IID_IShellLinkW);

    IShellLinkWPtr psl;

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void **>(&psl));

    if (FAILED(hr))
    {
      Log(T_("IShellLinkW could not be created (%08x)\n"), hr);
      MIKTEX_UNEXPECTED();
    }

    wstring str;

    hr = psl->SetPath(Expand(ld.lpszPathName, str).c_str());

    if (FAILED(hr))
    {
      Log(T_("IShellLinkW::SetPath() failed (%08x)\n"), hr);
      MIKTEX_UNEXPECTED();
    }

    if ((ld.flags & LD_USEARGS) != 0)
    {
      hr = psl->SetArguments(Expand(ld.lpszArgs, str).c_str());
      if (FAILED(hr))
      {
        Log(T_("IShellLinkW::SetArguments() failed (%08x)\n"), hr);
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEDESC) != 0)
    {
      hr = psl->SetDescription(UW_(ld.lpszDescription));
      if (FAILED(hr))
      {
        Log(T_("IShellLinkW::SetDescription() failed (%08x)\n"), hr);
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEICON) != 0)
    {
      hr = psl->SetIconLocation(Expand(ld.lpszIconPath, str).c_str(), ld.iconIndex);
      if (FAILED(hr))
      {
        Log(T_("IShellLinkW::SetIconLocation() failed (%08x)\n"), hr);
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEWORKDIR) != 0)
    {
      hr = psl->SetWorkingDirectory(Expand(ld.lpszWorkingDir, str).c_str());
      if (FAILED(hr))
      {
        Log(T_("IShellLinkW::SetWorkingDirectory() failed (%08x)\n"), hr);
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USESHOWCMD) != 0)
    {
      hr = psl->SetShowCmd(ld.showCmd);
      if (FAILED(hr))
      {
        Log(T_("IShellLinkW::SetShowCmd() failed (%08x)\n"), hr);
        MIKTEX_UNEXPECTED();
      }
    }

    if ((ld.flags & LD_USEHOTKEY) != 0)
    {
      hr = psl->SetHotkey(ld.hotKey);
      if (FAILED(hr))
      {
        Log(T_("IShellLinkW::SetHotkey() failed (%08x)\n"), hr);
        MIKTEX_UNEXPECTED();
      }
    }

    IPersistFilePtr ppf;

    hr = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&ppf));

    if (FAILED(hr))
    {
      Log(T_("IPersistFile could not be created (%08x)\n"), hr);
      MIKTEX_UNEXPECTED();
    }

    hr = ppf->Save(pathLink.ToWideCharString().c_str(), TRUE);

    if (FAILED(hr))
    {
      Log(T_("IPersistFile::Save() failed (%08x)\n"), hr);
      MIKTEX_UNEXPECTED();
    }
  }

  ULogAddFile(pathLink.GetData());
}

void winSetupServiceImpl::CreateInternetShortcut(const PathName & path, const char * lpszUrl)
{
  _COM_SMARTPTR_TYPEDEF(IUniformResourceLocatorW, IID_IUniformResourceLocatorW);

  IUniformResourceLocatorWPtr pURL;

  HRESULT hr = CoCreateInstance(CLSID_InternetShortcut, 0, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocatorW, reinterpret_cast<void **>(&pURL));

  if (FAILED(hr))
  {
    Log(T_("IUniformResourceLocator could not be created (%08x)\n"), hr);
    MIKTEX_UNEXPECTED();
  }

  hr = pURL->SetURL(UW_(lpszUrl), 0);

  if (FAILED(hr))
  {
    Log(T_("IUniformResourceLocatorW::SetURL() failed (%08x)\n"), hr);
    MIKTEX_UNEXPECTED();
  }

  IPersistFilePtr pPF;

  hr = pURL->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&pPF));

  if (FAILED(hr))
  {
    Log(T_("IPersistFile could not be created (%08x)\n"), hr);
    MIKTEX_UNEXPECTED();
  }

  hr = pPF->Save(path.ToWideCharString().c_str(), TRUE);

  if (FAILED(hr))
  {
    Log(T_("IPersistFile::Save() failed (%08x)\n"), hr);
    MIKTEX_UNEXPECTED();
  }
}

#define UNINST_HELP_LINK "http://miktex.org/support"
#define UNINST_PUBLISHER MIKTEX_COMP_COMPANY_STR
#define UNINST_DISPLAY_VERSION MIKTEX_VERSION_STR
#define UNINST_DISPLAY_STRING \
  (options.Task == SetupTask::PrepareMiKTeXDirect \
   ? UNINST_DISPLAY_NAME_MIKTEXDIRECT \
   : UNINST_DISPLAY_NAME)
#define UNINST_ABOUT_URL "http://miktex.org/about"
#define UNINST_UPDATE_URL "http://miktex.org"
#define UNINST_COMMENT T_("Uninstall MiKTeX")
#define UNINST_README MIKTEX_URL_WWW_KNOWN_ISSUES

#define UNINST_DISPLAY_NAME \
  MIKTEX_PRODUCTNAME_STR " " MIKTEX_VERSION_STR

#define UNINST_DISPLAY_NAME_MIKTEXDIRECT \
  "MiKTeXDirect" " " MIKTEX_VERSION_STR

#define REGSTR_PATH_UNINSTALL_A "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"

#define UNINST_REG_PATH \
    (options.Task == SetupTask::PrepareMiKTeXDirect \
     ? REGSTR_PATH_UNINSTALL_A "\\" UNINST_DISPLAY_NAME_MIKTEXDIRECT \
     : REGSTR_PATH_UNINSTALL_A "\\" UNINST_DISPLAY_NAME)

#define UNINST_HKEY_ROOT \
  (options.IsCommonSetup \
   ? HKEY_LOCAL_MACHINE \
   : HKEY_CURRENT_USER)

void winSetupServiceImpl::RegisterUninstaller()
{
  // make uninstall command line
  string commandLine;
  if (options.Task != SetupTask::PrepareMiKTeXDirect)
  {
    PathName pathCopyStart(GetInstallRoot(), (options.IsCommonSetup ? MIKTEX_PATH_INTERNAL_COPYSTART_ADMIN_EXE : MIKTEX_PATH_INTERNAL_COPYSTART_EXE));
    commandLine += Q_(pathCopyStart.GetData());
    commandLine += " ";
  }
  PathName pathUninstallDat(GetInstallRoot(), (options.IsCommonSetup ? MIKTEX_PATH_INTERNAL_UNINSTALL_ADMIN_EXE : MIKTEX_PATH_INTERNAL_UNINSTALL_EXE));
  commandLine += Q_(pathUninstallDat.GetData());

  // make icon path
  PathName iconPath(GetInstallRoot());
  iconPath /= MIKTEX_PATH_BIN_DIR;
  iconPath /= MIKTEX_MO_EXE;
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
  AddUninstallerRegValue(hkey, "Comment", UNINST_COMMENT);
  AddUninstallerRegValue(hkey, "DisplayIcon", iconPath.GetData());
  AddUninstallerRegValue(hkey, "DisplayName", UNINST_DISPLAY_STRING);
  AddUninstallerRegValue(hkey, "DisplayVersion", UNINST_DISPLAY_VERSION);
  AddUninstallerRegValue(hkey, "HelpLink", UNINST_HELP_LINK);
  AddUninstallerRegValue(hkey, "InstallLocation", installRoot.GetData());
  AddUninstallerRegValue(hkey, "NoModify", 1);
  AddUninstallerRegValue(hkey, "NoRepair", 1);
  AddUninstallerRegValue(hkey, "Publisher", UNINST_PUBLISHER);
  AddUninstallerRegValue(hkey, "Readme", UNINST_README);
  AddUninstallerRegValue(hkey, "UninstallString", commandLine.c_str());
  AddUninstallerRegValue(hkey, "UrlInfoAbout", UNINST_ABOUT_URL);
  AddUninstallerRegValue(hkey, "UrlUpdateInfo", UNINST_UPDATE_URL);
}

void winSetupServiceImpl::AddUninstallerRegValue(HKEY hkey, const char * lpszValueName, const char * lpszValue)
{
  wstring value(UW_(lpszValue));
  LONG result = RegSetValueExW(hkey, UW_(lpszValueName), 0, REG_SZ, reinterpret_cast<const BYTE *>(value.c_str()), static_cast<DWORD>((value.length() + 1) * sizeof(wchar_t)));
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegSetValueExW", result);
  }
  ULogAddRegValue(UNINST_HKEY_ROOT, UNINST_REG_PATH, lpszValueName);
}

void winSetupServiceImpl::AddUninstallerRegValue(HKEY hkey, const char * lpszValueName, DWORD value)
{
  LONG result = RegSetValueExW(hkey, UW_(lpszValueName), 0, REG_DWORD, reinterpret_cast<const BYTE *>(&value), static_cast<DWORD>(sizeof(value)));
  if (result != ERROR_SUCCESS)
  {
    MIKTEX_FATAL_WINDOWS_RESULT("RegSetValueExW", result);
  }
  ULogAddRegValue(UNINST_HKEY_ROOT, UNINST_REG_PATH, lpszValueName);
}

void winSetupServiceImpl::UnregisterShellFileTypes()
{
  shared_ptr<Session> session = Session::Get();
  PathName initexmfExe;
  if (!session->FindFile(MIKTEX_INITEXMF_EXE, FileType::EXE, initexmfExe))
  {
    MIKTEX_UNEXPECTED();
  }
  if (session->RunningAsAdministrator() || session->RunningAsPowerUser())
  {
    Process::Run(initexmfExe, { initexmfExe.GetFileNameWithoutExtension().ToString(), "--admin", "--unregister-shell-file-types" });
  }
  Process::Run(initexmfExe, { initexmfExe.GetFileNameWithoutExtension().ToString(), "--unregister-shell-file-types" });
}

void winSetupServiceImpl::UnregisterPath(bool shared)
{
#define REGSTR_KEY_ENVIRONMENT_COMMON \
   REGSTR_PATH_CURRENTCONTROLSET_A "\\Control\\Session Manager\\Environment"
#define REGSTR_KEY_ENVIRONMENT_USER "Environment"

  HKEY hkey;

#define REGSTR_PATH_CURRENTCONTROLSET_A "System\\CurrentControlSet"

  string subkey(shared
    ? REGSTR_KEY_ENVIRONMENT_COMMON
    : REGSTR_KEY_ENVIRONMENT_USER);

  LONG result = RegOpenKeyExW((shared ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER), UW_(subkey.c_str()), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey);

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
    if (RemoveBinDirFromPath(path))
    {
      CharBuffer<wchar_t> wpath(UW_(path.c_str()));
      result = RegSetValueExW(hkey, L"Path", 0, type, reinterpret_cast<const BYTE *>(wpath.GetData()), static_cast<DWORD>((StrLen(wpath.GetData()) + 1) * sizeof(wpath.GetData()[0])));

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

bool winSetupServiceImpl::RemoveBinDirFromPath(string & path)
{
  shared_ptr<Session> session = Session::Get();
  bool removed = false;
  string newPath;
  PathName userBinDir = session->GetSpecialPath(SpecialPath::UserInstallRoot);
  userBinDir /= MIKTEX_PATH_BIN_DIR;
  userBinDir.AppendDirectoryDelimiter();
  PathName commonBinDir = session->GetSpecialPath(SpecialPath::CommonInstallRoot);
  commonBinDir /= MIKTEX_PATH_BIN_DIR;
  commonBinDir.AppendDirectoryDelimiter();
  for (CsvList entry(path, PathName::PathNameDelimiter); entry; ++entry)
  {
    PathName dir(*entry);
    dir.AppendDirectoryDelimiter();
    if (userBinDir == dir || commonBinDir == dir)
    {
      removed = true;
    }
    else
    {
      if (!newPath.empty())
      {
        newPath += PathName::PathNameDelimiter;
      }
      newPath += *entry;
    }
  }
  if (removed)
  {
    path = newPath;
  }
  return removed;
}

void winSetupServiceImpl::RemoveRegistryKeys()
{
  shared_ptr<Session> session = Session::Get();
  bool shared = session->RunningAsAdministrator() || session->RunningAsPowerUser();

  if (shared && Exists(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_SERIES))
  {
    RemoveRegistryKey(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_SERIES);
  }

  if (Exists(HKEY_CURRENT_USER, MIKTEX_REGPATH_SERIES))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, MIKTEX_REGPATH_SERIES);
  }

  if (shared
    && Exists(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_PRODUCT)
    && IsEmpty(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_PRODUCT))
  {
    RemoveRegistryKey(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_PRODUCT);
  }

  if (shared
    && Exists(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_COMPANY)
    && IsEmpty(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_COMPANY))
  {
    RemoveRegistryKey(HKEY_LOCAL_MACHINE, MIKTEX_REGPATH_COMPANY);
  }

  if (Exists(HKEY_CURRENT_USER, MIKTEX_REGPATH_PRODUCT)
    && IsEmpty(HKEY_CURRENT_USER, MIKTEX_REGPATH_PRODUCT))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, MIKTEX_REGPATH_PRODUCT);
  }

  if (Exists(HKEY_CURRENT_USER, MIKTEX_REGPATH_COMPANY)
    && IsEmpty(HKEY_CURRENT_USER, MIKTEX_REGPATH_COMPANY))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, MIKTEX_REGPATH_COMPANY);
  }

  if (Exists(HKEY_CURRENT_USER, MIKTEX_GPL_GHOSTSCRIPT))
  {
    RemoveRegistryKey(HKEY_CURRENT_USER, MIKTEX_GPL_GHOSTSCRIPT);
  }
}

void winSetupServiceImpl::RemoveRegistryKey(HKEY hkeyRoot, const PathName & subKey)
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
    RemoveRegistryKey(hkeyRoot, PathName(subKey, szName));
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

bool winSetupServiceImpl::Exists(HKEY hkeyRoot, const PathName & subKey)
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

bool winSetupServiceImpl::IsEmpty(HKEY hkeyRoot, const PathName & subKey)
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
