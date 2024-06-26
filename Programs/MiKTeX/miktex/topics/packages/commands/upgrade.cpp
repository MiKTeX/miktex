/**
 * @file topics/packages/commands/upgrade.cpp
 * @author Christian Schenk
 * @brief packages upgrade
 *
 * @copyright Copyright © 2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <config.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Setup/SetupService>
#include <miktex/Util/PathName>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

#include "private.h"

namespace
{
    class UpgradeCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Upgrade the MiKTeX setup to a package level");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "upgrade";
        }

        std::string Synopsis() override
        {
            return "upgrade [--repository <repository>] <package-level>";
        }

        void Upgrade(OneMiKTeXUtility::ApplicationContext& ctx, MiKTeX::Packages::PackageLevel packageLevel, const std::string& repository);
    };
}

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Setup;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Packages;

unique_ptr<Command> Commands::Upgrade()
{
    return make_unique<UpgradeCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_REPOSITORY,
};

static const struct poptOption options[] =
{
    {
        "repository", 0,
        POPT_ARG_STRING, nullptr,
        OPT_REPOSITORY,
        T_("Use the specified location as the package repository.  The location can be either a fully qualified path name (a local package repository) or an URL (a remote package repository)."),
        T_("LOCATION")
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int UpgradeCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string repository;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_REPOSITORY:
            repository = popt.GetOptArg();
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftOvers = popt.GetLeftovers();
    if (leftOvers.size() != 1)
    {
        ctx.ui->IncorrectUsage(T_("expected one <package-level> argument"));
    }
    auto packageLevel = ToPackageLevel(leftOvers[0]);
    if (packageLevel == PackageLevel::None)
    {
        ctx.ui->FatalError(fmt::format(T_("{0}: unknown package level"), leftOvers[0]));
    }
    Upgrade(ctx, packageLevel, repository);
    return 0;
}

void UpgradeCommand::Upgrade(ApplicationContext& ctx, PackageLevel packageLevel, const string& repository)
{
    MyPackageInstallerCallback cb;
    auto packageInstaller = ctx.packageManager->CreateInstaller({ &cb, true, true });
    if (!repository.empty())
    {
        packageInstaller->SetRepository(repository);
    }
    cb.ctx = &ctx;
    cb.packageInstaller = packageInstaller.get();
    packageInstaller->FindUpgrades(packageLevel);
    vector<PackageInstaller::UpgradeInfo> upgrades = packageInstaller->GetUpgrades();
    if (upgrades.empty())
    {
        ctx.ui->Verbose(0, T_("There are currently no upgrades available."));
        return;
    }
    vector<string> toBeInstalled;
    for (const PackageInstaller::UpgradeInfo& upg : upgrades)
    {
        toBeInstalled.push_back(upg.packageId);
    }
    sort(toBeInstalled.begin(), toBeInstalled.end());
    packageInstaller->SetFileLists(toBeInstalled, {});
    packageInstaller->InstallRemove(PackageInstaller::Role::Application);
    if (toBeInstalled.size() == 1)
    {
        ctx.ui->Verbose(0, fmt::format(T_("Package \"{0}\" has been successfully installed."), toBeInstalled[0]));
    }
    else if (toBeInstalled.size() > 1)
    {
        ctx.ui->Verbose(0, fmt::format(T_("{0} packages have been successfully installed."), toBeInstalled.size()));
    }
}
