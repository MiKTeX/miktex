/**
 * @file topics/packages/commands/private.h
 * @author Christian Schenk
 * @brief Private parts
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#pragma once

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/PackageManager/PackageManager>

#include "internal.h"

class MyPackageInstallerCallback :
    public MiKTeX::Packages::PackageInstallerCallback
{
    void ReportLine(const std::string& str) override
    {
        ctx->ui->Verbose(1, str);
    }
    bool OnRetryableError(const std::string& message) override
    {
        return false;
    }
    bool OnProgress(MiKTeX::Packages::Notification nf) override
    {
        if (ctx->program->Canceled())
        {
            return false;
        }
        auto progressInfo = packageInstaller->GetProgressInfo();
        switch (nf)
        {
        case MiKTeX::Packages::Notification::DownloadPackageStart:
            ctx->ui->Verbose(0, fmt::format(T_("Downloading package {0}..."), progressInfo.packageId));
            break;
        case MiKTeX::Packages::Notification::InstallPackageStart:
            ctx->ui->Verbose(0, fmt::format(T_("Installing package {0}..."), progressInfo.packageId));
            break;
        case MiKTeX::Packages::Notification::RemovePackageStart:
            ctx->ui->Verbose(0, fmt::format(T_("Removing package {0}..."), progressInfo.packageId));
            break;
        }
        return true;
    }
public:
    OneMiKTeXUtility::ApplicationContext* ctx = nullptr;
    MiKTeX::Packages::PackageInstaller* packageInstaller = nullptr;
};

inline MiKTeX::Packages::PackageLevel ToPackageLevel(const std::string& s)
{
    if (s == "essential")
    {
        return MiKTeX::Packages::PackageLevel::Essential;
    }
    else if (s == "basic")
    {
        return MiKTeX::Packages::PackageLevel::Basic;
    }
    else if (s == "complete")
    {
        return MiKTeX::Packages::PackageLevel::Complete;
    }
    return MiKTeX::Packages::PackageLevel::None;
}

std::string Format(const std::string& outputTemplate, const MiKTeX::Packages::PackageInfo& packageInfo);
