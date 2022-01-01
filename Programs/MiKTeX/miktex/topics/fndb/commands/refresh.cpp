/**
 * @file topics/fndb/commands/update.cpp
 * @author Christian Schenk
 * @brief fndb update
 *
 * @copyright Copyright © 2021 Christian Schenk
 *
 * This file is part of One MiKTeX Utility.
 *
 * One MiKTeX Utility is licensed under GNU General Public
 * License version 2 or any later version.
 */

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

#include "internal.h"

#include "commands.h"

namespace
{
    class UpdateCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Update the file name database");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "update";
        }

        std::string Synopsis() override
        {
            return "update";
        }
    };
}

using namespace std;

using namespace MiKTeX::Configuration;
using namespace MiKTeX::Core;
using namespace MiKTeX::Packages;
using namespace MiKTeX::Util;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::FNDB;

unique_ptr<Command> Commands::Update()
{
    return make_unique<UpdateCommand>();
}

void UpdateFilenameDatabase(ApplicationContext& ctx, const PathName& root)
{
    if (!ctx.session->UnloadFilenameDatabase())
    {
        ctx.ui->FatalError(T_("the file name database could not be unloaded"));
    }
    unsigned rootIdx = ctx.session->DeriveTEXMFRoot(root);
    PathName fndbPath = ctx.session->GetFilenameDatabasePathName(rootIdx);
    if (ctx.session->IsCommonRootDirectory(rootIdx))
    {
        ctx.ui->Verbose(1, fmt::format(T_("Creating FNDB for common root directory ({0})..."), Q_(root)));
    }
    else
    {
        ctx.ui->Verbose(1, fmt::format(T_("Creating FNDB for user root directory ({0})..."), Q_(root)));
    }
    Fndb::Create(fndbPath, root, nullptr);
}

int UpdateCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (arguments.size() != 2)
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
                UpdateFilenameDatabase(ctx, ctx.session->GetRootDirectoryPath(r));
            }
            else
            {
                ctx.ui->Verbose(1, fmt::format(T_("Skipping user root directory ({0})..."), Q_(ctx.session->GetRootDirectoryPath(r))));
            }
        }
        else
        {
            if (!ctx.session->IsCommonRootDirectory(r) || ctx.session->IsMiKTeXPortable())
            {
                UpdateFilenameDatabase(ctx, ctx.session->GetRootDirectoryPath(r));
            }
            else
            {
                ctx.ui->Verbose(1, fmt::format(T_("Skipping common root directory ({0})..."), Q_(ctx.session->GetRootDirectoryPath(r))));
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
        ctx.ui->Verbose(1, T_("Creating FNDB for MPM..."));
        ctx.packageManager->CreateMpmFndb();
    }
}
