/**
 * @file topics/packages/commands/verify.cpp
 * @author Christian Schenk
 * @brief packages verify
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
    class VerifyCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Verify MiKTeX packages");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "verify";
        }

        std::string Synopsis() override
        {
            return "verify [--package-id-file=FILE] [<package-id>...]";
        }

        void Verify(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& toBeVerified);
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

unique_ptr<Command> Commands::Verify()
{
    return make_unique<VerifyCommand>();
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

int VerifyCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    string repository;
    vector<string> toBeVerified;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_PACKAGE_ID_FILE:
            ReadNames(PathName(popt.GetOptArg()), toBeVerified);
            break;
        }
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    auto leftOvers = popt.GetLeftovers();
    toBeVerified.insert(toBeVerified.end(), leftOvers.begin(), leftOvers.end());
    Verify(ctx, toBeVerified);
    return 0;
}

void VerifyCommand::Verify(ApplicationContext& ctx, const vector<string>& toBeVerifiedArg)
{
    vector<string> toBeVerified = toBeVerifiedArg;
    bool verifyAll = toBeVerified.empty();
    if (verifyAll)
    {
        unique_ptr<PackageIterator> packageIterator(ctx.packageManager->CreateIterator());
        PackageInfo packageInfo;
        while (packageIterator->GetNext(packageInfo))
        {
            if (!packageInfo.IsPureContainer() && packageInfo.IsInstalled())
            {
                toBeVerified.push_back(packageInfo.id);
            }
        }
    }
    bool ok = true;
    for (const string& packageID : toBeVerified)
    {
        if (!ctx.packageManager->TryVerifyInstalledPackage(packageID))
        {
            ctx.ui->Verbose(0, fmt::format(T_("{0}: this package needs to be reinstalled."), packageID));
            ok = false;
        }
    }
    if (ok)
    {
        if (verifyAll)
        {
            ctx.ui->Verbose(0, T_("All packages are correctly installed."));
        }
        else
        {
            if (toBeVerified.size() == 1)
            {
                ctx.ui->Verbose(0, fmt::format(T_("Package {0} is correctly installed."), toBeVerified[0]));
            }
            else
            {
                ctx.ui->Verbose(0, T_("The packages are correctly installed."));
            }
        }
    }
    else
    {
        ctx.ui->FatalError(T_("Some packages need to be reinstalled."));
    }
}
