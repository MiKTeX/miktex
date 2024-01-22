/**
 * @file Setup.h
 * @author Christian Schenk
 * @brief MiKTeX Setup Wizard
 *
 * @copyright Copyright © 1999-2024 Christian Schenk
 *
 * This file is part of MiKTeX Setup Wizard.
 *
 * MiKTeX Setup Wizard is licensed under GNU General Public License version 2 or
 * any later version.
 */

#pragma once

#include "resource.h"

#include "config.h"

#define T_(x) MIKTEXTEXT(x)

#define Q_(x) MiKTeX::Core::Quoter<char>(x).GetData()

#if !defined(UNICODE)
#error UNICODE required
#endif

#define TU_(x) MiKTeX::Util::CharBuffer<char>(x).GetData()
#define UT_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()

#define UW_(x) MiKTeX::Util::StringUtil::UTF8ToWideChar(x).c_str()
#define WU_(x) MiKTeX::Util::StringUtil::WideCharToUTF8(x).c_str()

#define DBGLOC() OutputDebugStringA(fmt::format("{0}:{1}", __FILE__, __LINE__).c_str())
#define DBGLOC2(s) OutputDebugStringA(fmt::format("{0}:{1} {2}", __FILE__, __LINE__, s).c_str())

class SetupApp :
    public CWinApp
{

public:

    SetupApp();

    BOOL InitInstance() override;

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

    void SetInstallationDirectory(const PathName& path)
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

    SetupTask GetTask()
    {
        return Service->GetOptions().Task;
    }

    void SetTask(SetupTask task)
    {
        SetupOptions options = Service->GetOptions();
        options.Task = task;
        Service->SetOptions(options);
    }

    bool IsCommonSetup()
    {
        return Service->GetOptions().IsCommonSetup;
    }

    bool IsPortable()
    {
        return Service->GetOptions().IsPortable;
    }

    bool IsDryRun()
    {
        return Service->GetOptions().IsDryRun;
    }

    PathName GetLocalPackageRepository()
    {
        return Service->GetOptions().LocalPackageRepository;
    }

    string GetRemotePackageRepository()
    {
        return Service->GetOptions().RemotePackageRepository;
    }

    PackageLevel GetPackageLevel()
    {
        return Service->GetOptions().PackageLevel;
    }

    PathName GetFolderName()
    {
        return Service->GetOptions().FolderName;
    }

    StartupConfig GetStartupConfig()
    {
        return Service->GetOptions().Config;
    }

    bool AllowUnattendedReboot = false;
    bool CheckUpdatesOnExit = false;
    bool IsMiKTeXDirect = false;
    bool IsRestarted = false;
    bool IsUnattended = false;
    unique_ptr<SetupService> Service;
    bool ShowLogFileOnExit = false;
    std::shared_ptr<MiKTeX::Packages::PackageManager> packageManager;
    PackageLevel prefabricatedPackageLevel = PackageLevel::None;
    unique_ptr<TraceStream> traceStream;

    static SetupApp* Instance;

    DECLARE_MESSAGE_MAP();
};

void DDV_Path(CDataExchange* dx, const CString& str);
bool FindFile(const PathName& fileName, PathName& result);
void ReportError(const MiKTeXException& e);
void ReportError(const exception& e);
