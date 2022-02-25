/**
 * @file topics/packages/commands/install.cpp
 * @author Christian Schenk
 * @brief packages install
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
#include <miktex/Util/PathName>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

#include "private.h"

namespace
{
    class InstallCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Install MiKTeX packages");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "install";
        }

        std::string Synopsis() override
        {
            return "install [--package-id=PACKAGEID] [--package-id-file=FILE] [--repository=REPOSITORY]";
        }

        void Install(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& toBeInstalled, const std::string& repository);
    };
}

using namespace std;

using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Packages;

unique_ptr<Command> Commands::Install()
{
    return make_unique<InstallCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_PACKAGE_ID,
    OPT_PACKAGE_ID_FILE,
    OPT_REPOSITORY
};

static const struct poptOption options[] =
{
    {
        "package-id", 0,
        POPT_ARG_STRING, nullptr,
        OPT_PACKAGE_ID,
        T_("Specify the package ID."),
        "PACKAGEID"
    },
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

int InstallCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string repository;
    vector<string> toBeInstalled;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_PACKAGE_ID:
            toBeInstalled.push_back(popt.GetOptArg());
            break;
        case OPT_PACKAGE_ID_FILE:
            ReadNames(PathName(popt.GetOptArg()), toBeInstalled);
            break;
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
    if (toBeInstalled.empty())
    {
        ctx.ui->FatalError(T_("missing package ID"));
    }
    Install(ctx, toBeInstalled, repository);
    return 0;
}

void InstallCommand::Install(ApplicationContext& ctx, const vector<string>& toBeInstalled, const string& repository)
{
    for (const string& packageID : toBeInstalled)
    {
        PackageInfo packageInfo = ctx.packageManager->GetPackageInfo(packageID);
        if (packageInfo.IsInstalled())
        {
            ctx.ui->FatalError(fmt::format(T_("{0}: package is already installed"), packageID));
        }
    }
    if (!repository.empty())
    {
        ctx.packageInstaller->SetRepository(repository);
    }
    MyPackageInstallerCallback cb;
    auto packageInstaller = ctx.packageManager->CreateInstaller({ &cb, true, true });
    cb.ctx = &ctx;
    cb.packageInstaller = packageInstaller.get();
    packageInstaller->SetFileLists(toBeInstalled, {});
    packageInstaller->InstallRemove(PackageInstaller::Role::Application);
    packageInstaller->Dispose();
}
