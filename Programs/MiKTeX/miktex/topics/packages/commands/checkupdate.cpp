/**
 * @file topics/packages/commands/checkupdate.cpp
 * @author Christian Schenk
 * @brief packages check-update
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
    class UpdateInfoComparer
    {
    public:
        bool operator() (const MiKTeX::Packages::PackageInstaller::UpdateInfo& ui1, const MiKTeX::Packages::PackageInstaller::UpdateInfo& ui2) const
        {
            return MiKTeX::Util::PathName::Compare(ui1.packageId, ui2.packageId) < 0;
        }
    };

    class CheckUpdateCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Check for MiKTeX updates");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "check-update";
        }

        std::string Synopsis() override
        {
            return "check-update [--repository=REPOSITORY]";
        }

        bool CheckUpdate(OneMiKTeXUtility::ApplicationContext& ctx, const std::string& repository);
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

unique_ptr<Command> Commands::CheckUpdate()
{
    return make_unique<CheckUpdateCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_REPOSITORY
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

int CheckUpdateCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
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
    if (!popt.GetLeftovers().empty())
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }
    return CheckUpdate(ctx, repository) ? 100 : 0;
}

bool CheckUpdateCommand::CheckUpdate(ApplicationContext& ctx, const string& repository)
{
    MyPackageInstallerCallback cb;
    auto packageInstaller = ctx.packageManager->CreateInstaller({ &cb, true, true });
    if (!repository.empty())
    {
        packageInstaller->SetRepository(repository);
    }
    cb.ctx = &ctx;
    cb.packageInstaller = packageInstaller.get();
    packageInstaller->FindUpdates();
    vector<string> serverUpdates;
    vector<string> toBeRemoved;
    auto updates = packageInstaller->GetUpdates();
    packageInstaller = nullptr;
    if (updates.empty())
    {
        ctx.ui->Verbose(0, T_("There are currently no updates available."));
        return false;
    }
    sort(updates.begin(), updates.end(), UpdateInfoComparer());
    for (const PackageInstaller::UpdateInfo& upd : updates)
    {
        switch (upd.action)
        {
        case PackageInstaller::UpdateInfo::Repair:
        case PackageInstaller::UpdateInfo::ReleaseStateChange:
        case PackageInstaller::UpdateInfo::Update:
        case PackageInstaller::UpdateInfo::ForceUpdate:
            ctx.ui->Output(upd.packageId);
            break;
        default:
            break;
        }
    }
    return true;
}
