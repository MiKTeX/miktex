/**
 * @file topics/fndb/commands/refresh.cpp
 * @author Christian Schenk
 * @brief fndb refresh
 *
 * @copyright Copyright © 2021-2022 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

#include <config.h>

#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <miktex/Configuration/ConfigurationProvider>
#include <miktex/Core/Fndb>
#include <miktex/Core/Paths>
#include <miktex/Core/Session>
#include <miktex/PackageManager/PackageManager>
#include <miktex/Util/PathName>
#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

namespace
{
    class RefreshCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Refresh the file name database");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "refresh";
        }

        std::string Synopsis() override
        {
            return "refresh";
        }
    };
}

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;
using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FNDB;

unique_ptr<Command> Commands::Refresh()
{
    return make_unique<RefreshCommand>();
}

void RefreshFilenameDatabase(ApplicationContext& ctx, const PathName& root)
{
    if (!ctx.session->UnloadFilenameDatabase())
    {
        ctx.ui->FatalError(T_("the file name database could not be unloaded"));
    }
    unsigned rootIdx = ctx.session->DeriveTEXMFRoot(root);
    PathName fndbPath = ctx.session->GetFilenameDatabasePathName(rootIdx);
    if (ctx.session->IsCommonRootDirectory(rootIdx))
    {
        ctx.ui->Verbose(1, fmt::format(T_("Creating FNDB for common root directory ({0})..."), Q_(root.ToDisplayString())));
    }
    else
    {
        ctx.ui->Verbose(1, fmt::format(T_("Creating FNDB for user root directory ({0})..."), Q_(root.ToDisplayString())));
    }
    Fndb::Create(fndbPath, root, nullptr);
}

enum Option
{
    OPT_AAA = 1,
};

static const struct poptOption options[] =
{
    POPT_AUTOHELP
    POPT_TABLEEND
};

int RefreshCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], options);
    int option;
    while ((option = popt.GetNextOpt()) >= 0)
    {
    }
    if (option != -1)
    {
        ctx.ui->IncorrectUsage(fmt::format("{0}: {1}", popt.BadOption(POPT_BADOPTION_NOALIAS), popt.Strerror(option)));
    }
    if (!popt.GetLeftovers().empty())
    {
        ctx.ui->IncorrectUsage(T_("unexpected command arguments"));
    }
    unsigned nRoots = ctx.session->GetNumberOfTEXMFRoots();
    for (unsigned r = 0; r < nRoots; ++r)
    {
        if (ctx.session->IsAdminMode())
        {
            if (ctx.session->IsCommonRootDirectory(r))
            {
                RefreshFilenameDatabase(ctx, ctx.session->GetRootDirectoryPath(r));
            }
            else
            {
                ctx.ui->Verbose(1, fmt::format(T_("Skipping user root directory ({0})..."), Q_(ctx.session->GetRootDirectoryPath(r).ToDisplayString())));
            }
        }
        else
        {
            if (!ctx.session->IsCommonRootDirectory(r) || ctx.session->IsMiKTeXPortable())
            {
                RefreshFilenameDatabase(ctx, ctx.session->GetRootDirectoryPath(r));
            }
            else
            {
                ctx.ui->Verbose(1, fmt::format(T_("Skipping common root directory ({0})..."), Q_(ctx.session->GetRootDirectoryPath(r).ToDisplayString())));
            }
        }
    }
    PackageInfo test;
    bool havePackageDatabase = ctx.packageManager->TryGetPackageInfo("miktex-tex", test);
    if (!havePackageDatabase)
    {
        if (ctx.installer->IsInstallerEnabled())
        {
            ctx.packageInstaller->UpdateDb({});
        }
        else
        {
            ctx.ui->Warning(T_("the local package database does not exist"));
        }
    }
    else
    {
        ctx.ui->Verbose(1, T_("Refreshing FNDB for MPM..."));
        ctx.packageManager->CreateMpmFndb();
    }
    return 0;
}
