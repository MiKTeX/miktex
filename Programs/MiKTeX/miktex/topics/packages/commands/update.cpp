/**
 * @file topics/packages/commands/update.cpp
 * @author Christian Schenk
 * @brief packages update
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
    class UpdateCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Update MiKTeX packages");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "update";
        }

        std::string Synopsis() override
        {
            return "update [--package-id-file <file>] [--repository <repository>] [<package-id>...]";
        }

        void Update(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& toBeUpdated, const std::string& repository);
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

unique_ptr<Command> Commands::Update()
{
    return make_unique<UpdateCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_PACKAGE_ID_FILE,
    OPT_REPOSITORY
};

static const struct poptOption options[] =
{
    {
        "package-id-file", 0,
        POPT_ARG_STRING, nullptr,
        OPT_PACKAGE_ID_FILE,
        T_("Read package IDs from file."),
        "FILE"
    },
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

int UpdateCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string repository;
    vector<string> requestedUpdates;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_PACKAGE_ID_FILE:
            ReadNames(PathName(popt.GetOptArg()), requestedUpdates);
            break;
        case OPT_REPOSITORY:
            repository = popt.GetOptArg();
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftOvers = popt.GetLeftovers();
    requestedUpdates.insert(requestedUpdates.end(), leftOvers.begin(), leftOvers.end());
    Update(ctx, requestedUpdates, repository);
    return 0;
}

void UpdateCommand::Update(ApplicationContext& ctx, const vector<string>& requestedUpdates, const string& repository)
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
    for (const PackageInstaller::UpdateInfo& upd : packageInstaller->GetUpdates())
    {
        switch (upd.action)
        {
        case PackageInstaller::UpdateInfo::Repair:
        case PackageInstaller::UpdateInfo::ReleaseStateChange:
        case PackageInstaller::UpdateInfo::Update:
        case PackageInstaller::UpdateInfo::ForceUpdate:
            serverUpdates.push_back(upd.packageId);
            break;
        case PackageInstaller::UpdateInfo::ForceRemove:
            toBeRemoved.push_back(upd.packageId);
            break;
        default:
            break;
        }
    }
    vector<string> toBeInstalled;
    if (requestedUpdates.empty())
    {
        if (serverUpdates.empty())
        {
            ctx.ui->Verbose(0, T_("There are currently no updates available."));
        }
        else
        {
            toBeInstalled = serverUpdates;
        }
    }
    else
    {
        toBeRemoved.clear();
        sort(serverUpdates.begin(), serverUpdates.end());
        for (const string& packageId : requestedUpdates)
        {
            PackageInfo packageInfo = ctx.packageManager->GetPackageInfo(packageId);
            if (!packageInfo.IsInstalled())
            {
                ctx.ui->FatalError(fmt::format(T_("{0}: package is not installed"), packageId));
            }
            if (binary_search(serverUpdates.begin(), serverUpdates.end(), packageId))
            {
                toBeInstalled.push_back(packageId);
            }
            else
            {
                ctx.ui->Verbose(0, fmt::format(T_("Package \"{0}\" is up to date."), packageId));
            }
        }
    }
    if (toBeInstalled.empty() && toBeRemoved.empty())
    {
        return;
    }
    packageInstaller->SetFileLists(toBeInstalled, toBeRemoved);
    packageInstaller->InstallRemove(PackageInstaller::Role::Updater);
    unique_ptr<SetupService> service = SetupService::Create();
    SetupOptions options = service->GetOptions();
    options.Task = SetupTask::FinishUpdate;
    options = service->SetOptions(options);
    service->Run();
}
