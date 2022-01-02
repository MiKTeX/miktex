/**
 * @file topics/links/commands/install.cpp
 * @author Christian Schenk
 * @brief links install
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

#include <miktex/Wrappers/PoptWrapper>

#include "internal.h"

#include "commands.h"

#include "LinksManager.h"

namespace
{
    class InstallCommand :
        public OneMiKTeXUtility::Topics::Command
    {
        std::string Description() override
        {
            return T_("Install links from scripts and formats to executables");
        }

        int MIKTEXTHISCALL Execute(OneMiKTeXUtility::ApplicationContext& ctx, const std::vector<std::string>& arguments) override;

        std::string Name() override
        {
            return "install";
        }

        std::string Synopsis() override
        {
            return "install [--force]";
        }
    };
}

using namespace std;

using namespace MiKTeX::Wrappers;

using namespace OneMiKTeXUtility;
using namespace OneMiKTeXUtility::Topics;
using namespace OneMiKTeXUtility::Topics::Links;

unique_ptr<Command> Commands::Install()
{
    return make_unique<InstallCommand>();
}

enum Option
{
    OPT_AAA = 1,
    OPT_FORCE,
};

static const struct poptOption install_options[] =
{
    {
        "force", 0,
        POPT_ARG_NONE, nullptr,
        OPT_FORCE,
        T_("Overwrite existing files."),
        nullptr,
    },
    POPT_AUTOHELP
    POPT_TABLEEND
};

int InstallCommand::Execute(ApplicationContext& ctx, const vector<string>& arguments)
{
    if (ctx.session->IsSharedSetup() && !ctx.session->IsAdminMode())
    {
        ctx.ui->FatalError(T_("this command must be run in admin mode"));
    }
    auto argv = MakeArgv(arguments);
    PoptWrapper popt(static_cast<int>(argv.size() - 1), &argv[0], install_options);
    int option;
    bool force = false;
    while ((option = popt.GetNextOpt()) >= 0)
    {
        switch (option)
        {
        case OPT_FORCE:
            force = true;
            break;
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
    LinksManager mgr;
    mgr.Init(ctx);
    mgr.Install(force);
    return 0;
}
