/**
 * @file topics/packages/commands/remove.cpp
 * @author Christian Schenk
 * @brief packages remove
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
    class RemoveCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Remove installed MiKTeX packages");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "remove";
        }

        std::string Synopsis() override
        {
            return "remove [--package-id-file <file>] <package-id>...";
        }

        void Remove(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& toBeRemoved);
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

unique_ptr<Command> Commands::Remove()
{
    return make_unique<RemoveCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_PACKAGE_ID_FILE,
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
    POPT_AUTOHELP
    POPT_TABLEEND
};

int RemoveCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string repository;
    vector<string> toBeRemoved;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_PACKAGE_ID_FILE:
            ReadNames(PathName(popt.GetOptArg()), toBeRemoved);
            break;
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftOvers = popt.GetLeftovers();
    toBeRemoved.insert(toBeRemoved.end(), leftOvers.begin(), leftOvers.end());
    if (toBeRemoved.empty())
    {
        ctx.ui->FatalError(T_("missing package ID"));
    }
    Remove(ctx, toBeRemoved);
    return 0;
}

void RemoveCommand::Remove(ApplicationContext& ctx, const vector<string>& toBeRemoved)
{
    for (const string& packageID : toBeRemoved)
    {
        PackageInfo packageInfo = ctx.packageManager->GetPackageInfo(packageID);
        if (!packageInfo.IsInstalled())
        {
            ctx.ui->FatalError(fmt::format(T_("{0}: package is not installed"), packageID));
        }
    }
    MyPackageInstallerCallback cb;
    auto packageInstaller = ctx.packageManager->CreateInstaller({ &cb, true, true });
    cb.ctx = &ctx;
    cb.packageInstaller = packageInstaller.get();
    packageInstaller->SetFileLists({}, toBeRemoved);
    packageInstaller->InstallRemove(PackageInstaller::Role::Application);
}
