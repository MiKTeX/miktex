/**
 * @file topics/packages/commands/require.cpp
 * @author Christian Schenk
 * @brief packages require
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
    class RequireCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Make sure required MiKTeX packages are installed");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "require";
        }

        std::string Synopsis() override
        {
            return "require [--package-id-file <file>] [--repository <repository>] <package-id>...";
        }

        void Require(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& requiredPackages, const std::string& repository);
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

unique_ptr<Command> Commands::Require()
{
    return make_unique<RequireCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_PACKAGE_ID_FILE,
    OPT_REPOSITORY,
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

int RequireCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string repository;
    vector<string> requiredPackages;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_PACKAGE_ID_FILE:
            ReadNames(PathName(popt.GetOptArg()), requiredPackages);
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
    requiredPackages.insert(requiredPackages.end(), leftOvers.begin(), leftOvers.end());
    if (requiredPackages.empty())
    {
        ctx.ui->FatalError(T_("missing package ID"));
    }
    Require(ctx, requiredPackages, repository);
    return 0;
}

void RequireCommand::Require(ApplicationContext& ctx, const vector<string>& requiredPackages, const string& repository)
{
    vector<string> toBeInstalled;
    for (const string& packageID : requiredPackages)
    {
        PackageInfo packageInfo = ctx.packageManager->GetPackageInfo(packageID);
        if (!packageInfo.IsInstalled())
        {
            toBeInstalled.push_back(packageID);
        }
    }
    MyPackageInstallerCallback cb;
    auto packageInstaller = ctx.packageManager->CreateInstaller({ &cb, true, true });
    if (!repository.empty())
    {
        packageInstaller->SetRepository(repository);
    }
    cb.ctx = &ctx;
    cb.packageInstaller = packageInstaller.get();
    packageInstaller->SetFileLists(toBeInstalled, {});
    packageInstaller->InstallRemove(PackageInstaller::Role::Application);
}
