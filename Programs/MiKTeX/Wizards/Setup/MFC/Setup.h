/* Setup.h:                                             -*- C++ -*-

   Copyright (C) 1999-2016 Christian Schenk

   This file is part of the MiKTeX Setup Wizard.

   The MiKTeX Setup Wizard is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Setup Wizard is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Setup Wizard; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#pragma once

#include "resource.h"

#include "config.h"

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).Get()

#if !defined(UNICODE)
#  error UNICODE required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#define UW_(x) MiKTeX::Util::StringUtil::UTF8ToWideChar(x).c_str()
#define WU_(x) MiKTeX::Util::StringUtil::WideCharToUTF8(x).c_str()

class SetupApp :
  public CWinApp
{
public:
  SetupApp();

public:
  static SetupApp * Instance;

public:
  virtual BOOL InitInstance();

public:
  DECLARE_MESSAGE_MAP();

public:
  unique_ptr<TraceStream> traceStream;

#if ENABLE_ADDTEXMF
public:
  vector<PathName> addTEXMFDirs;
#endif

public:
  std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;

public:
  unique_ptr<SetupService> Service;

public:
  PathName GetInstallationDirectory() const
  {
    SetupOptions options = Service->GetOptions();
    if (options.IsPortable)
    {
      return options.PortableRoot;
    }
    else
    {
      return options.IsCommonSetup ? options.Config.commonInstallRoot : options.Config.userInstallRoot;
    }
  }

public:
  void SetInstallationDirectory(const PathName & path)
  {
    SetupOptions options = Service->GetOptions();
    if (options.IsPortable)
    {
      options.PortableRoot = path;
    }
    else if (options.IsCommonSetup)
    {
      options.Config.commonInstallRoot = path;
    }
    else
    {
      options.Config.userInstallRoot = path;
    }
    Service->SetOptions(options);
  }

public:
  PathName GetInstallRoot() const
  {
    SetupOptions options = Service->GetOptions();
    if (options.IsPortable)
    {
      return options.PortableRoot / MIKTEX_PORTABLE_REL_INSTALL_DIR;
    }
    else
    {
      return options.IsCommonSetup ? options.Config.commonInstallRoot : options.Config.userInstallRoot;
    }
  }

public:
  SetupTask GetTask()
  {
    return Service->GetOptions().Task;
  }

public:
  void SetTask(SetupTask task)
  {
    SetupOptions options = Service->GetOptions();
    options.Task = task;
    Service->SetOptions(options);
  }

public:
  bool IsCommonSetup()
  {
    return Service->GetOptions().IsCommonSetup;
  }

public:
  bool IsPortable()
  {
    return Service->GetOptions().IsPortable;
  }

public:
  bool IsDryRun()
  {
    return Service->GetOptions().IsDryRun;
  }

public:
  PathName GetLocalPackageRepository()
  {
    return Service->GetOptions().LocalPackageRepository;
  }

public:
  string GetRemotePackageRepository()
  {
    return Service->GetOptions().RemotePackageRepository;
  }

public:
  PackageLevel GetPackageLevel()
  {
    return Service->GetOptions().PackageLevel;
  }

public:
  PathName GetFolderName()
  {
    return Service->GetOptions().FolderName;
  }

public:
  StartupConfig GetStartupConfig()
  {
    return Service->GetOptions().Config;
  }

#if ENABLE_ADDTEXMF
public:
  bool noAddTEXMFDirs;
#endif

public:
  bool AllowUnattendedReboot = false;

public:
  bool IsMiKTeXDirect = false;

public:
  PackageLevel prefabricatedPackageLevel = PackageLevel::None;

public:
  bool ShowLogFileOnExit = false;

public:
  bool IsUnattended = false;
};

#if ENABLE_ADDTEXMF
void CheckAddTEXMFDirs(string & directories, vector<PathName> & vec);
#endif

void DDV_Path(CDataExchange * DX, const CString & str);

bool FindFile(const PathName & fileName, PathName & result);

void ReportError(const MiKTeXException & e);

void ReportError(const exception & e);
